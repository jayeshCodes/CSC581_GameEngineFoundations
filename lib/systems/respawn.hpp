//
// Created by Utsav Lal on 10/28/24.
//

#pragma once
#include "../ECS/coordinator.hpp"
#include "../ECS/system.hpp"
#include "../EMS/event_coordinator.hpp"

extern Coordinator gCoordinator;
extern EventCoordinator eventCoordinator;

class RespawnSystem : public System {
private:
    EventHandler respawnHandler = [this](const std::shared_ptr<Event> &event) {
        if (event->type == EventType::EntityDeath) {
            const auto &data = std::get<EntityDeathData>(event->data);
            auto entity = data.entity;
            auto &transform = gCoordinator.getComponent<Transform>(entity);
            auto &kinematic = gCoordinator.getComponent<CKinematic>(entity);
            auto respawnTransform = data.respawnPosition;
            transform = respawnTransform;
            kinematic.velocity = {0, 0}; // Reset velocity on respawn
            kinematic.acceleration = {0, 0}; // Reset acceleration on respawn

            // Emit respawn event
            Event respawnEvent{EntityRespawn, EntityRespawnData{entity}};
            eventCoordinator.emit(std::make_shared<Event>(respawnEvent));
        }
    };

public:
    RespawnSystem() {
        eventCoordinator.subscribe(respawnHandler, EventType::EntityDeath);
    }

    ~RespawnSystem() {
        eventCoordinator.unsubscribe(respawnHandler, EventType::EntityDeath);
    }
};
