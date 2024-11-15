//
// Created by Utsav Lal on 11/14/24.
//

#pragma once

#include "../../../lib/ECS/coordinator.hpp"
#include "../../../lib/ECS/system.hpp"
#include "../../../lib/EMS/event_coordinator.hpp"
#include "../../../lib/model/event.hpp"
#include "../model/component.hpp"
#include "../model/events.hpp"

extern Coordinator gCoordinator;
extern EventCoordinator eventCoordinator;

class GameStateChecker : public System {
public:
    void update() {
        if(entities.empty()) {
            // Game over
            std::cout << "Game Over" << std::endl;
        }
    }
};
