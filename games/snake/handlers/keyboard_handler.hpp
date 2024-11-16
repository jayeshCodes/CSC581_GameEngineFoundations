//
// Created by Utsav Lal on 11/13/24.
//
#pragma once
#include "../.././../lib/ECS/coordinator.hpp"
#include "../.././../lib/ECS/system.hpp"
#include "../.././../lib/EMS/event_coordinator.hpp"
#include "../model/event.hpp"

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
                    Event leftEvent{eventTypeToString(GameEvents::EventType::Left), {}};
                    eventCoordinator.emit(std::make_shared<Event>(leftEvent));
                    break;
                }
                case SDL_SCANCODE_RIGHT: // Affordance for right arrow key
                case SDL_SCANCODE_D: {
                    Event leftEvent{eventTypeToString(GameEvents::EventType::Right), {}};
                    eventCoordinator.emit(std::make_shared<Event>(leftEvent));
                    break;
                }
                case SDL_SCANCODE_UP:
                case SDL_SCANCODE_W: {
                    Event leftEvent{eventTypeToString(GameEvents::EventType::Top), {}};
                    eventCoordinator.emit(std::make_shared<Event>(leftEvent));
                    break;
                }
                case SDL_SCANCODE_DOWN:
                case SDL_SCANCODE_S: {
                    Event leftEvent{eventTypeToString(GameEvents::EventType::Down), {}};
                    eventCoordinator.emit(std::make_shared<Event>(leftEvent));
                    break;
                }
                default:
                    break;
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
