//
// Created by Utsav Lal on 8/28/24.
//

#include "rectangle.hpp"

#include "../../core/defs.hpp"

void Rectangle::draw() const {
    // Set the color of the rectangle
    SDL_SetRenderDrawColor(app->renderer, color.r, color.g, color.b, color.a);

    // Set the outline
    SDL_RenderDrawRect(app->renderer, &rect);

    // Fill the rectangle
    SDL_RenderFillRect(app->renderer, &rect);
}

void Rectangle::update() {
    if (rect.y == SCREEN_HEIGHT) {
        rect.y = SCREEN_HEIGHT - 10;
        return;
    }
    rect.y = rect.y + 1;
}
