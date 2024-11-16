//
// Created by Utsav Lal on 11/16/24.
//

#pragma once
#include <nlohmann/json.hpp>

struct Snake {
    int length = 1;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Snake, length)

