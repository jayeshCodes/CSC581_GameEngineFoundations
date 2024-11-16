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

class GameStateHandler : public System {
    StateSerializer serializer = [](nlohmann::json &snapshot, Entity &entity) {
        if (gCoordinator.hasComponent<Transform>(entity)) {
            snapshot["c"]["type"] = "Transform";
            snapshot["c"]["v"].emplace_back(gCoordinator.getComponent<Transform>(entity));
        }
        if (gCoordinator.hasComponent<Color>(entity)) {
            snapshot["c"]["type"] = "Color";
            snapshot["c"]["v"].emplace_back(gCoordinator.getComponent<Color>(entity));
        }
        if (gCoordinator.hasComponent<Gravity>(entity)) {
            snapshot["c"]["type"] = "Gravity";
            snapshot["c"]["v"].emplace_back(gCoordinator.getComponent<Gravity>(entity));
        }
        if (gCoordinator.hasComponent<CKinematic>(entity)) {
            snapshot["c"]["type"] = "CKinematic";
            snapshot["c"]["v"].emplace_back(gCoordinator.getComponent<CKinematic>(entity));
        }
        if (gCoordinator.hasComponent<Camera>(entity)) {
            snapshot["c"]["type"] = "Camera";
            snapshot["c"]["v"].emplace_back(gCoordinator.getComponent<Camera>(entity));
        }
        if (gCoordinator.hasComponent<Destroy>(entity)) {
            snapshot["c"]["type"] = "Destroy";
            snapshot["c"]["v"].emplace_back(gCoordinator.getComponent<Destroy>(entity));
        }
        if (gCoordinator.hasComponent<Collision>(entity)) {
            snapshot["c"]["type"] = "Collision";
            snapshot["c"]["v"].emplace_back(gCoordinator.getComponent<Collision>(entity));
        }
        if (gCoordinator.hasComponent<Receiver>(entity)) {
            snapshot["c"]["type"] = "Receiver";
            snapshot["c"]["v"].emplace_back(gCoordinator.getComponent<Receiver>(entity));
        }
        if (gCoordinator.hasComponent<ClientEntity>(entity)) {
            snapshot["c"]["type"] = "ClientEntity";
            snapshot["c"]["v"].emplace_back(gCoordinator.getComponent<ClientEntity>(entity));
        }
        if (gCoordinator.hasComponent<KeyboardMovement>(entity)) {
            snapshot["c"]["type"] = "KeyboardMovement";
            snapshot["c"]["v"].emplace_back(gCoordinator.getComponent<KeyboardMovement>(entity));
        }
        if (gCoordinator.hasComponent<Ball>(entity)) {
            snapshot["c"]["type"] = "Ball";
            snapshot["c"]["v"].emplace_back(gCoordinator.getComponent<Ball>(entity));
        }
        if (gCoordinator.hasComponent<Brick>(entity)) {
            snapshot["c"]["type"] = "Brick";
            snapshot["c"]["v"].emplace_back(gCoordinator.getComponent<Brick>(entity));
        }
        if (gCoordinator.hasComponent<Launcher>(entity)) {
            snapshot["c"]["type"] = "Launcher";
            snapshot["c"]["v"].emplace_back(gCoordinator.getComponent<Launcher>(entity));
        }
    };

    StateDeserializer deserializer = [](nlohmann::json &entitySnap, Entity &entity) {
        for (auto &component: entitySnap["c"]) {
            if (component["type"] == "Transform") {
                gCoordinator.addComponent(entity, component["v"]);
                auto &transform = gCoordinator.getComponent<Transform>(entity);
                transform = component["v"];
            }
            if (component["type"] == "Color") {
                gCoordinator.addComponent(entity, component["v"]);
                auto &color = gCoordinator.getComponent<Color>(entity);
                color = component["v"];
            }
            if (component["type"] == "Gravity") {
                gCoordinator.addComponent(entity, component["v"]);
                auto &gravity = gCoordinator.getComponent<Gravity>(entity);
                gravity = component["v"];
            }
            if (component["type"] == "CKinematic") {
                gCoordinator.addComponent(entity, component["v"]);
                auto &cKinematic = gCoordinator.getComponent<CKinematic>(entity);
                cKinematic = component["v"];
            }
            if (component["type"] == "Camera") {
                gCoordinator.addComponent(entity, component["v"]);
                auto &camera = gCoordinator.getComponent<Camera>(entity);
                camera = component["v"];
            }
            if (component["type"] == "Destroy") {
                gCoordinator.addComponent(entity, component["v"]);
                auto &destroy = gCoordinator.getComponent<Destroy>(entity);
                destroy = component["v"];
            }
            if (component["type"] == "Collision") {
                gCoordinator.addComponent(entity, component["v"]);
                auto &collision = gCoordinator.getComponent<Collision>(entity);
                collision = component["v"];
            }
            if (component["type"] == "Receiver") {
                gCoordinator.addComponent(entity, component["v"]);
                auto &receiver = gCoordinator.getComponent<Receiver>(entity);
                receiver = component["v"];
            }
            if (component["type"] == "ClientEntity") {
                gCoordinator.addComponent(entity, component["v"]);
                auto &clientEntity = gCoordinator.getComponent<ClientEntity>(entity);
                clientEntity = component["v"];
            }
            if (component["type"] == "KeyboardMovement") {
                gCoordinator.addComponent(entity, component["v"]);
                auto &keyboardMovement = gCoordinator.getComponent<KeyboardMovement>(entity);
                keyboardMovement = component["v"];
            }
            if (component["type"] == "Ball") {
                gCoordinator.addComponent(entity, component["v"]);
                auto &ball = gCoordinator.getComponent<Ball>(entity);
                ball = component["v"];
            }
            if (component["type"] == "Brick") {
                gCoordinator.addComponent(entity, component["v"]);
                auto &brick = gCoordinator.getComponent<Brick>(entity);
                brick = component["v"];
            }
            if (component["type"] == "Launcher") {
                gCoordinator.addComponent(entity, component["v"]);
                auto &launcher = gCoordinator.getComponent<Launcher>(entity);
                launcher = component["v"];
            }
        }
    };

    EventHandler gameStartHandler = [this](const std::shared_ptr<Event> &event) {
        if (event->type == GameEvents::eventTypeToString(GameEvents::GameStart)) {
            gCoordinator.backup(serializer);
            for (const auto entity: entities) {
                auto &ball = gCoordinator.getComponent<Ball>(entity);
                ball.isLaunched = true;
            }
        }
    };

    EventHandler gameEndHandler = [this](const std::shared_ptr<Event> &event) {
        if (event->type == GameEvents::eventTypeToString(GameEvents::GameOver)) {
            gCoordinator.restore(deserializer);
        }
    };
};
