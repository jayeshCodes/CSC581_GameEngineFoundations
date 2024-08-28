//
// Created by Utsav Lal on 8/28/24.
//

#ifndef OBJECT_HPP
#define OBJECT_HPP
#include <SDL_rect.h>

#include "../core/kinematic.hpp"

class Object {
public:
    Object(SDL_FRect rect, SDL_Color color);

    virtual void update(float dt);

    virtual ~Object();
    
    SDL_FRect rect;
    float orientation;
    SDL_Color color;

    SDL_FPoint velocity;
    float rotation;

    SDL_FPoint acceleration;
    float angular_acceleration;
};

#endif //OBJECT_HPP
