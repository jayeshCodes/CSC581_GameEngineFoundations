//
// Created by Utsav Lal on 11/13/24.
//

#pragma once
#include <string>

namespace GameEvents {
    enum EventType {
        MoveLeft,
        MoveRight,
        EntityInput,
        Launch,
        OutOfBounds,
        Stop,
        PowerUpCollected,
        GameStart,
        GameOver
    };

    inline std::string eventTypeToString(EventType type) {
        switch (type) {
            case MoveLeft:
                return "MoveLeft";
            case MoveRight:
                return "MoveRight";
            case Stop:
                return "Stop";
            case Launch:
                return "Launch";
            case OutOfBounds:
                return "OutOfBounds";
            case PowerUpCollected:
                return "PowerUpCollected";
            case GameStart:
                return "GameStart";
            case GameOver:
                return "GameOver";
            default: return "Unknown";
        }
    }

    struct EntityInputData {
        SDL_Keycode key;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(EntityInputData, key)

    struct OutOfBoundsData {
        Entity entity;
        enum class Direction {
            Left,
            Right,
            Top,
            Bottom
        } direction;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(OutOfBoundsData, entity, direction)

    struct PowerUpCollectedData {
        PowerUpType type;
        bool isFinal = false;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PowerUpCollectedData, type, isFinal)

}
