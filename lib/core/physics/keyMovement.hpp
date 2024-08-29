//
// Created by Utsav Lal on 8/29/24.
//

#ifndef KEYMOVEMENT_HPP
#define KEYMOVEMENT_HPP
#include "physics.hpp"

class KeyMovement : public Physics {
public:
    KeyMovement(float xVel, float yVel);

    void calculate(Object &character, SDL_FPoint direction) override;

    ~KeyMovement() override = default;

private:
    float xVelocity;
    float yVelocity;
};

#endif //KEYMOVEMENT_HPP
