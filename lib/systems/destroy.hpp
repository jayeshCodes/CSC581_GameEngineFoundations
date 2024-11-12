//
// Created by Utsav Lal on 10/13/24.
//

#pragma once

#include "../ECS/coordinator.hpp"
#include "../ECS/system.hpp"
#include "../model/components.hpp"

extern Coordinator gCoordinator;

class DestroySystem : public System {
    EventHandler destroyHandler = [this](const std::shared_ptr<Event> &event) {
        if (event->type == eventTypeToString(EventType::EntityDestroyed)) {
            const EntityDestroyedData data = event->data;
            gCoordinator.destroyEntity(data.entity);
        }
    };

public:
    DestroySystem() {
        eventCoordinator.subscribe(destroyHandler, eventTypeToString(EventType::EntityDestroyed));
    }

    ~DestroySystem() {
        eventCoordinator.unsubscribe(destroyHandler, eventTypeToString(EventType::EntityDestroyed));
    }


    void update() const {
        std::vector<Entity> entitiesToDestroy;

        for (const auto entity: entities) {
            if (auto &[slot, destroyed, isSent] = gCoordinator.getComponent<Destroy>(entity); destroyed) {
                entitiesToDestroy.push_back(entity);
            }
        }

        for (const auto entity: entitiesToDestroy) {
            gCoordinator.destroyEntity(entity);
        }
    }
};
