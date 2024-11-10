//
// Created by Utsav Lal on 11/3/24.
//

#pragma once
#include "event_system.hpp"
#include "../ECS/coordinator.hpp"
#include "../ECS/system.hpp"
#include "../EMS/event_coordinator.hpp"
#include "../strategy/send_strategy.hpp"

extern Coordinator gCoordinator;
extern EventCoordinator eventCoordinator;


class EntityCreatedHandler : public System {
    EventHandler collisionHandler = [this](const std::shared_ptr<Event> &event) {
        if (event->type == EventType::MainCharCreated) {
            const auto &data = std::get<MainCharCreatedData>(event->data);
            const nlohmann::json received_msg = nlohmann::json::parse(data.message);
            SimpleMessage msg = received_msg;
            auto generatedId = gCoordinator.createEntity(msg.entity_key);
            for (auto &component: msg.components) {
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
                    gCoordinator.addComponent<CKinematic>(generatedId, received_kinematic);
                }
                if (std::holds_alternative<Destroy>(component)) {
                    auto received_destroy = std::get<Destroy>(component);
                    gCoordinator.addComponent<Destroy>(generatedId, received_destroy);
                }

            }
        }
    };

public:
    EntityCreatedHandler() {
        eventCoordinator.subscribe(collisionHandler, EventType::MainCharCreated);
    }

    ~EntityCreatedHandler() {
        eventCoordinator.unsubscribe(collisionHandler, EventType::MainCharCreated);
    }
};
