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
        snapshot["c"] = nlohmann::json::array();
        if (gCoordinator.hasComponent<Transform>(entity)) {
            nlohmann::json j;
            j["type"] = "Transform";
            j["v"] = gCoordinator.getComponent<Transform>(entity);
            snapshot["c"].emplace_back(j);
        }
        if (gCoordinator.hasComponent<Color>(entity)) {
            nlohmann::json j;
            j["type"] = "Color";
            j["v"] = gCoordinator.getComponent<Color>(entity);
            snapshot["c"].emplace_back(j);
        }
        if (gCoordinator.hasComponent<Gravity>(entity)) {
            nlohmann::json j;
            j["type"] = "Gravity";
            j["v"] = gCoordinator.getComponent<Gravity>(entity);
            snapshot["c"].emplace_back(j);
        }
        if (gCoordinator.hasComponent<CKinematic>(entity)) {
            nlohmann::json j;
            j["type"] = "CKinematic";
            j["v"] = gCoordinator.getComponent<CKinematic>(entity);
            snapshot["c"].emplace_back(j);
        }
        if (gCoordinator.hasComponent<Camera>(entity)) {
            nlohmann::json j;
            j["type"] = "Camera";
            j["v"] = gCoordinator.getComponent<Camera>(entity);
            snapshot["c"].emplace_back(j);
        }
        if (gCoordinator.hasComponent<Destroy>(entity)) {
            nlohmann::json j;
            j["type"] = "Destroy";
            j["v"] = gCoordinator.getComponent<Destroy>(entity);
            snapshot["c"].emplace_back(j);
        }
        if (gCoordinator.hasComponent<Collision>(entity)) {
            nlohmann::json j;
            j["type"] = "Collision";
            j["v"] = gCoordinator.getComponent<Collision>(entity);
            snapshot["c"].emplace_back(j);
        }
        if (gCoordinator.hasComponent<Receiver>(entity)) {
            nlohmann::json j;
            j["type"] = "Receiver";
            j["v"] = gCoordinator.getComponent<Receiver>(entity);
            snapshot["c"].emplace_back(j);
        }
        if (gCoordinator.hasComponent<ClientEntity>(entity)) {
            nlohmann::json j;
            j["type"] = "ClientEntity";
            j["v"] = gCoordinator.getComponent<ClientEntity>(entity);
            snapshot["c"].emplace_back(j);
        }
        if (gCoordinator.hasComponent<KeyboardMovement>(entity)) {
            nlohmann::json j;
            j["type"] = "KeyboardMovement";
            j["v"] = gCoordinator.getComponent<KeyboardMovement>(entity);
            snapshot["c"].emplace_back(j);
        }
        if (gCoordinator.hasComponent<Ball>(entity)) {
            nlohmann::json j;
            j["type"] = "Ball";
            j["v"] = gCoordinator.getComponent<Ball>(entity);
            snapshot["c"].emplace_back(j);
        }
        if (gCoordinator.hasComponent<Brick>(entity)) {
            nlohmann::json j;
            j["type"] = "Brick";
            j["v"] = gCoordinator.getComponent<Brick>(entity);
            snapshot["c"].emplace_back(j);
        }
        if (gCoordinator.hasComponent<Launcher>(entity)) {
            nlohmann::json j;
            j["type"] = "Launcher";
            j["v"] = gCoordinator.getComponent<Launcher>(entity);
            snapshot["c"].emplace_back(j);
        }
        if (gCoordinator.hasComponent<PowerUp>(entity)) {
            nlohmann::json j;
            j["type"] = "PowerUp";
            j["v"] = gCoordinator.getComponent<PowerUp>(entity);
            snapshot["c"].emplace_back(j);
        }
    };

    StateDeserializer deserializer = [](nlohmann::json &entitySnap, Entity &entity) {
        for (auto &component: entitySnap["c"]) {
            if (component["type"] == "Transform") {
                Transform t = component["v"];
                gCoordinator.addComponent(entity, t);
                auto &transform = gCoordinator.getComponent<Transform>(entity);
                transform = t;
            }
            if (component["type"] == "Color") {
                Color c = component["v"];
                gCoordinator.addComponent(entity, c);
                auto &color = gCoordinator.getComponent<Color>(entity);
                color = c;
            }
            if (component["type"] == "Gravity") {
                Gravity g = component["v"];
                gCoordinator.addComponent(entity, g);
                auto &gravity = gCoordinator.getComponent<Gravity>(entity);
                gravity = g;
            }
            if (component["type"] == "CKinematic") {
                CKinematic k = component["v"];
                gCoordinator.addComponent(entity, k);
                auto &cKinematic = gCoordinator.getComponent<CKinematic>(entity);
                cKinematic = k;
            }
            if (component["type"] == "Camera") {
                Camera c = component["v"];
                gCoordinator.addComponent(entity, c);
                auto &camera = gCoordinator.getComponent<Camera>(entity);
                camera = c;
            }
            if (component["type"] == "Destroy") {
                Destroy d = component["v"];
                gCoordinator.addComponent(entity, d);
                auto &destroy = gCoordinator.getComponent<Destroy>(entity);
                destroy = d;
            }
            if (component["type"] == "Collision") {
                Collision c = component["v"];
                gCoordinator.addComponent(entity, c);
                auto &collision = gCoordinator.getComponent<Collision>(entity);
                collision = c;
            }
            if (component["type"] == "ClientEntity") {
                ClientEntity ce = component["v"];
                gCoordinator.addComponent(entity, ce);
                auto &clientEntity = gCoordinator.getComponent<ClientEntity>(entity);
                clientEntity = ce;
            }
            if (component["type"] == "KeyboardMovement") {
                KeyboardMovement k = component["v"];
                gCoordinator.addComponent(entity, k);
                auto &keyboardMovement = gCoordinator.getComponent<KeyboardMovement>(entity);
                keyboardMovement = k;
            }
            if (component["type"] == "Receiver") {
                Receiver r = component["v"];
                gCoordinator.addComponent(entity, r);
                auto &receiver = gCoordinator.getComponent<Receiver>(entity);
                receiver = r;
            }
            if (component["type"] == "Ball") {
                Ball b = component["v"];
                gCoordinator.addComponent(entity, b);
                auto &ball = gCoordinator.getComponent<Ball>(entity);
                ball = b;
            }
            if (component["type"] == "Brick") {
                Brick b = component["v"];
                gCoordinator.addComponent(entity, b);
                auto &brick = gCoordinator.getComponent<Brick>(entity);
                brick = b;
            }
            if (component["type"] == "Launcher") {
                Launcher l = component["v"];
                gCoordinator.addComponent(entity, l);
                auto &launcher = gCoordinator.getComponent<Launcher>(entity);
                launcher = l;
            }
            if (component["type"] == "PowerUp") {
                PowerUp p = component["v"];
                gCoordinator.addComponent(entity, p);
                auto &powerUp = gCoordinator.getComponent<PowerUp>(entity);
                powerUp = p;
            }
        }
    };

    EventHandler gameStartHandler = [this](const std::shared_ptr<Event> &event) {
        if (event->type == GameEvents::eventTypeToString(GameEvents::GameStart)) {
            gCoordinator.backup(serializer);
        }
    };

    EventHandler gameEndHandler = [this](const std::shared_ptr<Event> &event) {
        if (event->type == GameEvents::eventTypeToString(GameEvents::GameOver)) {
            gCoordinator.restore(deserializer);
            eventCoordinator.clearQueue();
        }
    };

public:
    GameStateHandler() {
        eventCoordinator.subscribe(gameStartHandler, GameEvents::eventTypeToString(GameEvents::GameStart));
        eventCoordinator.subscribe(gameEndHandler, GameEvents::eventTypeToString(GameEvents::GameOver));
    }

    ~GameStateHandler() {
        eventCoordinator.unsubscribe(gameStartHandler, GameEvents::eventTypeToString(GameEvents::GameStart));
        eventCoordinator.unsubscribe(gameEndHandler, GameEvents::eventTypeToString(GameEvents::GameOver));
    }
};
