//
// Created by Utsav Lal on 10/14/24.
//

#ifndef WORKER_HPP
#define WORKER_HPP

#include <iostream>
#include <unordered_set>
#include <utility>
#include <zmq.hpp>

#include "../enum/message_type.hpp"


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

    void work() {
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


                if(clients.find(identity.to_string()) == clients.end()) {
                    // send all entities to new client
                    for (const auto &entity: gCoordinator.getEntityIds()) {
                        if(!gCoordinator.hasComponent<Transform>(entity.second) || !gCoordinator.hasComponent<Color>(entity.second)) {
                            continue;
                        }
                        auto &transform = gCoordinator.getComponent<Transform>(entity.second);
                        auto &color = gCoordinator.getComponent<Color>(entity.second);

                        std::vector<float> message;
                        message.reserve(10);
                        message.emplace_back(Message::UPDATE);
                        message.emplace_back(entity.second);
                        message.emplace_back(transform.x);
                        message.emplace_back(transform.y);
                        message.emplace_back(transform.h);
                        message.emplace_back(transform.w);
                        message.emplace_back(color.color.r);
                        message.emplace_back(color.color.g);
                        message.emplace_back(color.color.b);
                        message.emplace_back(color.color.a);

                        worker.send(zmq::buffer(entity_id.to_string()), zmq::send_flags::sndmore);
                        worker.send(zmq::buffer(message), zmq::send_flags::none);
                    }
                }

                clients.insert(identity.to_string());

                const std::vector<float> received_msg(static_cast<float *>(entity_data.data()),
                                                      static_cast<float *>(entity_data.data()) + entity_data.size() /
                                                      sizeof
                                                      (float));


                for (const auto &clientId: clients) {
                    if (clientId == identity.to_string()) {
                        continue;
                    }

                    worker.send(zmq::buffer(clientId + "R"), zmq::send_flags::sndmore);
                    worker.send(zmq::buffer(identity.to_string()), zmq::send_flags::sndmore);
                    worker.send(zmq::buffer(received_msg), zmq::send_flags::none);
                }


                // Process further
            }
        } catch (std::exception &e) {
            std::cout << "Worker error: " << e.what() << std::endl;
        }
    }
};

#endif //WORKER_HPP
