//
// Created by Utsav Lal on 11/13/24.
//

#pragma once
#include <string>
#include <nlohmann/json.hpp>

#include "../../../lib/ECS/types.hpp"
#include "../../../lib/enum/enum.hpp"

struct BBMessage {
    Message type;
    std::string entity_key;
    std::vector<nlohmann::json> components;

    BBMessage() {
        components.reserve(MAX_COMPONENTS);
    }
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(BBMessage, type, entity_key, components)
