//
// Created by Utsav Lal on 11/13/24.
//

#pragma once
#include "../../../lib/ECS/coordinator.hpp"
#include "../../../lib/ECS/system.hpp"
#include "../../../lib/EMS/event_coordinator.hpp"
#include "../../../lib/EMS/event_manager.hpp"
#include "../model/component.hpp"
#include "../model/events.hpp"

extern Coordinator gCoordinator;
extern EventCoordinator eventCoordinator;

class LaunchHandler : public System {
    EventHandler launchHandler = [this](const std::shared_ptr<Event> &event) {
        if (event->type == GameEvents::eventTypeToString(GameEvents::Launch)) {
            for (const auto entity: entities) {
                auto &ball = gCoordinator.getComponent<Ball>(entity);
                auto &kinematic = gCoordinator.getComponent<CKinematic>(entity);
                ball.isLaunched = true;
                kinematic.velocity.y = -150.f;
                if (gCoordinator.hasComponent<KeyboardMovement>(entity)) {
                    gCoordinator.removeComponent<KeyboardMovement>(entity);
                }
            }
        }
    };

public:
    LaunchHandler() {
        eventCoordinator.subscribe(launchHandler, GameEvents::eventTypeToString(GameEvents::Launch));
    }

    ~LaunchHandler() {
        eventCoordinator.unsubscribe(launchHandler, GameEvents::eventTypeToString(GameEvents::Launch));
    }
};
