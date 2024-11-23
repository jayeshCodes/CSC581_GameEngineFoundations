//
// Created by Jayesh Gajbhar on 10/23/24.
//

#ifndef EVENT_HPP
#define EVENT_HPP

#include <nlohmann/json.hpp>
#include "../ECS/types.hpp"
#include "../model/components.hpp"

// all the events in the game engine will inherit from this class
enum EventType {
    None,
    EntityRespawn,
    EntityDeath,
    EntityCollided,
    EntityInput,
    EntityTriggered,
    MainCharCreated,
    PositionChanged,
    DashRight,
    DashLeft,
    ReplayTransformChanged,
    StartRecording,
    StopRecording,
    StartReplaying,
    StopReplaying,
    EntityCreated,
    EntityDestroyed,
    ProjectileCreate,
    GameOver,
    GameRestart,
    GameStarted,
    MonsterHit,
    MonsterDefeated
};

inline std::string eventTypeToString(EventType type) {
    switch (type) {
        case EntityRespawn:
            return "EntityRespawn";
        case EntityDeath:
            return "EntityDeath";
        case EntityCollided:
            return "EntityCollided";
        case EntityInput:
            return "EntityInput";
        case EntityTriggered:
            return "EntityTriggered";
        case MainCharCreated:
            return "MainCharCreated";
        case PositionChanged:
            return "PositionChanged";
        case DashRight:
            return "DashRight";
        case DashLeft:
            return "DashLeft";
        case ReplayTransformChanged:
            return "ReplayTransformChanged";
        case StartRecording:
            return "StartRecording";
        case StopRecording:
            return "StopRecording";
        case StartReplaying:
            return "StartReplaying";
        case StopReplaying:
            return "StopReplaying";
        case EntityCreated:
            return "EntityCreated";
        case EntityDestroyed:
            return "EntityDestroyed";
        case ProjectileCreate:
            return "ProjectileCreate";
        case GameOver:
            return "GameOver";
        case GameRestart:
            return "GameRestart";
        case GameStarted:
            return "GameStarted";
        case MonsterHit:
            return "MonsterHit";
        case MonsterDefeated:
            return "MonsterDefeated";
        default: return "Unknown";
    }
}

struct EntityRespawnData {
    Entity entity;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(EntityRespawnData, entity);

struct EntityDeathData {
    Entity entity;
    Transform respawnPosition;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(EntityDeathData, respawnPosition);

struct EntityCollidedData {
    Entity entityA;
    Entity entityB;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(EntityCollidedData, entityA, entityB);

struct EntityTriggeredData {
    Entity triggerEntity;
    Entity otherEntity;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(EntityTriggeredData, triggerEntity, otherEntity);

struct EntityInputData {
    Entity entity;
    SDL_Keycode key;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(EntityInputData, entity, key)

struct MainCharCreatedData {
    Entity entity;
    std::string message;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MainCharCreatedData, entity, message)

struct PositionChangedData {
    Entity entity;
    std::string message;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PositionChangedData, entity, message)

struct DashData {
    Entity entity;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DashData, entity)

struct ReplayTransformData {
    Entity entity;
    Transform transform;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ReplayTransformData, entity, transform)

struct EntityCreatedData {
    Entity entity;
    std::string id;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(EntityCreatedData, entity, id)

struct EntityDestroyedData {
    Entity entity;
    std::string id;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(EntityDestroyedData, entity, id)

struct ProjectileCreateData {
    Transform shooterTransform;
    float angle;
    float velocity;  // Added velocity fieldr
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ProjectileCreateData, shooterTransform, angle)

struct GameOverData {
    Entity player;
    Entity monster;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(GameOverData, player, monster)

struct MonsterHitData {
    Entity monster;
    Entity projectile;
    float damage;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MonsterHitData, monster, projectile, damage)

#endif //EVENT_HPP
