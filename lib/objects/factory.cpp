//
// Created by Utsav Lal on 8/28/24.
//

#include "factory.hpp"


std::unique_ptr<Rectangle> Factory::createRectangle(const SDL_Color color, const SDL_FRect rect, bool rigid, float mass, float restitution) {
    auto rectangle = std::make_unique<Rectangle>(color, rect, rigid, mass, restitution);
    return rectangle;
}
