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

    virtual void calculate(Object &character) = 0;

    virtual ~Physics() = default;
};


#endif //PHYSICS_HPP
