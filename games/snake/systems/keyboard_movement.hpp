//
// Created by Utsav Lal on 11/16/24.
//

#pragma once

#include "../../../lib/ECS/coordinator.hpp"
#include "../../../lib/ECS/system.hpp"
#include "../../../lib/EMS/event_coordinator.hpp"


extern EventCoordinator eventCoordinator;
extern Coordinator gCoordinator;
extern Timeline gameTimeline;

class KeyboardMovementSystem : public System {
    std::unordered_map<SDL_Scancode, bool> prevKeyState;

    const Uint8 *keyboardState = nullptr;

public:
    KeyboardMovementSystem() {
        prevKeyState[SDL_SCANCODE_D] = false;
        prevKeyState[SDL_SCANCODE_A] = false;
        prevKeyState[SDL_SCANCODE_W] = false;
        prevKeyState[SDL_SCANCODE_S] = false;
    }

    void update() {
        keyboardState = SDL_GetKeyboardState(nullptr);
        for (auto &keyState: prevKeyState) {
            auto key = keyState.first;
            bool wasPressed = keyState.second;
            bool isPressed = keyboardState[key];

            if (isPressed && !wasPressed) {
                // Key just pressed
                Event individualEvent{GameEvents::eventTypeToString(GameEvents::EntityInput), GameEvents::EntityInputData{key}};
                eventCoordinator.emit(std::make_shared<Event>(individualEvent));
            } else if (isPressed) {
                // Key is being held down
                Event holdEvent{eventTypeToString(GameEvents::EntityInput), GameEvents::EntityInputData{key}};
                eventCoordinator.emit(std::make_shared<Event>(holdEvent));
            } else if (wasPressed) {
                // Key just released
                Event releaseEvent{
                    eventTypeToString(GameEvents::EntityInput), GameEvents::EntityInputData{key | 0x8000}
                };
                eventCoordinator.emit(std::make_shared<Event>(releaseEvent));
            }

            prevKeyState[key] = isPressed; // Update previous state
        }
    }
};
