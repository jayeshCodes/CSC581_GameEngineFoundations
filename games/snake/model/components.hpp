//
// Created by Utsav Lal on 11/16/24.
//

#pragma once
#include <nlohmann/json.hpp>

struct Snake {
    int length = 1;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Snake, length)

struct Map {
    int length;
    int rows;
    int cols;
    int screen_width;
    int screen_height;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Map, rows, cols)


struct Food {
    int length = 1;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Food, length)
