//
// Created by Utsav Lal on 11/13/24.
//

#pragma once
#include <SDL_keycode.h>
#include <string>
#include <nlohmann/json.hpp>

namespace GameEvents {
    enum EventType {
        EntityInput,
        Left,
        Right,
        Top,
        Down,
        FoodEaten,
        GameStart,
        GameEnd
    };

    inline std::string eventTypeToString(EventType type) {
        switch (type) {
            case EntityInput:
                return "EntityInput";
            case Left:
                return "Left";
            case Right:
                return "Right";
            case Top:
                return "Top";
            case Down:
                return "Down";
            case FoodEaten:
                return "FoodEaten";
            case GameStart:
                return "GameStart";
            case GameEnd:
                return "GameEnd";
            default: return "Unknown";
        }
    }

    struct EntityInputData {
        SDL_Keycode key;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(EntityInputData, key)

    struct GameStartData {
        int length;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(GameStartData, length)

}
