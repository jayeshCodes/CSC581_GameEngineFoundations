//
// Created by Utsav Lal on 9/2/24.
//

#include "moveBetween2Points.hpp"

#include <SDL_timer.h>

MoveBetween2Points::MoveBetween2Points(float point1, float point2, bool movingLeft): left(point1),
    right(point2), movingLeft(movingLeft) {
}

void MoveBetween2Points::moveBetween2Points(Object &object) {
    float currentX = object.rect.x;
    if (movingLeft) {
        if (currentX <= left) {
            movingLeft = false;
            return;
        }
        object.velocity.x = -100;
    } else {
        if (currentX >= right) {
            movingLeft = true;
            return;
        }
        object.velocity.x = 100;
    }
};
