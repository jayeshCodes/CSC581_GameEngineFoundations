//
// Created by Utsav Lal on 10/2/24.
//

#pragma once
#include <SDL.h>
#include <zmq.hpp>
#include <nlohmann/json.hpp>
#include "../enum/enum.hpp"
#include "../core/timeline.hpp"

using Entity = std::uint32_t;
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
    bool movingUp = false;
    bool movingDown = false;

    // Double tap detection
    int64_t lastLeftTapTime = timeline.getElapsedTime();
    int64_t lastRightTapTime = timeline.getElapsedTime();
    int64_t lastSpaceTapTime = timeline.getElapsedTime();
    bool wasLeftReleased = false;
    bool wasRightReleased = false;
    bool wasSpaceReleased = false;
    static constexpr float doubleTapThreshold = 0.3f; // seconds
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
    BUBBLE
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
    bool isDead = false;
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

struct Dash {
    float dashSpeed = 500.0f;
    float dashDuration = 0.2f;
    float dashCooldown = 0.5f;
    bool isDashing = false;
    float dashTimeRemaining = 0.0f;
    float cooldownTimeRemaining = 0.0f;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Dash, dashSpeed, dashDuration, dashCooldown, isDashing, dashTimeRemaining,
                                   cooldownTimeRemaining)

struct Stomp {
    bool isStomping = false;
    float stompSpeed = 500.0f;
    float stompDuration = 0.2f;
    float stompCooldown = 0.5f;
    float stompTimeRemaining = 0.0f;
    float cooldownTimeRemaining = 0.0f;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Stomp, isStomping, stompSpeed, stompDuration, stompCooldown, stompTimeRemaining,
                                   cooldownTimeRemaining)

struct VerticalBoost {
    float velocity = -100.f;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(VerticalBoost, velocity)

// Represents a bubble in the grid
struct Bubble {
    bool isActive = true;
    int row = 0;
    int col = 0;
    float radius = 16.f; // Radius of the bubble
    int colorIndex = 0; // Index of the color in the color palette
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Bubble, isActive, row, col, radius, colorIndex)

// Component for the bubble shooter/canon
struct BubbleShooter {
    float rotationSpeed = 2.f; // Speed at which the canon rotates (degrees per frame)
    float minAngle = -85.f; // Minimum angle the canon can rotate to
    float maxAngle = 85.f; // Maximum angle the canon can rotate to
    float currentAngle = 0.f; // Current angle of the canon
    float shootForce = 400.f; // Speed at which the bubble is shot
    bool canShoot = true; // Whether the canon can shoot a bubble
    float reloadTime = 0.5f; // Time it takes to reload the canon
    float currentReloadTime = 0.f; // Time since the canon last shot a bubble
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(BubbleShooter, rotationSpeed, minAngle, maxAngle, currentAngle, shootForce, canShoot,
                                   reloadTime, currentReloadTime)

// Component for the moving bubble projectile
struct BubbleProjectile {
    SDL_FPoint velocity = {0.f, 0.f}; // Velocity of the bubble
    bool isMoving = false; // Whether the bubble is moving
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(BubbleProjectile, velocity, isMoving)

// Component for the bubble grid manager (co-authored by github copilot)
struct BubbleGridManager {
    static constexpr int GRID_ROWS = 12; // Number of rows in the grid
    static constexpr int GRID_COLS = 8; // Number of columns in the grid
    static constexpr float BUBBLE_SPACING = 32.f; // Spacing between bubbles
    static constexpr float START_X = 100.f; // X-coordinate of the top-left corner of the grid
    static constexpr float START_Y = 50.f; // Y-coordinate of the top-left corner of the grid
    std::vector<Entity> grid; // List of entities representing the bubbles in the grid
    int currentLevel = 1;
    int score = 0;
};

using ALL_COMPONENTS = std::variant<Transform, Color, CKinematic, Camera, Gravity, KeyboardMovement, Server, Receiver,
    MovingPlatform, ServerEntity, ClientEntity, Destroy, Collision, Jump, Respawnable, RigidBody, Dash, Stomp,
    VerticalBoost, Bubble, BubbleProjectile, BubbleShooter, BubbleGridManager>;