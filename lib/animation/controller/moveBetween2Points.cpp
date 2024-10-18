//
// Created by Utsav Lal on 9/2/24.
//

#include "moveBetween2Points.hpp"

#include <SDL_timer.h>

MoveBetween2Points::MoveBetween2Points(float point1, float point2, MovementState moveState, int waitTime, Timeline &timeline): left(point1),
    right(point2), state(moveState), waitTime(waitTime), timeline(timeline) {
    currTime = timeline.getElapsedTime();
}

void MoveBetween2Points::moveBetween2Points(Object &object) {
    float currentX = object.rect.x;
    switch (state) {
        case TO:
            if (currentX <= left) {
                state = STOP;
                currTime = timeline.getElapsedTime();
                object.velocity.x = 0;
                return;
            }
        object.velocity.x = -200;
        break;
        case FRO:
            if (currentX >= right) {
                state = STOP;
                currTime = timeline.getElapsedTime();
                object.velocity.x = 0;
                return;
            }
        object.velocity.x = 200;
        break;
        case STOP:
            Uint32 passedTime = timeline.getElapsedTime();
        if ((passedTime - currTime) / 1000.f > waitTime) {
            if (currentX <= left) {
                state = FRO;
            } else {
                state = TO;
            }
        }
        break;
    }
}