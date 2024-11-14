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

}
