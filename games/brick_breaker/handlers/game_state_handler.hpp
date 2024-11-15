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

using ACOMPONENTS = std::variant<Transform, Color, Gravity, CKinematic, Respawnable, Camera, Destroy, Collision,
    Receiver, ClientEntity, KeyboardMovement, Ball, Launcher>;

struct BrickBreakerEntitySnapshot {
    Entity entity;
    std::string id;
    std::vector<ACOMPONENTS> components;
};

class GameStateHandler : public System {
    Snapshot snapshot{};

    BrickBreakerEntitySnapshot createSnapshot(const Entity entity, const std::string id) {
        BrickBreakerEntitySnapshot snapshot{};
        snapshot.entity = entity;
        snapshot.id = id;
        if (gCoordinator.hasComponent<Transform>(entity)) {
            snapshot.components.emplace_back(gCoordinator.getComponent<Transform>(entity));
        }
        if (gCoordinator.hasComponent<Color>(entity)) {
            snapshot.components.emplace_back(gCoordinator.getComponent<Color>(entity));
        }
        if (gCoordinator.hasComponent<Gravity>(entity)) {
            snapshot.components.emplace_back(gCoordinator.getComponent<Gravity>(entity));
        }
        if (gCoordinator.hasComponent<CKinematic>(entity)) {
            snapshot.components.emplace_back(gCoordinator.getComponent<CKinematic>(entity));
        }
        if (gCoordinator.hasComponent<Respawnable>(entity)) {
            snapshot.components.emplace_back(gCoordinator.getComponent<Respawnable>(entity));
        }
        if (gCoordinator.hasComponent<Camera>(entity)) {
            snapshot.components.emplace_back(gCoordinator.getComponent<Camera>(entity));
        }
        if (gCoordinator.hasComponent<Destroy>(entity)) {
            snapshot.components.emplace_back(gCoordinator.getComponent<Destroy>(entity));
        }
        if (gCoordinator.hasComponent<Collision>(entity)) {
            snapshot.components.emplace_back(gCoordinator.getComponent<Collision>(entity));
        }
        if (gCoordinator.hasComponent<Receiver>(entity)) {
            snapshot.components.emplace_back(gCoordinator.getComponent<Receiver>(entity));
        }
        if (gCoordinator.hasComponent<ClientEntity>(entity)) {
            snapshot.components.emplace_back(gCoordinator.getComponent<ClientEntity>(entity));
        }
        if (gCoordinator.hasComponent<KeyboardMovement>(entity)) {
            snapshot.components.emplace_back(gCoordinator.getComponent<KeyboardMovement>(entity));
        }
        if (gCoordinator.hasComponent<Ball>(entity)) {
            snapshot.components.emplace_back(gCoordinator.getComponent<Ball>(entity));
        }
        if (gCoordinator.hasComponent<Launcher>(entity)) {
            snapshot.components.emplace_back(gCoordinator.getComponent<Launcher>(entity));
        }
        return snapshot;
    }

    void backup() {
        this->snapshot.clear();
        for (auto &[id, entity]: entities) {
            this->snapshot.emplace_back(createSnapshot(entity, id));
        }
    }

    EventHandler gameStartHandler = [this](const std::shared_ptr<Event> &event) {
        if (event->type == GameEvents::eventTypeToString(GameEvents::GameStart)) {
            for (const auto entity: entities) {
                auto &ball = gCoordinator.getComponent<Ball>(entity);
                ball.isLaunched = true;
            }
        }
    };
};
