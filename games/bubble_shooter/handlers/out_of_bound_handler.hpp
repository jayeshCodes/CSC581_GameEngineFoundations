//
// Created by Utsav Lal on 11/13/24.
//

#pragma once

#include "../../../lib/ECS/coordinator.hpp"
#include "../../../lib/ECS/system.hpp"
#include "../../../lib/EMS/event_coordinator.hpp"
#include "../../../lib/EMS/types.hpp"
#include "../model/events.hpp"


extern Coordinator gCoordinator;
extern EventCoordinator eventCoordinator;

class OutOfBoundHandler : public System {
    EventHandler outOfBoundHandler = [this](const std::shared_ptr<Event> &event) {
        if (event->type == GameEvents::eventTypeToString(GameEvents::OutOfBounds)) {
            GameEvents::OutOfBoundsData data = (event->data);
            if (!gCoordinator.hasComponent<CKinematic>(data.entity)) return;
            auto &kinematic = gCoordinator.getComponent<CKinematic>(data.entity);

            if (gCoordinator.hasComponent<Ball>(data.entity)) {
                if (data.direction == GameEvents::OutOfBoundsData::Direction::Left) {
                    kinematic.velocity.x *= -1;
                } else if (data.direction == GameEvents::OutOfBoundsData::Direction::Right) {
                    kinematic.velocity.x *= -1;
                } else if (data.direction == GameEvents::OutOfBoundsData::Direction::Top) {
                    kinematic.velocity.y *= -1;
                } else if (data.direction == GameEvents::OutOfBoundsData::Direction::Bottom) {
                }
            }

            if (gCoordinator.hasComponent<Launcher>(data.entity)) {
                if (data.direction == GameEvents::OutOfBoundsData::Direction::Left) {
                    kinematic.velocity.x = std::fmax(0.f, kinematic.velocity.x);
                } else if (data.direction == GameEvents::OutOfBoundsData::Direction::Right) {
                    kinematic.velocity.x = std::fmin(0.f, kinematic.velocity.x);
                }
            }
        }
    };

public:
    OutOfBoundHandler() {
        eventCoordinator.subscribe(outOfBoundHandler, GameEvents::eventTypeToString(GameEvents::OutOfBounds));
    }

    ~OutOfBoundHandler() {
        eventCoordinator.unsubscribe(outOfBoundHandler, GameEvents::eventTypeToString(GameEvents::OutOfBounds));
    }
};
