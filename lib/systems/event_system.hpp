//
// Created by Utsav Lal on 10/29/24.
//

#pragma once
#include "../ECS/coordinator.hpp"
#include "../ECS/system.hpp"
#include "../EMS/event_coordinator.hpp"


extern Coordinator gCoordinator;
extern EventCoordinator eventCoordinator;
extern Timeline eventTimeline;

class EventSystem : public System {

public:
    void update() {
        // Process events in the queue
        eventCoordinator.processEventsInQueue(eventTimeline.getElapsedTime());
    }
};
