//
// Created by Utsav Lal on 10/15/24.
//

#pragma once
#include "../ECS/coordinator.hpp"
#include "../ECS/system.hpp"
#include "../helpers/network_helper.hpp"
#include "../model/components.hpp"
#include "../strategy/send_strategy.hpp"

extern Coordinator gCoordinator;

class ReceiverSystem : public System {
    bool testStarted = false;

private:
    void handleNormalMessage(Send_Strategy *send_strategy, zmq::message_t &copy, std::string &entity_id) {
        try {
            SimpleMessage receivedMessage = send_strategy->parse_message(copy);
            if (receivedMessage.type == Message::SYNC) {
                std::cout << "Syncing" << std::endl;
                auto ids = gCoordinator.getEntityIds();
                for (const auto &id: ids) {
                    if (gCoordinator.hasComponent<ClientEntity>(id.second)) {
                        gCoordinator.getComponent<ClientEntity>(id.second).noOfTimes = 5;
                    }
                }
            }
            if (receivedMessage.type == Message::UPDATE) {
                auto generatedId = gCoordinator.createEntity(receivedMessage.entity_key);

                auto received_transform = std::get<Transform>(receivedMessage.components[0]);

                if (!gCoordinator.hasComponent<Transform>(generatedId)) { return; }

                auto &transform = gCoordinator.getComponent<Transform>(generatedId);

                transform = received_transform;
            }
            if (receivedMessage.type == Message::DELETE) {
                auto entities = gCoordinator.getEntitiesStartsWith(receivedMessage.entity_key);
                for (auto entity: entities) {
                    auto generatedId = gCoordinator.getEntityIds()[receivedMessage.entity_key];
                    gCoordinator.addComponent<Destroy>(generatedId, Destroy{});
                    gCoordinator.getComponent<Destroy>(generatedId).destroy = true;
                }
            }
            if (receivedMessage.type == Message::CREATE) {
                auto generatedId = gCoordinator.createEntity(receivedMessage.entity_key);
                for (auto &component: receivedMessage.components) {
                    if (std::holds_alternative<Transform>(component)) {
                        auto received_transform = std::get<Transform>(component);
                        gCoordinator.addComponent<Transform>(generatedId, received_transform);
                    }
                    if (std::holds_alternative<Color>(component)) {
                        auto received_color = std::get<Color>(component);
                        gCoordinator.addComponent<Color>(generatedId, received_color);
                    }
                    if (std::holds_alternative<RigidBody>(component)) {
                        auto received_rigidBody = std::get<RigidBody>(component);
                        gCoordinator.addComponent<RigidBody>(generatedId, received_rigidBody);
                    }
                    if (std::holds_alternative<Collision>(component)) {
                        auto received_collision = std::get<Collision>(component);
                        gCoordinator.addComponent<Collision>(generatedId, received_collision);
                    }
                    if (std::holds_alternative<CKinematic>(component)) {
                        auto received_kinematic = std::get<CKinematic>(component);
                        gCoordinator.addComponent<CKinematic>(generatedId, CKinematic{});
                    }
                    if (std::holds_alternative<Destroy>(component)) {
                        auto received_destroy = std::get<Destroy>(component);
                        gCoordinator.addComponent<Destroy>(generatedId, received_destroy);
                    }
                }
            }
        } catch (std::exception &e) {
            std::string jsonString(static_cast<char *>(copy.data()), copy.size());
            std::cerr << "Error parsing message: " << jsonString << std::endl;
        }
    }

    static void handleEventMessage(const zmq::message_t &copy) {
        const nlohmann::json eventJson = nlohmann::json::parse(copy.to_string());
        auto event = std::make_shared<Event>();
        from_json(eventJson, *event);
        eventCoordinator.emit(event);
    }

public:
    void update(zmq::socket_t &socket, Send_Strategy *send_strategy) {
        zmq::pollitem_t items[] = {{static_cast<void *>(socket), 0, ZMQ_POLLIN, 0}};
        zmq::poll(items, 1, std::chrono::milliseconds(17));
        if (items[0].revents & ZMQ_POLLIN) {
            zmq::message_t copy;
            std::string entity_id;
            NetworkHelper::receiveMessageClient(socket, copy, entity_id);
            if (entity_id == NetworkHelper::EVENT_ENTITY_ID) {
                handleEventMessage(copy);
            } else {
                handleNormalMessage(send_strategy, copy, entity_id);
            }
        }
    }
};
