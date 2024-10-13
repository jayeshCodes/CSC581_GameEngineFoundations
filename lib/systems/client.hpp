//
// Created by Utsav Lal on 10/7/24.
//

#ifndef CLIENT_HPP
#define CLIENT_HPP
#include <zmq.hpp>
#include <iostream>

#include "../enum/message_type.hpp"

extern Coordinator gCoordinator;

class ClientSystem : public System {
    bool connected = false;
    const int MESSAGE_SIZE = 11;

public:
    void connect(zmq::socket_t &connect_socket, zmq::socket_t &pub_socket, const int pub_port, const int slot) {
        std::array<float, 3> message = {Message::CONNECT, static_cast<float>(pub_port), static_cast<float>(slot)};
        connect_socket.send(zmq::buffer(message), zmq::send_flags::none);

        if (std::array<float, 2> response{}; connect_socket.recv(zmq::buffer(response), zmq::recv_flags::none)) {
            if (response[0] == Message::CONNECTED) {
                pub_socket.connect("tcp://localhost:" + std::to_string(pub_port));
            }
            std::cout << "Connected to server" << std::endl;
            connected = true;
        }
    }

    void disconnect(zmq::socket_t &connect_socket, zmq::socket_t &pub_socket, const int slot) {
        std::array<float, 3> message = {Message::DISCONNECT, -1, static_cast<float>(slot)};
        connect_socket.send(zmq::buffer(message), zmq::send_flags::none);

        if (std::array<float, 1> response{}; connect_socket.recv(zmq::buffer(response), zmq::recv_flags::none)) {
            if (response[0] == Message::DISCONNECTED) {
                connected = false;
                std::cout << "Disconnected from server" << std::endl;
                return;
            }
        }
        std::cout << "Failed to disconnect from server" << std::endl;
    }


    void update(zmq::socket_t &sub_socket, const int client_slot) {
        if (!connected) {
            std::cout << "Not connected to server" << std::endl;
            return;
        }
        zmq::message_t message;
        if (sub_socket.recv(message, zmq::recv_flags::none)) {
            const std::vector<float> received_msg(static_cast<float *>(message.data()),
                                                  static_cast<float *>(message.data()) + message.size() / sizeof(
                                                      float));

            for (int i = 0; i < received_msg.size(); i += MESSAGE_SIZE) {
                if (received_msg[i] == Message::END) {
                    break;
                }
                auto entity = static_cast<Entity>(received_msg[i + 2]);
                auto slot = static_cast<int>(received_msg[i + 1]);
                if (slot == client_slot) {
                    continue;
                }
                const std::string key = std::to_string(slot) + "_" + Coordinator::createKey(entity);

                switch (static_cast<Message>(received_msg[i])) {
                    case CONNECT:
                    case CREATE:
                    case DISCONNECT:
                        break;
                    case DESTROY: {
                        const Entity generatedId = gCoordinator.createEntity(key);
                        gCoordinator.addComponent<Destroy>(generatedId, Destroy{});
                        gCoordinator.getComponent<Destroy>(generatedId).destroy = true;
                        gCoordinator.getComponent<Destroy>(generatedId).isSent = true;
                        break;
                    }
                    case ATTACH:
                    case POSITION:
                        break;
                    case UPDATE: {
                        const Entity generatedId = gCoordinator.createEntity(key);
                        gCoordinator.addComponent<Transform>(generatedId, Transform{});
                        gCoordinator.addComponent<Color>(generatedId, Color{});
                        auto &[x, y, h, w, orientation, scale] = gCoordinator.getComponent<Transform>(generatedId);
                        auto &[color] = gCoordinator.getComponent<Color>(generatedId);
                        x = received_msg[i + 3];
                        y = received_msg[i + 4];
                        w = received_msg[i + 5];
                        h = received_msg[i + 6];
                        color.r = static_cast<Uint8>(received_msg[i + 7]);
                        color.g = static_cast<Uint8>(received_msg[i + 8]);
                        color.b = static_cast<Uint8>(received_msg[i + 9]);
                        color.a = static_cast<Uint8>(received_msg[i + 10]);
                    }
                    case END:
                    case CONNECTED:
                    case DISCONNECTED:
                        break;
                }
            }
        }
    }
};
#endif //CLIENT_HPP
