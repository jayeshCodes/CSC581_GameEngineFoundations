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
            default: return "Unknown";
        }
    }

    struct EntityInputData {
        SDL_Keycode key;
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(EntityInputData, key)
}
