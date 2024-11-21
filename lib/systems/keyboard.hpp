//
// Created by Jayesh Gajbhar on 10/29/24.
//

#ifndef KEYBOARD_HPP
#define KEYBOARD_HPP

#include "../ECS/coordinator.hpp"
#include "../ECS/system.hpp"
#include "../EMS/event_coordinator.hpp"
#include "../model/components.hpp"
#include "../model/event.hpp"

extern Timeline gameTimeline;
extern Coordinator gCoordinator;
extern EventCoordinator eventCoordinator;

class KeyboardSystem : public System {
private:
    EventHandler keyboardHandler = [this](const std::shared_ptr<Event> &event) {
        std::cout << "Keyboard event received" << std::endl;
        if (event->type == eventTypeToString(EventType::EntityInput)) {
            std::cout<<"Keyboard event received"<<std::endl;
            const EntityInputData &data = event->data;
            auto entity = data.entity;
            if (!gCoordinator.hasComponent<KeyboardMovement>(entity)) { return; }
            auto &keyboard = gCoordinator.getComponent<KeyboardMovement>(entity);

            switch (data.key) {
                // handle keypress events
                case SDL_SCANCODE_P | 0x8000: {
                    auto pauseEvent = std::make_shared<Event>(eventTypeToString(EventType::Pause), nlohmann::json{});
                    eventCoordinator.emit(pauseEvent);
                    break;
                }
                case SDL_SCANCODE_R: {
                    Event resetEvent{eventTypeToString(EventType::Reset), {}};
                    eventCoordinator.emit(std::make_shared<Event>(resetEvent));
                    break;
                }
            }
        }
    };

public:
    KeyboardSystem() {
        eventCoordinator.subscribe(keyboardHandler, eventTypeToString(EventType::EntityInput));
    }

    ~KeyboardSystem() {
        // unsubscribe keyboard events when the system is destroyed
        eventCoordinator.unsubscribe(keyboardHandler, eventTypeToString(EventType::EntityInput));
    }
};

#endif //KEYBOARD_HPP
