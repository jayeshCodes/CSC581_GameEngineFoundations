//
// Created by Utsav Lal on 10/2/24.
//

#ifndef TRANSFORM_HPP
#define TRANSFORM_HPP
#include <SDL.h>
#include <zmq.hpp>
#include <nlohmann/json.hpp>
#include "../enum/enum.hpp"
#include "../core/timeline.hpp"

inline Timeline timeline;

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
    bool movingLeft = false;
    bool movingRight = false;
    bool movingUp = false;
    bool movingDown = false;

    // Double tap detection
    int64_t lastLeftTapTime = timeline.getElapsedTime();
    int64_t lastRightTapTime = timeline.getElapsedTime();
    int64_t lastSpaceTapTime = timeline.getElapsedTime();
    bool wasLeftReleased = true;
    bool wasRightReleased = true;
    bool wasSpaceReleased = true;
    static constexpr float doubleTapThreshold = 0.3f; // seconds
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(KeyboardMovement, speed)

struct Server {
    int listen_port;
    int publish_port;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Server, listen_port, publish_port)


struct Receiver {};



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
};

struct Destroy {
    int slot = -1;
    bool destroy = false;
    bool isSent = false;
};

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

// Jump
struct Jump {
    float maxJumpHeight;
    float jumpDuration;
    bool isJumping;
    float jumpTime;
    bool canJump;
    float initialJumpVelocity;
};

struct Platform {
    float minX;
    float maxX;
    float minY;
    float maxY;
};

struct Respawnable {
    Transform lastSafePosition;
    bool isRespawn = false;
};

/**
 * If mass is -1 then the object is considered to be static
 */
struct RigidBody {
    float mass;
    float drag;
    float angular_drag;
    float gravity_scale;
};

struct TestServer {
    bool start = false;
};

struct TestClient {
    int entities = 1000;
    bool testStarted = false;
    bool testCompleted = false;
    int iterations = 0;
    int64_t startTime = 0;
    int64_t endTime = 0;
};

struct Dash {
    float dashSpeed = 500.0f;
    float dashDuration = 0.2f;
    float dashCooldown = 0.5f;
    bool isDashing = false;
    float dashTimeRemaining = 0.0f;
    float cooldownTimeRemaining = 0.0f;
};

struct Stomp {
    bool isStomping = false;
    float stompSpeed = 500.0f;
    float stompDuration = 0.2f;
    float stompCooldown = 0.5f;
    float stompTimeRemaining = 0.0f;
    float cooldownTimeRemaining = 0.0f;
};

#endif //TRANSFORM_HPP
