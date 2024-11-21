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
    GameOver,
    BubbleCreated,
    BubbleShot,
    BubbleCollision,
    GridDropped,
    GridDropCompleted,
    CheckFloatingBubbles,
    ResetShooter,
    DisableShooter,
    BubbleMatch,
    ScoreUpdated,
    BubblePopped,
    Reset,
    Pause
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
        case GameOver:
            return "GameOver";
        case DisableShooter:
            return "DisableShooter";
        case ResetShooter:
            return "ResetShooter";
        case BubbleCreated:
            return "BubbleCreated";
        case GridDropped:
            return "GridDropped";
        case GridDropCompleted:
            return "GridDropCompleted";
        case BubbleShot:
            return "BubbleShot";
        case CheckFloatingBubbles:
            return "CheckFloatingBubbles";
        case BubbleCollision:
            return "BubbleCollision";
        case BubbleMatch:
            return "BubbleMatch";
        case ScoreUpdated:
            return "ScoreUpdated";
        case BubblePopped:
            return "BubblePopped";
        case Reset:
            return "Reset";
        case Pause:
            return "Pause";
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

struct GameOverData {
    std::string message;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(GameOverData, message)

struct BubbleCreatedData {
    Entity entity;
    int row;
    int col;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(BubbleCreatedData, entity, row, col)

struct DisableShooterData {
    int64_t timestamp;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DisableShooterData, timestamp)

struct ResetShooterData {
    int64_t timestamp;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ResetShooterData, timestamp)

struct BubbleMatchData {
    std::vector<Entity> matches;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(BubbleMatchData, matches)

struct ScoreUpdateData {
    int value;
    int multiplier;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ScoreUpdateData, value, multiplier)


#endif //EVENT_HPP
