//
// Created by Utsav Lal on 8/28/24.
//

#ifndef RECTANGLE_HPP
#define RECTANGLE_HPP
#include <SDL_pixels.h>
#include <SDL_rect.h>

#include "../object.hpp"
#include "../../core/structs.hpp"

extern App *app;

class Rectangle : public Object{
public:
    Rectangle(const SDL_Color color, const SDL_FRect rect, bool rigid, float mass, float restitution): Object(rect, color, rigid, mass, restitution) {
    }

    void draw() const;

    void update(float dt) override;
};

#endif //RECTANGLE_HPP
