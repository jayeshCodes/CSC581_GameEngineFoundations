//
// Created by Jayesh Gajbhar on 8/28/24.
//

#include "gravity.hpp"

void Gravity::calculate(Object &character) {
    character.acceleration.y += gravityY;
    character.acceleration.x += gravityX;
}


