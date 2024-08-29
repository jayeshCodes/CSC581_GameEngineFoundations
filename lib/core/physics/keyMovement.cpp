//
// Created by Utsav Lal on 8/29/24.
//

#include "keyMovement.hpp"

KeyMovement::KeyMovement(float xVel, float yVel) {
    xVelocity = xVel;
    yVelocity = yVel;
}

void KeyMovement::calculate(Object &character, SDL_FPoint direction) {
    if(direction.x == 0 && direction.y == 0) {
        character.velocity.x = 0;
        character.velocity.y = 0;
    }
    character.velocity.x += (direction.x * xVelocity);
    character.velocity.y += (direction.y * yVelocity);
}

