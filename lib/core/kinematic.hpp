//
// Created by Jayesh Gajbhar on 8/28/24.
//

#ifndef KINEMATIC_HPP
#define KINEMATIC_HPP
#include <SDL_rect.h>

#include "../objects/object.hpp"


class Kinematic {
public:
    SDL_Point position;
    float orientation;
    SDL_Point velocity;
    float rotation;
    SDL_Point acceleration;
    float angularAcceleration;

    Kinematic() {
        position = SDL_Point();
        orientation = 0;
        velocity = SDL_Point();
        rotation = 0;
        acceleration = SDL_Point();
        angularAcceleration = 0;
    }

    Kinematic(
        SDL_Point position
        , float orientation, SDL_Point velocity, float rotation, SDL_Point acceleration, float angularAcceleration){
        this->position = position;
        this->orientation = orientation;
        this->velocity = velocity;
        this->rotation = rotation;
    }
};

#endif //KINEMATIC_HPP
