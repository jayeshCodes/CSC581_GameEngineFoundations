//
// Created by Utsav Lal on 10/15/24.
//

#pragma once
#include "../ECS/coordinator.hpp"
#include "../ECS/system.hpp"
#include "../model/components.hpp"

extern Coordinator gCoordinator;

class ReceiverSystem : public System {
public:
    void update(zmq::socket_t &socket) {
        zmq::pollitem_t items[] = {{static_cast<void *>(socket), 0, ZMQ_POLLIN, 0}};
        zmq::poll(items, 1, std::chrono::milliseconds(17));
        if (items[0].revents & ZMQ_POLLIN) {
            zmq::message_t entity_id;
            zmq::message_t entity_data;

            socket.recv(entity_id, zmq::recv_flags::none);
            socket.recv(entity_data, zmq::recv_flags::none);

            const std::vector<float> received_msg(static_cast<float *>(entity_data.data()),
                                                  static_cast<float *>(entity_data.data()) + entity_data.size() / sizeof
                                                  (float));

            if (received_msg.empty()) return;

            auto command = static_cast<Message>(received_msg[0]);
            if (command == Message::SYNC) {
                std::cout << "Syncing" << std::endl;
                auto ids = gCoordinator.getEntityIds();
                for (const auto &id: ids) {
                    if (gCoordinator.hasComponent<ClientEntity>(id.second)) {
                        gCoordinator.getComponent<ClientEntity>(id.second).noOfTimes = 5;
                    }
                }
            }
            if (command == Message::UPDATE) {
                const std::string eId = entity_id.to_string();
                auto generatedId = gCoordinator.createEntity(eId);

                gCoordinator.addComponent<Transform>(generatedId, Transform{});
                gCoordinator.addComponent<Color>(generatedId, Color{});

                auto &transform = gCoordinator.getComponent<Transform>(generatedId);
                auto &color = gCoordinator.getComponent<Color>(generatedId);


                transform.x = received_msg[2];
                transform.y = received_msg[3];
                transform.h = received_msg[4];
                transform.w = received_msg[5];
                color.color.r = static_cast<Uint8>(received_msg[6]);
                color.color.g = static_cast<Uint8>(received_msg[7]);
                color.color.b = static_cast<Uint8>(received_msg[8]);
                color.color.a = static_cast<Uint8>(received_msg[9]);
            }
            if (command == Message::DELETE) {
                std::cout << "Destroying" << std::endl;
                std::string eId = entity_id.to_string();
                auto entities = gCoordinator.getEntitiesStartsWith(eId);
                for (auto entity: entities) {
                    auto generatedId = gCoordinator.getEntityIds()[eId];
                    gCoordinator.addComponent<Destroy>(generatedId, Destroy{});
                    gCoordinator.getComponent<Destroy>(generatedId).destroy = true;
                }
            }
        }
    }
};
