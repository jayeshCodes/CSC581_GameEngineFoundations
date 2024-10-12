//
// Created by Utsav Lal on 10/8/24.
//

#ifndef ECS_HELPERS_HPP
#define ECS_HELPERS_HPP
#include <nlohmann/json.hpp>

#include "../ECS/coordinator.hpp"
#include "../model/components.hpp"

namespace ECS {
    inline nlohmann::json constructGameState(Coordinator &gCoordinator) {
        nlohmann::json gameState;

        for (auto entities = gCoordinator.getEntityIds(); const auto &[key, entity]: entities) {
            nlohmann::json entityJson;
            entityJson["entity"] = key;
            nlohmann::json componentsJson;

            if (gCoordinator.hasComponent<Transform>(entity)) {
                componentsJson["Transform"] = gCoordinator.getComponent<Transform>(entity);
            }
            if (gCoordinator.hasComponent<Color>(entity)) {
                componentsJson["Color"] = gCoordinator.getComponent<Color>(entity);
            }
            if (gCoordinator.hasComponent<CKinematic>(entity)) {
                componentsJson["CKinematic"] = gCoordinator.getComponent<CKinematic>(entity);
            }
            if (gCoordinator.hasComponent<Camera>(entity)) {
                componentsJson["Camera"] = gCoordinator.getComponent<Camera>(entity);
            }
            if (gCoordinator.hasComponent<Gravity>(entity)) {
                componentsJson["Gravity"] = gCoordinator.getComponent<Gravity>(entity);
            }
            if (gCoordinator.hasComponent<KeyboardMovement>(entity)) {
                componentsJson["KeyboardMovement"] = gCoordinator.getComponent<KeyboardMovement>(entity);
            }
            if (gCoordinator.hasComponent<MovingPlatform>(entity)) {
                componentsJson["MovingPlatform"] = gCoordinator.getComponent<MovingPlatform>(entity);
            }

            entityJson["components"] = std::move(componentsJson);
            gameState.push_back(std::move(entityJson));
        }
        return gameState;
    }

    inline void parseGameState(Coordinator &gCoordinator, const nlohmann::json &gameState) {
        for (const auto &entityJson: gameState) {
            Entity entity = gCoordinator.createEntity(entityJson["entity"]);
            const auto &componentsJson = entityJson["components"];
            if (componentsJson.contains("Transform")) {
                const auto t = componentsJson["Transform"].get<Transform>();
                gCoordinator.addComponent(entity, componentsJson["Transform"].get<Transform>());
                auto &transform = gCoordinator.getComponent<Transform>(entity);
                transform = t;
            }
            if (componentsJson.contains("Color")) {
                const auto c = componentsJson["Color"].get<Color>();
                gCoordinator.addComponent(entity, componentsJson["Color"].get<Color>());
                auto &color = gCoordinator.getComponent<Color>(entity);
                color = c;
            }
            if (componentsJson.contains("CKinematic")) {
                const auto k = componentsJson["CKinematic"].get<CKinematic>();
                gCoordinator.addComponent(entity, componentsJson["CKinematic"].get<CKinematic>());
                auto &kinematic = gCoordinator.getComponent<CKinematic>(entity);
                kinematic = k;
            }
            if (componentsJson.contains("Camera")) {
                const auto c = componentsJson["Camera"].get<Camera>();
                gCoordinator.addComponent(entity, componentsJson["Camera"].get<Camera>());
                auto &camera = gCoordinator.getComponent<Camera>(entity);
                camera = c;
            }
            if (componentsJson.contains("Gravity")) {
                const auto g = componentsJson["Gravity"].get<Gravity>();
                gCoordinator.addComponent(entity, componentsJson["Gravity"].get<Gravity>());
                auto &gravity = gCoordinator.getComponent<Gravity>(entity);
                gravity = g;
            }
            if (componentsJson.contains("KeyboardMovement")) {
                const auto k = componentsJson["KeyboardMovement"].get<KeyboardMovement>();
                gCoordinator.addComponent(entity, componentsJson["KeyboardMovement"].get<KeyboardMovement>());
                auto &keyboardMovement = gCoordinator.getComponent<KeyboardMovement>(entity);
                keyboardMovement = k;
            }
            if (componentsJson.contains("MovingPlatform")) {
                const auto m = componentsJson["MovingPlatform"].get<MovingPlatform>();
                gCoordinator.addComponent(entity, componentsJson["MovingPlatform"].get<MovingPlatform>());
                auto &movingPlatform = gCoordinator.getComponent<MovingPlatform>(entity);
                movingPlatform = m;
            }
        }
    }
}

#endif //ECS_HELPERS_HPP
