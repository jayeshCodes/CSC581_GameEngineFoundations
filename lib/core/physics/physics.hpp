//
// Created by Jayesh Gajbhar on 8/28/24.
//

#ifndef PHYSICS_HPP
#define PHYSICS_HPP

#include <SDL.h>

#include "../../objects/object.hpp"


class Physics {
public:

    Physics();

    virtual void calculate(Object &character) {}

    virtual void calculate(Object &character, SDL_FPoint direction) {}

    virtual ~Physics() = default;
};


#endif //PHYSICS_HPP
