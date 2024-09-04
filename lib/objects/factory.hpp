//
// Created by Utsav Lal on 8/28/24.

#ifndef FACTORY_HPP
#define FACTORY_HPP
#include "shapes/rectangle.hpp"
#include <memory>

/**
 * This is a factory to create shapes. Any and all types of objects will be created using this factory
 */
class Factory {
public:
    static std::unique_ptr<Rectangle> createRectangle(SDL_Color color, SDL_FRect rect, bool rigid, float mass, float restitution);
};

#endif //FACTORY_HPP
