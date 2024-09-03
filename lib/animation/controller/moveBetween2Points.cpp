//
// Created by Utsav Lal on 9/2/24.
//

#include "moveBetween2Points.hpp"

#include <SDL_timer.h>

MoveBetween2Points::MoveBetween2Points(float point1, float point2, MovementState moveState, int waitTime): left(point1),
    right(point2), state(moveState), waitTime(waitTime) {
    currTime = SDL_GetTicks();
}

void MoveBetween2Points::moveBetween2Points(Object &object) {
    float currentX = object.rect.x;
    switch (state) {
        case LEFT:
            if (currentX <= left) {
                state = STOP;
                currTime = SDL_GetTicks();
                object.velocity.x = 0;
                return;
            }
            object.velocity.x = -200;
            break;
        case RIGHT:
            if (currentX >= right) {
                state = STOP;
                currTime = SDL_GetTicks();
                object.velocity.x = 0;
                return;
            }
            object.velocity.x = 200;
            break;
        case STOP:
            Uint32 passedTime = SDL_GetTicks();
            if ((passedTime - currTime) / 1000.f > waitTime) {
                if (currentX <= left) {
                    state = RIGHT;
                } else {
                    state = LEFT;
                }
            }
            break;
    }
}
