//
// Created by Utsav Lal on 11/17/24.
//

#pragma once

#include "../../../lib/ECS/coordinator.hpp"
#include "../../../lib/ECS/system.hpp"
#include "../../../lib/EMS/event_coordinator.hpp"
#include "../../../lib/EMS/types.hpp"
#include "../model/event.hpp"


extern Coordinator gCoordinator;
extern EventCoordinator eventCoordinator;

class OOBDetectorSystem : public System {
public:
    void update(float screen_width, float screen_height) {
        for(auto &entity: entities) {
            if(!gCoordinator.hasComponent<Transform>(entity)) return;
            auto &transform = gCoordinator.getComponent<Transform>(entity);
            if(transform.x < 0) {
                Event outOfBoundsEvent{eventTypeToString(GameEvents::GameEnd), {}};
                eventCoordinator.emit(std::make_shared<Event>(outOfBoundsEvent));
            }
            if(transform.x + transform.w > screen_width) {
                Event outOfBoundsEvent{eventTypeToString(GameEvents::GameEnd), {}};
                eventCoordinator.emit(std::make_shared<Event>(outOfBoundsEvent));
            }
            if(transform.y < 0) {
                Event outOfBoundsEvent{eventTypeToString(GameEvents::GameEnd), {}};
                eventCoordinator.emit(std::make_shared<Event>(outOfBoundsEvent));
            }
            if(transform.y + transform.h > screen_height) {
                Event outOfBoundsEvent{eventTypeToString(GameEvents::GameEnd), {}};
                eventCoordinator.emit(std::make_shared<Event>(outOfBoundsEvent));
            }
        }
    }
};
