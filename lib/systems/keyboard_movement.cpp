//
// Created by Utsav Lal on 10/6/24.
//

#include "../ECS/coordinator.hpp"
#include "../ECS/system.hpp"
#include "../model/components.hpp"
#include "../model/event.hpp"
#include "../EMS/event_coordinator.hpp"
#include <thread>
#include <unordered_set>

extern Coordinator gCoordinator;
extern EventCoordinator eventCoordinator;

struct KeyCombo {
    std::vector<SDL_Scancode> keys;
    int64_t comboWindow;
    EventType eventType;
};

extern Timeline gameTimeline;

class KeyboardMovementSystem : public System {
    std::vector<KeyCombo> keyCombos = {
        {{SDL_SCANCODE_LSHIFT, SDL_SCANCODE_D}, 1000, EventType::DashRight},
        {{SDL_SCANCODE_LSHIFT, SDL_SCANCODE_A}, 1000, EventType::DashLeft},
    };

    std::unordered_map<SDL_Scancode, bool> prevKeyState;
    std::unordered_map<SDL_Scancode, int64_t> keyPressTime;

    const Uint8 *keyboardState = nullptr;

public:
    KeyboardMovementSystem() {
        prevKeyState[SDL_SCANCODE_D] = false;
        prevKeyState[SDL_SCANCODE_A] = false;
        prevKeyState[SDL_SCANCODE_LSHIFT] = false;
        prevKeyState[SDL_SCANCODE_SPACE] = false;
        prevKeyState[SDL_SCANCODE_8] = false;
        prevKeyState[SDL_SCANCODE_9] = false;
        prevKeyState[SDL_SCANCODE_0] = false;
        prevKeyState[SDL_SCANCODE_R] = false;
        prevKeyState[SDL_SCANCODE_P] = false;
    }

    void update() {
        for (auto &entity: entities) {
            keyboardState = SDL_GetKeyboardState(nullptr);
            int64_t now = gameTimeline.getElapsedTime();
            bool comboEventTriggered = false;

            for (const auto &combo: keyCombos) {
                bool comboTriggered = true; // Reset for each combo
                for (auto key: combo.keys) {
                    if (!keyboardState[key] || !keyPressTime.contains(key)) {
                        comboTriggered = false;
                        break;
                    }
                }

                if (comboTriggered) {
                    // Find the oldest press time in this combo
                    auto oldestPressTime = now;
                    for (auto key: combo.keys) {
                        if (const auto pressTime = keyPressTime[key]; pressTime < oldestPressTime) {
                            oldestPressTime = pressTime;
                        }
                    }

                    if (now - oldestPressTime < combo.comboWindow) {
                        Event comboEvent{eventTypeToString(combo.eventType), DashData{entity}};
                        eventCoordinator.emit(std::make_shared<Event>(comboEvent));

                        for (auto key: combo.keys) {
                            keyPressTime.erase(key);
                        }

                        comboEventTriggered = true;
                        break; // Stop checking further combos but continue with individual key checks
                    }
                }
            }

            // Process individual key events even if a combo was triggered
            for (auto &keyState: prevKeyState) {
                auto key = keyState.first;
                bool wasPressed = keyState.second;
                bool isPressed = keyboardState[key];

                if (isPressed && !wasPressed) {
                    // Key just pressed
                    keyPressTime[key] = now;
                    std::cout << "Key pressed: " << key << std::endl;
                    Event individualEvent{eventTypeToString(EventType::EntityInput), EntityInputData{entity, key}};
                    std::cout << "Individual event: " << individualEvent.type << std::endl;
                    std::cout << "Individual event data: " << individualEvent.data << std::endl;
                    eventCoordinator.emit(std::make_shared<Event>(individualEvent));
                } else if (!isPressed && wasPressed) {
                    // Key just released
                    Event releaseEvent{
                        eventTypeToString(EventType::EntityInput), EntityInputData{entity, key | 0x8000}
                    };
                    eventCoordinator.emit(std::make_shared<Event>(releaseEvent));
                }

                prevKeyState[key] = isPressed; // Update previous state
            }
        }
    }
};
