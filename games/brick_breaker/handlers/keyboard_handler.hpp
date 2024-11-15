//
// Created by Utsav Lal on 11/13/24.
//
#pragma once
#include "../../../lib/model/event.hpp"
#include "../.././../lib/ECS/coordinator.hpp"
#include "../.././../lib/ECS/system.hpp"
#include "../.././../lib/EMS/event_coordinator.hpp"
#include "../.././../lib/model/components.hpp"
#include "../model/events.hpp"

extern Timeline gameTimeline;
extern Coordinator gCoordinator;
extern EventCoordinator eventCoordinator;

class KeyboardHandler : public System {
private:
    EventHandler keyboardHandler = [this](const std::shared_ptr<Event> &event) {
        if (event->type == eventTypeToString(GameEvents::EntityInput)) {
            const GameEvents::EntityInputData &data = event->data;

            switch (data.key) {
                // handle keypress events
                case SDL_SCANCODE_LEFT: // Affordance for left arrow key
                case SDL_SCANCODE_A: {
                    Event leftEvent{eventTypeToString(GameEvents::EventType::MoveLeft), {}};
                    eventCoordinator.emit(std::make_shared<Event>(leftEvent));
                    break;
                }
                case SDL_SCANCODE_RIGHT: // Affordance for right arrow key
                case SDL_SCANCODE_D: {
                    Event leftEvent{eventTypeToString(GameEvents::EventType::MoveRight), {}};
                    eventCoordinator.emit(std::make_shared<Event>(leftEvent));
                    break;
                }
                case SDL_SCANCODE_SPACE: {
                }
                // handle key release events
                case SDL_SCANCODE_LEFT | 0x8000: // Affordance for left arrow key
                case SDL_SCANCODE_RIGHT | 0x8000: // Affordance for right arrow key
                case SDL_SCANCODE_D | 0x8000:
                case SDL_SCANCODE_A | 0x8000: {
                    Event stopEvent{eventTypeToString(GameEvents::EventType::Stop), {}};
                    eventCoordinator.emit(std::make_shared<Event>(stopEvent));
                    break;
                }
                case SDL_SCANCODE_SPACE | 0x8000: {
                    Event launchEvent{eventTypeToString(GameEvents::EventType::Launch), {}};
                    eventCoordinator.emit(std::make_shared<Event>(launchEvent));
                    break;
                }
            }
        }
    };

public:
    KeyboardHandler() {
        eventCoordinator.subscribe(keyboardHandler, eventTypeToString(GameEvents::EntityInput));
    }

    ~KeyboardHandler() {
        // unsubscribe keyboard events when the system is destroyed
        eventCoordinator.unsubscribe(keyboardHandler, eventTypeToString(GameEvents::EntityInput));
    }
};
