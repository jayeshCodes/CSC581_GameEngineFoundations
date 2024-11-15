//
// Created by Utsav Lal on 11/13/24.
//

#pragma once

#include <iostream>

#include "../../../lib/ECS/system.hpp"
#include "../../../lib/helpers/network_helper.hpp"
#include "../../../lib/strategy/send_strategy.hpp"
#include "../model/component.hpp"
#include "../model/data_model.hpp"

class ReceiverSystem : public System {

    void handleMessage(Send_Strategy *send_strategy, zmq::message_t &copy, std::string &entity_id) {
        try {
            BBMessage receivedMessage = send_strategy->parse_message(copy);
            if (receivedMessage.type == Message::CREATE) {
                auto generatedId = gCoordinator.createEntity(receivedMessage.entity_key);
                for (auto &component: receivedMessage.components) {
                    std::string component_type = component["type"];
                    if (component_type == "Transform") {
                        auto received_transform = component.get<Transform>();
                        gCoordinator.addComponent<Transform>(generatedId, received_transform);
                    }
                    else if (component_type == "Color") {
                        auto received_color = component.get<Color>();
                        gCoordinator.addComponent<Color>(generatedId, received_color);
                    }
                    else if (component_type == "RigidBody") {
                        auto received_rigidbody = component.get<RigidBody>();
                        gCoordinator.addComponent<RigidBody>(generatedId, received_rigidbody);
                    }
                    else if (component_type == "Collision") {
                        auto received_collision = component.get<Collision>();
                        gCoordinator.addComponent<Collision>(generatedId, received_collision);
                    }
                    else if (component_type == "CKinematic") {
                        auto received_kinematic = component.get<CKinematic>();
                        gCoordinator.addComponent<CKinematic>(generatedId, CKinematic{});
                    }
                    else if (component_type == "Destroy") {
                        auto received_destroy = component.get<Destroy>();
                        gCoordinator.addComponent<Destroy>(generatedId, received_destroy);
                    }
                    else if (component_type == "VerticalBoost") {
                        auto received_boost = component.get<VerticalBoost>();
                        gCoordinator.addComponent<VerticalBoost>(generatedId, received_boost);
                    }
                    else if (component_type == "Brick") {
                        auto received_brick = component.get<Brick>();
                        gCoordinator.addComponent<Brick>(generatedId, received_brick);
                    }

                }
            }
        } catch (std::exception &e) {
            std::string jsonString(static_cast<char *>(copy.data()), copy.size());
            std::cerr << "Error parsing message: " << jsonString << std::endl;
        }
    }

public:
    void update(zmq::socket_t &socket, Send_Strategy *send_strategy) {
        zmq::pollitem_t items[] = {{static_cast<void *>(socket), 0, ZMQ_POLLIN, 0}};
        zmq::poll(items, 1, std::chrono::milliseconds(17));
        if (items[0].revents & ZMQ_POLLIN) {
            zmq::message_t copy;
            std::string entity_id;
            NetworkHelper::receiveMessageClient(socket, copy, entity_id);
            handleMessage(send_strategy, copy, entity_id);
        }
    }
};
