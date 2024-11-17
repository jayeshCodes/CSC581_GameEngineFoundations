//
// Created by Utsav Lal on 11/13/24.
//

#pragma once

#include "../../../lib/ECS/coordinator.hpp"
#include "../../../lib/ECS/system.hpp"
#include "../../../lib/EMS/event_coordinator.hpp"
#include "../../../lib/model/event.hpp"
#include "../model/components.hpp"
#include "../model/event.hpp"

extern Coordinator gCoordinator;
extern EventCoordinator eventCoordinator;

class CollisionHandler : public System {
    EventHandler collisionHandler = [this](const std::shared_ptr<Event> &event) {
        if (event->type == eventTypeToString(EntityCollided)) {
            const EntityCollidedData data = event->data;
            auto &entityA = data.entityA;
            auto &entityB = data.entityB;
            if (gCoordinator.hasComponent<Snake>(entityA) && gCoordinator.hasComponent<Food>(entityB) || gCoordinator.
                hasComponent<Snake>(entityB) && gCoordinator.hasComponent<Food>(entityA)) {
                if (gCoordinator.hasComponent<Snake>(entityA)) {
                    gCoordinator.getComponent<Destroy>(entityB).destroy = true;
                } else {
                    gCoordinator.getComponent<Destroy>(entityA).destroy = true;
                }
                Event foodEatenEvent{GameEvents::eventTypeToString(GameEvents::FoodEaten), {}};
                eventCoordinator.emit(std::make_shared<Event>(foodEatenEvent));
            }
        }
    };

public:
    CollisionHandler() {
        eventCoordinator.subscribe(collisionHandler, eventTypeToString(EntityCollided));
    }

    ~CollisionHandler() {
        eventCoordinator.unsubscribe(collisionHandler, eventTypeToString(EntityCollided));
    }
};
