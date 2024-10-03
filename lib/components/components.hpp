//
// Created by Utsav Lal on 10/2/24.
//

#ifndef TRANSFORM_HPP
#define TRANSFORM_HPP
#include <SDL.h>

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

#endif //TRANSFORM_HPP
