//
// Created by Utsav Lal on 8/28/24.
//

#include "rectangle.hpp"

#include "../../core/defs.hpp"

void Rectangle::draw() const {
    // Set the color of the rectangle
    SDL_SetRenderDrawColor(app->renderer, color.r, color.g, color.b, color.a);

    // Set the outline
    SDL_RenderDrawRectF(app->renderer, &rect);

    // Fill the rectangle
    SDL_RenderFillRectF(app->renderer, &rect);
}

void Rectangle::update(float dt) {
    // if (rect.y == SCREEN_HEIGHT) {
    //     // rect.y =
    //     return;
    // }
    // rect.y = rect.y + 1;

    Object::update(dt);
}
