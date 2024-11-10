//
// Created by Utsav Lal on 11/10/24.
//

#pragma once
#include "../ECS/coordinator.hpp"
#include "../ECS/system.hpp"
#include "../EMS/event_coordinator.hpp"

extern Coordinator gCoordinator;
extern EventCoordinator eventCoordinator;

class PositionSystem : public System {
    std::map<Entity, Transform> previous;

public:
    void update() {
        for (auto entity: entities) {
            auto &transform = gCoordinator.getComponent<Transform>(entity);
            if (previous[entity].equal(transform)) {
                continue;
            }
            previous[entity] = transform;
            Event transformChangedEvent{
                EventType::ReplayTransformChanged,
                ReplayTransformData{entity, transform}
            };
            eventCoordinator.emit(std::make_shared<Event>(transformChangedEvent));
        }
    }
};
