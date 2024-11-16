//
// Created by Utsav Lal on 11/16/24.
//

#pragma once
#include "../.././../lib/ECS/coordinator.hpp"
#include "../.././../lib/ECS/system.hpp"
#include "../.././../lib/EMS/event_coordinator.hpp"
#include "../model/event.hpp"

extern Timeline gameTimeline;
extern Coordinator gCoordinator;
extern EventCoordinator eventCoordinator;

class MovementHandler : public System {
    EventHandler movementHandler = [this](const std::shared_ptr<Event> &event) {
        if (event->type == GameEvents::eventTypeToString(GameEvents::Left)) {
            for (const auto entity: entities) {
                auto &kinematic = gCoordinator.getComponent<CKinematic>(entity);
                kinematic.velocity.x = -150.f;
                kinematic.velocity.y = 0;
            }
        } else if (event->type == GameEvents::eventTypeToString(GameEvents::Right)) {
            for (auto entity: entities) {
                auto &kinematic = gCoordinator.getComponent<CKinematic>(entity);
                kinematic.velocity.x = 150.f;
                kinematic.velocity.y = 0;
            }
        } else if (event->type == GameEvents::eventTypeToString(GameEvents::Top)) {
            for (const auto entity: entities) {
                auto &kinematic = gCoordinator.getComponent<CKinematic>(entity);
                kinematic.velocity.x = 0.f;
                kinematic.velocity.y = -150.f;
            }
        } else if (event->type == GameEvents::eventTypeToString(GameEvents::Down)) {
            for (const auto entity: entities) {
                auto &kinematic = gCoordinator.getComponent<CKinematic>(entity);
                kinematic.velocity.x = 0.f;
                kinematic.velocity.y = 150.f;
            }
        }
    };

public:
    MovementHandler() {
        eventCoordinator.subscribe(movementHandler, GameEvents::eventTypeToString(GameEvents::Left));
        eventCoordinator.subscribe(movementHandler, GameEvents::eventTypeToString(GameEvents::Right));
        eventCoordinator.subscribe(movementHandler, GameEvents::eventTypeToString(GameEvents::Top));
        eventCoordinator.subscribe(movementHandler, GameEvents::eventTypeToString(GameEvents::Down));
    }

    ~MovementHandler() {
        eventCoordinator.unsubscribe(movementHandler, GameEvents::eventTypeToString(GameEvents::Left));
        eventCoordinator.unsubscribe(movementHandler, GameEvents::eventTypeToString(GameEvents::Right));
        eventCoordinator.unsubscribe(movementHandler, GameEvents::eventTypeToString(GameEvents::Top));
        eventCoordinator.unsubscribe(movementHandler, GameEvents::eventTypeToString(GameEvents::Down));
    }
};
