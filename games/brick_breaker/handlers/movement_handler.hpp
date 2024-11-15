//
// Created by Utsav Lal on 11/13/24.
//

#pragma once
#include "../../../lib/ECS/coordinator.hpp"
#include "../../../lib/ECS/system.hpp"
#include "../../../lib/EMS/event_coordinator.hpp"
#include "../../../lib/EMS/event_manager.hpp"
#include "../model/events.hpp"

extern Coordinator gCoordinator;
extern EventCoordinator eventCoordinator;

class MovementHandler : public System {
    EventHandler movementHandler = [this](const std::shared_ptr<Event> &event) {
        if (event->type == GameEvents::eventTypeToString(GameEvents::MoveLeft)) {
            for (const auto entity: entities) {
                auto &kinematic = gCoordinator.getComponent<CKinematic>(entity);
                kinematic.velocity.x = -150.f;
            }
        } else if (event->type == GameEvents::eventTypeToString(GameEvents::MoveRight)) {
            for (auto entity: entities) {
                auto &kinematic = gCoordinator.getComponent<CKinematic>(entity);
                kinematic.velocity.x = 150.f;
            }
        } else if (event->type == GameEvents::eventTypeToString(GameEvents::Stop)) {
            for (const auto entity: entities) {
                auto &kinematic = gCoordinator.getComponent<CKinematic>(entity);
                kinematic.velocity.x = 0.f;
            }
        }
    };

public:
    MovementHandler() {
        eventCoordinator.subscribe(movementHandler, GameEvents::eventTypeToString(GameEvents::MoveLeft));
        eventCoordinator.subscribe(movementHandler, GameEvents::eventTypeToString(GameEvents::MoveRight));
        eventCoordinator.subscribe(movementHandler, GameEvents::eventTypeToString(GameEvents::Stop));
    }

    ~MovementHandler() {
        eventCoordinator.unsubscribe(movementHandler, GameEvents::eventTypeToString(GameEvents::MoveLeft));
        eventCoordinator.unsubscribe(movementHandler, GameEvents::eventTypeToString(GameEvents::MoveRight));
        eventCoordinator.unsubscribe(movementHandler, GameEvents::eventTypeToString(GameEvents::Stop));
    }
};
