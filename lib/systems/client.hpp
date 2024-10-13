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
public:
    void update(zmq::socket_t &socket) {
        zmq::message_t message;
        if (socket.recv(message, zmq::recv_flags::none)) {
            const std::vector<float> received_msg(static_cast<float *>(message.data()),
                                                  static_cast<float *>(message.data()) + message.size() / sizeof(
                                                      float));

            for (int i = 0; i < received_msg.size(); i += 10) {
                if (received_msg[i] == Message::END) {
                    break;
                }
                auto entity = static_cast<Entity>(received_msg[i + 1]);
                const Entity generatedId = gCoordinator.createEntity(Coordinator::createKey(entity));
                gCoordinator.addComponent<Transform>(generatedId, Transform{});
                gCoordinator.addComponent<Color>(generatedId, Color{});
                auto &[x, y, h, w, orientation, scale] = gCoordinator.getComponent<Transform>(generatedId);
                auto &[color] = gCoordinator.getComponent<Color>(generatedId);
                x = received_msg[i + 2];
                y = received_msg[i + 3];
                w = received_msg[i + 4];
                h = received_msg[i + 5];
                color.r = static_cast<Uint8>(received_msg[i + 6]);
                color.g = static_cast<Uint8>(received_msg[i + 7]);
                color.b = static_cast<Uint8>(received_msg[i + 8]);
                color.a = static_cast<Uint8>(received_msg[i + 9]);
            }
        }
    }
};
#endif //CLIENT_HPP
