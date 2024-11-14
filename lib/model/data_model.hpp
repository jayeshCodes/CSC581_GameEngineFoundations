//
// Created by Utsav Lal on 11/1/24.
//

#pragma once
#include "components.hpp"
#include "../ECS/types.hpp"

using SERIALIZABLE_COMPONENTS = std::variant<Transform, Color, RigidBody, Collision, CKinematic, MovingPlatform, Destroy
    , Jump
    , Respawnable, Camera, KeyboardMovement, Gravity, VerticalBoost>;

struct NetworkedMessage {};

struct SimpleMessage: NetworkedMessage {
    Message type;
    std::string entity_key;
    std::vector<SERIALIZABLE_COMPONENTS> components;

    SimpleMessage() {
        components.reserve(MAX_COMPONENTS);
    }
};

inline void to_json(nlohmann::json &j,
                    const std::vector<SERIALIZABLE_COMPONENTS> &components) {
    // for all components
    for (const auto &component: components) {
        if (std::holds_alternative<Transform>(component)) {
            j["comp"]["transform"] = std::get<Transform>(component);
        } else if (std::holds_alternative<Color>(component)) {
            j["comp"]["color"] = std::get<Color>(component);
        } else if (std::holds_alternative<RigidBody>(component)) {
            j["comp"]["rb"] = std::get<RigidBody>(component);
        } else if (std::holds_alternative<Collision>(component)) {
            j["comp"]["collision"] = std::get<Collision>(component);
        } else if (std::holds_alternative<CKinematic>(component)) {
            j["comp"]["cKinematic"] = std::get<CKinematic>(component);
        } else if (std::holds_alternative<MovingPlatform>(component)) {
            j["comp"]["movingPlatform"] = std::get<MovingPlatform>(component);
        } else if (std::holds_alternative<Destroy>(component)) {
            j["comp"]["destroy"] = std::get<Destroy>(component);
        } else if (std::holds_alternative<Jump>(component)) {
            j["comp"]["jump"] = std::get<Jump>(component);
        } else if (std::holds_alternative<Respawnable>(component)) {
            j["comp"]["respawnable"] = std::get<Respawnable>(component);
        } else if (std::holds_alternative<Camera>(component)) {
            j["comp"]["camera"] = std::get<Camera>(component);
        } else if (std::holds_alternative<KeyboardMovement>(component)) {
            j["comp"]["keyboardMovement"] = std::get<KeyboardMovement>(component);
        } else if (std::holds_alternative<Gravity>(component)) {
            j["comp"]["gravity"] = std::get<Gravity>(component);
        } else if (std::holds_alternative<VerticalBoost>(component)) {
            j["comp"]["vb"] = std::get<VerticalBoost>(component);
        }
    }
}

inline void from_json(const nlohmann::json &j, std::vector<SERIALIZABLE_COMPONENTS> &components) {
    components.clear(); // Start with an empty vector
    components.reserve(MAX_COMPONENTS);

    // Loop through each component type and add to vector if found
    if (j.contains("comp")) {
        const auto &comp = j.at("comp");

        if (comp.contains("transform")) {
            components.emplace_back(comp.at("transform").get<Transform>());
        }
        if (comp.contains("color")) {
            components.emplace_back(comp.at("color").get<Color>());
        }
        if (comp.contains("rb")) {
            components.emplace_back(comp.at("rb").get<RigidBody>());
        }
        if (comp.contains("collision")) {
            components.emplace_back(comp.at("collision").get<Collision>());
        }
        if (comp.contains("cKinematic")) {
            components.emplace_back(comp.at("cKinematic").get<CKinematic>());
        }
        if (comp.contains("movingPlatform")) {
            components.emplace_back(comp.at("movingPlatform").get<MovingPlatform>());
        }
        if (comp.contains("destroy")) {
            components.emplace_back(comp.at("destroy").get<Destroy>());
        }
        if (comp.contains("jump")) {
            components.emplace_back(comp.at("jump").get<Jump>());
        }
        if (comp.contains("respawnable")) {
            components.emplace_back(comp.at("respawnable").get<Respawnable>());
        }
        if (comp.contains("camera")) {
            components.emplace_back(comp.at("camera").get<Camera>());
        }
        if (comp.contains("keyboardMovement")) {
            components.emplace_back(comp.at("keyboardMovement").get<KeyboardMovement>());
        }
        if (comp.contains("vb")) {
            components.emplace_back(comp.at("vb").get<VerticalBoost>());
        }
    }
}


NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SimpleMessage, type, entity_key, components)
