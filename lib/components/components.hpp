//
// Created by Utsav Lal on 10/2/24.
//

#ifndef TRANSFORM_HPP
#define TRANSFORM_HPP
#include <SDL.h>
#include "../core/defs.hpp"

struct Transform {
    float x, y;
    float h, w;
    float orientation;
    float scale;
};

struct Color {
    SDL_Color color;
};

struct CKinematic {
    SDL_FPoint velocity;
    float rotation;
    SDL_FPoint acceleration;
    float angular_acceleration;
};

struct Camera {
    float x;
    float y;
    float zoom;
    float rotation;
    float viewport_width;
    float viewport_height;

    // constructor
    // explicit Camera(
    // float x = 0.f,
    // float y = 0.f,
    // float zoom = 1.f,
    // float rotation = 0.f,
    // float viewport_width = SCREEN_WIDTH,
    // float viewport_height = SCREEN_HEIGHT
    // ) : x(x), y(y), zoom(zoom), rotation(rotation), viewport_width(viewport_width), viewport_height(viewport_height) {}
};

#endif //TRANSFORM_HPP
