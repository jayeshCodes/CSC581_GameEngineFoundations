//
// Created by Utsav Lal on 10/13/24.
//

#pragma once

#include "../ECS/coordinator.hpp"
#include "../ECS/system.hpp"
#include "../model/components.hpp"

extern Coordinator gCoordinator;
extern EventCoordinator eventCoordinator;

class DestroySystem : public System {
    EventHandler entityCreatedHandler = [](const std::shared_ptr<Event> &event) {
        if (event->type == EventType::EntityCreated) {
            const auto entityCreatedEvent = std::get<std::string>(event->data);
            std::cout << "Entity created: " << entityCreatedEvent << std::endl;
        }
    };

public:
    DestroySystem() {
        eventCoordinator.subscribe(entityCreatedHandler, EventType::EntityCreated);
    }

    ~DestroySystem() {
        eventCoordinator.unsubscribe(entityCreatedHandler, EventType::EntityCreated);
    }
};
