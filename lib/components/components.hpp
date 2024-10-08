//
// Created by Utsav Lal on 10/2/24.
//

#ifndef TRANSFORM_HPP
#define TRANSFORM_HPP
#include <SDL.h>

#include "../animation/controller/moveBetween2Points.hpp"
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
};

struct Gravity {};

struct KeyboardMovement {
    float speed;
};

struct Server {
    int listen_port;
    int publish_port;
};

struct Client {
    int listen_port;
    int publish_port;
};

struct MovingPlatform {
    float p1;
    float p2;
    MovementState state;
    int wait_time;
};

#endif //TRANSFORM_HPP
