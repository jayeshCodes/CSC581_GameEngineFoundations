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

                zmq::message_t entity_id_copy;
                zmq::message_t entity_data_copy;

                worker.recv(identity, zmq::recv_flags::none);
                worker.recv(entity_id, zmq::recv_flags::none);
                worker.recv(entity_data, zmq::recv_flags::none);


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

                        worker.send(zmq::buffer(identity.to_string() + "R"), zmq::send_flags::sndmore);
                        worker.send(zmq::buffer(entity.first), zmq::send_flags::sndmore);
                        if (std::holds_alternative<std::string>(message)) {
                            auto str = std::get<std::string>(message);
                            worker.send(zmq::buffer(str), zmq::send_flags::none);
                        } else {
                            auto vec = std::get<std::vector<float> >(message);
                            worker.send(zmq::buffer(vec), zmq::send_flags::none);
                        }
                    }
                }

                clients.insert(identity.to_string());

                std::variant<std::vector<char>, std::vector<float> > received_msg = send_strategy->copy_message(
                    entity_data);


                for (const auto &clientId: clients) {
                    if (clientId == identity.to_string()) {
                        continue;
                    }

                    worker.send(zmq::buffer(clientId + "R"), zmq::send_flags::sndmore);
                    worker.send(zmq::buffer(entity_id.to_string()), zmq::send_flags::sndmore);
                    if (std::holds_alternative<std::vector<float> >(received_msg)) {
                        auto str = std::get<std::vector<float> >(received_msg);
                        worker.send(zmq::buffer(str), zmq::send_flags::none);
                    } else {
                        auto vec = std::get<std::vector<char> >(received_msg);
                        worker.send(zmq::buffer(vec), zmq::send_flags::none);
                    }
                }


                // Process further
            }
        } catch (std::exception &e) {
            std::cout << "Worker error: " << e.what() << std::endl;
        }
    }
};

#endif //WORKER_HPP
