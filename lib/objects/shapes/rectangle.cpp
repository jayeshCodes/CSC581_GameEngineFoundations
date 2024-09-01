//
// Created by Utsav Lal on 8/28/24.
//

#include "rectangle.hpp"

#include <iostream>
#include <ostream>

void Rectangle::draw() const {
    // Set the color of the rectangle
    SDL_SetRenderDrawColor(app->renderer, color.r, color.g, color.b, color.a);


    SDL_FRect tRect = rect;

    // Calculate scale
    Object::scale(tRect);

    // Set the outline
    SDL_RenderDrawRectF(app->renderer, &tRect);

    // Fill the rectangle with the scaled dimensions
    SDL_RenderFillRectF(app->renderer, &tRect);
}

void Rectangle::update(float dt) {
    Object::update(dt);
}
