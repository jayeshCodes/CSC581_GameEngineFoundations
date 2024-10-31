//
// Created by Utsav Lal on 10/14/24.
//

#ifndef WORKER_HPP
#define WORKER_HPP

#include <iostream>
#include <unordered_set>
#include <utility>
#include <zmq.hpp>

#include "../enum/enum.hpp"
#include "../helpers/network_helper.hpp"
#include "../strategy/send_strategy.hpp"


extern Coordinator gCoordinator;

class Worker {
    zmq::context_t &context;
    zmq::socket_t worker;
    std::unordered_set<std::string> clients;
    Timeline timeline;
    std::string id;

public:
    Worker(zmq::context_t &context, int socket_type, std::string id) : context(context), worker(context, socket_type),
                                                                       timeline(nullptr, 1000), id(std::move(id)) {
    }

    void work(Send_Strategy *send_strategy) {
        worker.set(zmq::sockopt::routing_id, id);
        worker.connect("tcp://localhost:5571");

        try {
            while (true) {
                zmq::message_t identity;
                zmq::message_t entity_id;
                zmq::message_t entity_data;


                NetworkHelper::receiveMessageServer(worker, identity, entity_id, entity_data);


                if (clients.find(identity.to_string()) == clients.end()) {
                    // send all entities to new client
                    for (const auto &entity: gCoordinator.getEntityIds()) {
                        if (!gCoordinator.hasComponent<Transform>(entity.second) || !gCoordinator.hasComponent<Color>(
                                entity.second)) {
                            continue;
                        }
                        auto &transform = gCoordinator.getComponent<Transform>(entity.second);
                        auto &color = gCoordinator.getComponent<Color>(entity.second);
                        RigidBody rigidBody{0};
                        Collision collision{false, false};
                        if(gCoordinator.hasComponent<RigidBody>(entity.second)) {
                            rigidBody = gCoordinator.getComponent<RigidBody>(entity.second);
                        }
                        if(gCoordinator.hasComponent<Collision>(entity.second)) {
                            collision = gCoordinator.getComponent<Collision>(entity.second);
                        }
                        auto message = send_strategy->get_message(entity.second, transform, color, Message::UPDATE,
                                                                  rigidBody, collision);

                        NetworkHelper::sendMessageServer(worker, identity.to_string(), entity.first, message);
                    }
                }

                clients.insert(identity.to_string());

                std::variant<std::vector<char>, std::vector<float> > received_msg = send_strategy->copy_message(
                    entity_data);


                for (const auto &clientId: clients) {
                    if (clientId == identity.to_string()) {
                        continue;
                    }

                    NetworkHelper::sendMessageServer(worker, clientId, entity_id.to_string(), received_msg);
                }


                // Process further
            }
        } catch (std::exception &e) {
            std::cout << "Worker error: " << e.what() << std::endl;
        }
    }
};

#endif //WORKER_HPP
