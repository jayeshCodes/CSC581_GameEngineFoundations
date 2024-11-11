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
    EntityDestroyed
};

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

using EventDataTypes = std::variant<EntityRespawnData, EntityDeathData, EntityCollidedData, EntityInputData,
    EntityTriggeredData, MainCharCreatedData, PositionChangedData, DashData, ReplayTransformData, EntityCreatedData,
    EntityDestroyedData>;

struct Event {
    EventType type;
    EventDataTypes data;
};

inline std::string eventTypeToString(const EventType type) {
    switch (type) {
        case EntityRespawn: return "EntityRespawn";
        case EntityDeath: return "EntityDeath";
        case EntityCollided: return "EntityCollided";
        case EntityInput: return "EntityInput";
        case EntityTriggered: return "EntityTriggered";
        case MainCharCreated: return "EntityCreated";
        case PositionChanged: return "PositionChanged";
        case DashRight: return "DashRight";
        case DashLeft: return "DashLeft";
        case ReplayTransformChanged: return "TransformChanged";
        default: return "Unknown";
    }
}

inline EventType stringToEventType(const std::string &str) {
    if (str == "EntityRespawn") return EntityRespawn;
    if (str == "EntityDeath") return EntityDeath;
    if (str == "EntityCollided") return EntityCollided;
    if (str == "EntityInput") return EntityInput;
    if (str == "EntityTriggered") return EntityTriggered;
    if (str == "EntityCreated") return MainCharCreated;
    if (str == "PositionChanged") return PositionChanged;
    if (str == "DashRight") return DashRight;
    if (str == "DashLeft") return DashLeft;
    if (str == "TransformChanged") return ReplayTransformChanged;
    throw std::invalid_argument("Unknown EventType string");
}

// Define to_json and from_json for Event
inline void to_json(nlohmann::json &j, const Event &event) {
    j["type"] = eventTypeToString(event.type);

    std::visit([&j]<typename T0>(T0 &&arg) {
        using T = std::decay_t<T0>;
        if constexpr (std::is_same_v<T, EntityRespawnData>) {
            j["data"] = arg;
            j["data_type"] = "EntityRespawnData";
        } else if constexpr (std::is_same_v<T, EntityDeathData>) {
            j["data"] = arg;
            j["data_type"] = "EntityDeathData";
        } else if constexpr (std::is_same_v<T, EntityCollidedData>) {
            j["data"] = arg;
            j["data_type"] = "EntityCollidedData";
        } else if constexpr (std::is_same_v<T, EntityInputData>) {
            j["data"] = arg;
            j["data_type"] = "EntityInputData";
        } else if constexpr (std::is_same_v<T, EntityTriggeredData>) {
            j["data"] = arg;
            j["data_type"] = "EntityTriggeredData";
        } else if constexpr (std::is_same_v<T, MainCharCreatedData>) {
            j["data"] = arg;
            j["data_type"] = "EntityCreatedData";
        } else if constexpr (std::is_same_v<T, PositionChangedData>) {
            j["data"] = arg;
            j["data_type"] = "PositionChangedData";
        } else if constexpr (std::is_same_v<T, DashData>) {
            j["data"] = arg;
            j["data_type"] = "DashData";
        } else if constexpr (std::is_same_v<T, ReplayTransformData>) {
            j["data"] = arg;
            j["data_type"] = "TransformData";
        }
    }, event.data);
}

inline void from_json(const nlohmann::json &j, Event &event) {
    event.type = stringToEventType(j.at("type").get<std::string>());

    if (const std::string &dataType = j.at("data_type").get<std::string>(); dataType == "EntityRespawnData") {
        event.data = j.at("data").get<EntityRespawnData>();
    } else if (dataType == "EntityDeathData") {
        event.data = j.at("data").get<EntityDeathData>();
    } else if (dataType == "EntityCollidedData") {
        event.data = j.at("data").get<EntityCollidedData>();
    } else if (dataType == "EntityInputData") {
        event.data = j.at("data").get<EntityInputData>();
    } else if (dataType == "EntityTriggeredData") {
        event.data = j.at("data").get<EntityTriggeredData>();
    } else if (dataType == "EntityCreatedData") {
        event.data = j.at("data").get<MainCharCreatedData>();
    } else if (dataType == "PositionChangedData") {
        event.data = j.at("data").get<PositionChangedData>();
    } else if (dataType == "DashData") {
        event.data = j.at("data").get<DashData>();
    } else if (dataType == "TransformData") {
        event.data = j.at("data").get<ReplayTransformData>();
    }
}


#endif //EVENT_HPP
