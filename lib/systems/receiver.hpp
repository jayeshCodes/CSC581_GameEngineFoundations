//
// Created by Utsav Lal on 10/15/24.
//

#pragma once
#include "../ECS/coordinator.hpp"
#include "../ECS/system.hpp"
#include "../model/components.hpp"
#include "../strategy/send_strategy.hpp"

extern Coordinator gCoordinator;

class ReceiverSystem : public System {
public:
    void update(zmq::socket_t &socket, Send_Strategy *send_strategy) {
        zmq::pollitem_t items[] = {{static_cast<void *>(socket), 0, ZMQ_POLLIN, 0}};
        zmq::poll(items, 1, std::chrono::milliseconds(17));
        if (items[0].revents & ZMQ_POLLIN) {
            zmq::message_t entity_id;
            zmq::message_t entity_data;

            socket.recv(entity_id, zmq::recv_flags::none);
            socket.recv(entity_data, zmq::recv_flags::none);

            zmq::message_t copy;
            copy.copy(entity_data);

            try {
                auto [command, received_transform, received_color] = send_strategy->parse_message(copy);
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


                    transform = received_transform;
                    color.color = {
                        received_color.color.r, received_color.color.g, received_color.color.b,
                        received_color.color.a
                    };
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
            } catch (std::exception &e) {
                std::string jsonString(static_cast<char*>(copy.data()), copy.size());
                std::cerr << "Error parsing message: " << jsonString << std::endl;
            }
        }
    }
};
