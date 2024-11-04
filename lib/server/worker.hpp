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
    Timeline timeline;
    std::string id;

public:
    Worker(zmq::context_t &context, int socket_type, std::string id) : context(context), worker(context, socket_type),
                                                                       timeline(nullptr, 1000), id(std::move(id)) {
    }

    void work(Send_Strategy *send_strategy, std::unordered_set<std::string> &clients, std::shared_mutex &mutex) {
        worker.set(zmq::sockopt::routing_id, id);
        worker.connect("tcp://localhost:5571");

        try {
            while (true) {
                zmq::message_t identity;
                zmq::message_t entity_id;
                zmq::message_t entity_data;


                NetworkHelper::receiveMessageServer(worker, identity, entity_id, entity_data); {
                    std::shared_lock<std::shared_mutex> read_lock(mutex);
                    if (clients.find(identity.to_string()) == clients.end()) {
                        read_lock.unlock();


                        // send all entities to new client
                        for (const auto &entity: gCoordinator.getEntityIds()) {
                                auto message = send_strategy->get_message(entity.second, Message::CREATE);

                                NetworkHelper::sendMessageServer(worker, identity.to_string(), entity.first, message);
                        }
                    }
                } {
                    std::unique_lock<std::shared_mutex> write_lock(mutex);
                    clients.insert(identity.to_string());
                    write_lock.unlock();
                }


                std::string received_msg = send_strategy->copy_message(entity_data); {
                    std::shared_lock<std::shared_mutex> read_lock(mutex);
                    for (const auto &clientId: clients) {
                        if (clientId == identity.to_string()) {
                            continue;
                        }

                        NetworkHelper::sendMessageServer(worker, clientId, entity_id.to_string(), received_msg);
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
