//
// Created by Utsav Lal on 8/28/24.
//

#include "factory.hpp"


std::unique_ptr<Rectangle> Factory::createRectangle(const SDL_Color color, const SDL_FRect rect) {
    auto rectangle = std::make_unique<Rectangle>(color, rect);
    return rectangle;
}
