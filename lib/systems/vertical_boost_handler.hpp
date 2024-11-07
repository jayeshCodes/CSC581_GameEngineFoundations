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
        if (event->type == EventType::EntityTriggered) {
            const auto &data = std::get<EntityTriggeredData>(event->data);
            auto &triggerEntity = data.triggerEntity;
            auto &otherEntity = data.otherEntity;

            if (gCoordinator.getEntityKey(otherEntity) != mainCharID) return;

            if (gCoordinator.hasComponent<CKinematic>(otherEntity) && gCoordinator.hasComponent<VerticalBoost>(
                    triggerEntity)) {
                auto &kinematic = gCoordinator.getComponent<CKinematic>(otherEntity);
                auto &[velocity] = gCoordinator.getComponent<VerticalBoost>(triggerEntity);
                kinematic.velocity.y = velocity;
            }
        }
    };

public:
    VerticalBoostHandler() {
        eventCoordinator.subscribe(triggerHandler, EventType::EntityTriggered);
    }

    ~VerticalBoostHandler() {
        eventCoordinator.unsubscribe(triggerHandler, EventType::EntityTriggered);
    }
};
