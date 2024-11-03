//
// Created by Utsav Lal on 10/2/24.
//

#ifndef TRANSFORM_HPP
#define TRANSFORM_HPP
#include <SDL.h>
#include <zmq.hpp>
#include <nlohmann/json.hpp>
#include "../enum/enum.hpp"

struct Transform {
    float x, y;
    float h, w;
    float orientation;
    float scale;

public:
    [[nodiscard]] bool equal(const Transform &other) const {
        return x == other.x && y == other.y && h == other.h && w == other.w && orientation == other.orientation &&
               scale == other.scale;
    }
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Transform, x, y, h, w, orientation, scale)

struct Color {
    SDL_Color color;
};

inline void to_json(nlohmann::json &j, const SDL_Color &c) {
    j = nlohmann::json{{"r", c.r}, {"g", c.g}, {"b", c.b}, {"a", c.a}};
}

inline void from_json(const nlohmann::json &j, SDL_Color &c) {
    j.at("r").get_to(c.r);
    j.at("g").get_to(c.g);
    j.at("b").get_to(c.b);
    j.at("a").get_to(c.a);
}

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Color, color)

struct CKinematic {
    SDL_FPoint velocity;
    float rotation;
    SDL_FPoint acceleration;
    float angular_acceleration;
};

inline void to_json(nlohmann::json &j, const SDL_FPoint &p) {
    j = nlohmann::json{{"x", p.x}, {"y", p.y}};
}

inline void from_json(const nlohmann::json &j, SDL_FPoint &p) {
    j.at("x").get_to(p.x);
    j.at("y").get_to(p.y);
}

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CKinematic, velocity, rotation, acceleration, angular_acceleration)

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
    bool movingLeft = false;
    bool movingRight = false;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(KeyboardMovement, speed)

struct Server {
    int listen_port;
    int publish_port;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Server, listen_port, publish_port)


struct Receiver {
};


struct MovingPlatform {
    float p1;
    float p2;
    MovementState state;
    int wait_time;
    MovementType movementType;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MovingPlatform, p1, p2, state, wait_time)

struct ServerEntity {
};


struct ClientEntity {
    int noOfTimes = 0;
    bool synced = false;
};

struct Destroy {
    int slot = -1;
    bool destroy = false;
    bool isSent = false;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Destroy, slot, destroy, isSent)

/**
 * The CollisionShape is an enumeration that defines the shape of the collision boundary for an entity.
 * It's used to determine how collision detection should be performed for that entity.
 */
enum class CollisionShape {
    RECTANGLE,
    CIRCLE
};

enum CollisionLayer {
    OTHER = 0,
    PLAYER,
    MOVING_PLATFORM,
};

struct Collision {
    bool isCollider;
    bool isTrigger;
    CollisionLayer layer;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Collision, isCollider, isTrigger, layer)

// Jump
struct Jump {
    float maxJumpHeight;
    float jumpDuration;
    bool isJumping;
    float jumpTime;
    bool canJump;
    float initialJumpVelocity;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Jump, maxJumpHeight, jumpDuration, isJumping, jumpTime, canJump, initialJumpVelocity)

struct Respawnable {
    Transform lastSafePosition;
    bool isRespawn = false;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Respawnable, lastSafePosition, isRespawn)

/**
 * If mass is -1 then the object is considered to be static
 */
struct RigidBody {
    float mass;
    float drag;
    float angular_drag;
    float gravity_scale;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(RigidBody, mass, drag, angular_drag, gravity_scale)

#endif //TRANSFORM_HPP
