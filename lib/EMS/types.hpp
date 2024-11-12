//
// Created by Utsav Lal on 11/12/24.
//

#pragma once
#include <string>
#include <nlohmann/json.hpp>

struct Event {
    std::string type;
    nlohmann::json data;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Event, type, data);
