//
// Created by Jayesh Gajbhar on 8/28/24.
//

#ifndef PHYSICS_HPP
#define PHYSICS_HPP

#include <SDL.h>
#include <memory>
#include <vector>

#include "../../objects/object.hpp"
#include "../../objects/shapes/rectangle.hpp"


class Physics {
public:

    Physics();

    virtual void calculate(Object &character) {}

    virtual void calculate(Object &character, SDL_FPoint direction) {}

    virtual void calculate(Object &character, std::vector<std::unique_ptr<Rectangle>> &objects) {}

    virtual ~Physics() = default;
};


#endif //PHYSICS_HPP
