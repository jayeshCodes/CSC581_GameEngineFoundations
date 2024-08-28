//
// Created by Utsav Lal on 8/28/24.
//

#include "object.hpp"

Object::Object(SDL_Rect rect, SDL_Color color) {
    this->rect = rect;
    this->color = color;
}

Object::~Object() {
}
