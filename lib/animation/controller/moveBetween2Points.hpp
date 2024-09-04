//
// Created by Utsav Lal on 9/2/24.
//

#ifndef HORIZONTALMOVEMENTCONTROLLER_HPP
#define HORIZONTALMOVEMENTCONTROLLER_HPP
#include "../../objects/object.hpp"

enum MovementState {
    LEFT,
    RIGHT,
    STOP
};

/**
 * This class exposes a simple animation for a object to go from left to right and wait for few momements in between
 * @param point1: x-coordinate of left corner
 * @param point2: x-coordinate of right corner
 * @param currState: the currState of the object, could be left or right only
 * @param waitTime: integer value denoting the wait time for the object
 */
class MoveBetween2Points {
public:
    MoveBetween2Points(float point1, float point2, MovementState currState, int waitTime);

    void moveBetween2Points(Object &object);

private:
    float left;
    float right;
    MovementState state;
    Uint32 currTime;
    int waitTime;
};

#endif //HORIZONTALMOVEMENTCONTROLLER_HPP