//
// Created by Jayesh Gajbhar on 10/30/24.
//

#pragma once
#include "../ECS/coordinator.hpp"
#include "../EMS/event_coordinator.hpp"
#include "../model/components.hpp"
#include "../ECS/system.hpp"

extern Coordinator gCoordinator;
extern EventCoordinator eventCoordinator;
extern std::string mainCharID;

class VerticalBoostHandler : public System {
    EventHandler triggerHandler = [this](const std::shared_ptr<Event> &event) {
        if (event->type == eventTypeToString(EventType::EntityTriggered)) {
            const EntityTriggeredData &data = event->data;
            auto &triggerEntity = data.triggerEntity;
            auto &otherEntity = data.otherEntity;

            if (gCoordinator.getEntityKey(otherEntity) != mainCharID) return;

            // Get transforms for both entities
            auto &triggerTransform = gCoordinator.getComponent<Transform>(triggerEntity);
            auto &playerTransform = gCoordinator.getComponent<Transform>(otherEntity);
            auto &playerKinematic = gCoordinator.getComponent<CKinematic>(otherEntity);

            // Check if player is above the trigger and moving downward
            bool isAboveTrigger = (playerTransform.y + playerTransform.h) <= (triggerTransform.y + 5.0f);
            // Small tolerance
            bool isMovingDown = playerKinematic.velocity.y > 0;

            if (isAboveTrigger && isMovingDown) {
                if (gCoordinator.hasComponent<CKinematic>(otherEntity) &&
                    gCoordinator.hasComponent<VerticalBoost>(triggerEntity)) {
                    auto &kinematic = gCoordinator.getComponent<CKinematic>(otherEntity);
                    auto &[velocity] = gCoordinator.getComponent<VerticalBoost>(triggerEntity);

                    // Apply vertical boost
                    kinematic.velocity.y = velocity;

                    // Optional: Add visual/audio feedback here
                }
            }
        }
    };

public:
    VerticalBoostHandler() {
        eventCoordinator.subscribe(triggerHandler, eventTypeToString(EventType::EntityTriggered));
    }

    ~VerticalBoostHandler() {
        eventCoordinator.unsubscribe(triggerHandler, eventTypeToString(EventType::EntityTriggered));
    }
};
