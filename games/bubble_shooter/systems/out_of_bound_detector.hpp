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

class OutOfBoundsDetectorSystem : public System {
public:
    void update(float screen_width, float screen_height) {
        for(auto &entity: entities) {
            auto &transform = gCoordinator.getComponent<Transform>(entity);
            if(transform.x < 0) {
                Event outOfBoundsEvent{eventTypeToString(GameEvents::OutOfBounds), GameEvents::OutOfBoundsData{entity, GameEvents::OutOfBoundsData::Direction::Left}};
                eventCoordinator.emit(std::make_shared<Event>(outOfBoundsEvent));
            }
            if(transform.x + transform.w > screen_width) {
                Event outOfBoundsEvent{eventTypeToString(GameEvents::OutOfBounds), GameEvents::OutOfBoundsData{entity, GameEvents::OutOfBoundsData::Direction::Right}};
                eventCoordinator.emit(std::make_shared<Event>(outOfBoundsEvent));
            }
            if(transform.y < 0) {
                Event outOfBoundsEvent{eventTypeToString(GameEvents::OutOfBounds), GameEvents::OutOfBoundsData{entity, GameEvents::OutOfBoundsData::Direction::Top}};
                eventCoordinator.emit(std::make_shared<Event>(outOfBoundsEvent));
            }
            if(transform.y + transform.h > screen_height) {
                Event outOfBoundsEvent{eventTypeToString(GameEvents::OutOfBounds), GameEvents::OutOfBoundsData{entity, GameEvents::OutOfBoundsData::Direction::Bottom}};
                eventCoordinator.emit(std::make_shared<Event>(outOfBoundsEvent));
            }
        }
    }
};
