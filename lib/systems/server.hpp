//
// Created by Utsav Lal on 10/12/24.
//

#pragma once
#include <thread>
#include <zmq.hpp>
#include <memory>

#include "../ECS/coordinator.hpp"
#include "../enum/message_type.hpp"
#include "../generic/packed_map.hpp"
#include "../model/components.hpp"

extern Coordinator gCoordinator;

/**
 * This is a system that will handle client messages
 * First type of message is connect and disconnect which will simply trigger disconnection and entity destruction on all the clients
 * Second type of message is update which will update all the entities on all the clients
 */
class ServerSystem : public System {
    std::unordered_map<int, std::shared_ptr<zmq::socket_t> > clients;
    std::unordered_map<int, std::atomic<bool> > connected;
    std::unordered_map<int, std::thread> threads;
    int MESSAGE_SIZE = 11;

    void listen_client(zmq::socket_t &sub_socket, const int slot) const {
        zmq::pollitem_t pollitems[] = {{sub_socket, 0, ZMQ_POLLIN, 0}};

        while (connected.at(slot).load()) {
            zmq::poll(&pollitems[0], 1, std::chrono::milliseconds(1000));
            if (pollitems[0].revents & ZMQ_POLLIN) {
                zmq::message_t message;
                auto res = sub_socket.recv(message, zmq::recv_flags::none);
                const std::vector<float> received_msg(static_cast<float *>(message.data()),
                                                      static_cast<float *>(message.data()) + message.size() / sizeof
                                                      (
                                                          float));

                for (int i = 0; i < received_msg.size(); i += MESSAGE_SIZE) {
                    bool end = false;
                    const auto entity = static_cast<Entity>(received_msg[i + 2]);
                    const std::string key = std::to_string(slot) + "_" + Coordinator::createKey(entity);
                    switch (static_cast<Message>(received_msg[i])) {
                        case DESTROY: {
                            const Entity generatedId = gCoordinator.createEntity(key);
                            auto &[thisslot, destroyed, isSent] = gCoordinator.getComponent<Destroy>(generatedId);
                            thisslot = slot;
                            destroyed = true;
                            isSent = false;
                            break;
                        }
                        case UPDATE: {
                            const Entity generatedId = gCoordinator.createEntity(key);
                            gCoordinator.addComponent<Transform>(generatedId, Transform{});
                            gCoordinator.addComponent<Color>(generatedId, Color{});
                            gCoordinator.addComponent<ServerEntity>(generatedId, ServerEntity{});
                            gCoordinator.addComponent<Destroy>(generatedId, Destroy{slot, false});
                            auto &[x, y, h, w, orientation, scale] = gCoordinator.getComponent<Transform>(
                                generatedId);
                            auto &[color] = gCoordinator.getComponent<Color>(generatedId);
                            x = received_msg[i + 3];
                            y = received_msg[i + 4];
                            w = received_msg[i + 5];
                            h = received_msg[i + 6];
                            color.r = static_cast<Uint8>(received_msg[i + 7]);
                            color.g = static_cast<Uint8>(received_msg[i + 8]);
                            color.b = static_cast<Uint8>(received_msg[i + 9]);
                            color.a = static_cast<Uint8>(received_msg[i + 10]);
                            break;
                        }
                        case END:
                            end = true;
                            break;
                        default:
                            break;
                    }
                    // If message ends break out of the loop
                    if (end) break;
                }
            }
        }
        // Printing the message when the client disconnects to ensure thread is closed
        std::cout << "Client subscription thread closed" << std::endl;
    }


    void mark_destroyed(const int slot) const {
        auto allEntities = gCoordinator.getEntityIds();
        std::string check = std::to_string(slot) + "_";
        for (auto &[key,id]: allEntities) {
            if (key.rfind(check, 0) == 0) {
                gCoordinator.getComponent<Destroy>(id).destroy = true;
                gCoordinator.getComponent<Destroy>(id).isSent = false;
            }
        }
    }

    //Message: [CONNECT, port, slot]
public
:
    void update(zmq::context_t &context, zmq::socket_t &connect_socket) {
        std::array<float, 3> message{};
        if (connect_socket.recv(zmq::buffer(message), zmq::recv_flags::none)) {
            if (message[0] == Message::CONNECT) {
                const int slot = static_cast<int>(message[2]);
                const int port = static_cast<int>(message[1]);
                clients[slot] = std::make_shared<zmq::socket_t>(context, ZMQ_SUB);
                clients[slot]->bind("tcp://localhost:" + std::to_string(port));
                clients[slot]->set(zmq::sockopt::subscribe, "");
                connected[slot].store(true);
                threads[slot] = std::thread(&ServerSystem::listen_client, this, std::ref(*clients[slot]), slot);
                std::array<float, 2> response = {Message::CONNECTED, static_cast<float>(slot)};
                connect_socket.send(zmq::buffer(response), zmq::send_flags::none);
                std::cout << "Client connected: " << slot << std::endl;
            } else if (message[0] == Message::DISCONNECT) {
                const int slot = static_cast<int>(message[2]);
                connected[slot].store(false);
                threads[slot].join();
                clients[slot]->close();
                clients.erase(slot);
                mark_destroyed(slot);
                std::array<float, 1> response = {Message::DISCONNECTED};
                connect_socket.send(zmq::buffer(response), zmq::send_flags::none);
                std::cout << "Client disconnected: " << slot << std::endl;
            }
        }
    }
};
