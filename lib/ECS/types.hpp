//
// Created by Utsav Lal on 10/2/24.
//

#ifndef TYPES_HPP
#define TYPES_HPP

#include <array>
#include <bitset>
#include <cstdint>
#include <vector>

// Forward declare ALL_COMPONENTS to avoid circular dependency
namespace Components {
    struct Transform;
    struct Color;
    struct CKinematic;
    struct Camera;
    struct Gravity;
    struct KeyboardMovement;
    struct Server;
    struct Receiver;
    struct MovingPlatform;
    struct ServerEntity;
    struct ClientEntity;
    struct Destroy;
    struct Collision;
    struct Jump;
    struct Respawnable;
    struct RigidBody;
    struct Dash;
    struct Stomp;
    struct Bubble;
    struct BubbleShooter;
    struct BubbleProjectile;
    struct BubbleGridManager;
    struct Score;
    struct GridMovement;
    struct GridGenerator;
}

// Giving an alias to the data type and defining the maximum number of entities
using Entity = std::uint32_t;
constexpr Entity MAX_ENTITIES = 5000;
constexpr Entity INVALID_ENTITY = MAX_ENTITIES + 100;

// Giving an alias to the data type and defining the maximum number of components
using ComponentType = std::uint8_t;
constexpr ComponentType MAX_COMPONENTS = 32;

// Signature helps identify the components that an entity has
// For example if an entity has components 0, 1, 2 then Signature will be 0000000000000000000000000000111
using Signature = std::bitset<MAX_COMPONENTS>;

constexpr Entity EntityNotCreated = -1;


struct EntitySnapshot;
using Snapshot = std::vector<EntitySnapshot>;

#include "../model/components.hpp"

struct EntitySnapshot {
    Entity entity;
    std::string id;
    std::vector<ALL_COMPONENTS> components;
};


#endif //TYPES_HPP
