//
// Created by Utsav Lal on 10/2/24.
//

#ifndef TRANSFORM_HPP
#define TRANSFORM_HPP
#include <SDL.h>
#include <zmq.hpp>
#include <nlohmann/json.hpp>

#include "../animation/controller/moveBetween2Points.hpp"
#include "../ECS/types.hpp"

struct Transform {
    float x, y;
    float h, w;
    float orientation;
    float scale;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Transform, x, y, h, w, orientation, scale)

struct Color {
    SDL_Color color;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Color, color.r, color.g, color.b, color.a)

struct CKinematic {
    SDL_FPoint velocity;
    float rotation;
    SDL_FPoint acceleration;
    float angular_acceleration;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CKinematic, velocity.x, velocity.y, rotation, acceleration.x, acceleration.y,
                                   angular_acceleration)

struct Camera {
    float x;
    float y;
    float zoom;
    float rotation;
    float viewport_width;
    float viewport_height;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Camera, x, y, zoom, rotation, viewport_width, viewport_height)

struct Gravity {
    float gravX;
    float gravY;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Gravity, gravX, gravY);

struct KeyboardMovement {
    float speed;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(KeyboardMovement, speed)

struct Server {
    int listen_port;
    int publish_port;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Server, listen_port, publish_port)


struct Client {
    int listen_port;
    int publish_port;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Client, listen_port, publish_port)

struct MovingPlatform {
    float p1;
    float p2;
    MovementState state;
    int wait_time;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MovingPlatform, p1, p2, state, wait_time)

struct Network {
    zmq::socket_t *socket;
};
#endif //TRANSFORM_HPP
