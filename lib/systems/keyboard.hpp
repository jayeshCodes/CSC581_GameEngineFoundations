//
// Created by Jayesh Gajbhar on 10/29/24.
//

#ifndef KEYBOARD_HPP
#define KEYBOARD_HPP

#include "../ECS/coordinator.hpp"
#include "../ECS/system.hpp"
#include "../EMS/event_coordinator.hpp"
#include "../model/components.hpp"

extern Coordinator gCoordinator;
extern EventCoordinator eventCoordinator;

class KeyboardSystem : public System {
private:
    EventHandler keyboardHandler = [this](const std::shared_ptr<Event> &event) {
        if (event->type == EventType::EntityInput) {
            const auto &data = std::get<EntityInputData>(event->data);
            auto entity = data.entity;
            auto &kinematic = gCoordinator.getComponent<CKinematic>(entity);
            auto &keyboard = gCoordinator.getComponent<KeyboardMovement>(entity);
            auto &jump = gCoordinator.getComponent<Jump>(entity);

            switch (data.key) {
                // handle keypress events
                case SDLK_LEFT: // Affordance for left arrow key
                case SDLK_a:
                    kinematic.velocity.x = -keyboard.speed;
                    keyboard.movingLeft = true;
                    break;
                case SDLK_RIGHT: // Affordance for right arrow key
                case SDLK_d:
                    kinematic.velocity.x = keyboard.speed;
                    keyboard.movingRight = true;
                    break;
                case SDLK_SPACE:
                    if (jump.canJump && !jump.isJumping) {
                        kinematic.velocity.y = -jump.initialJumpVelocity;
                        jump.isJumping = false;
                        jump.canJump = false;
                        jump.jumpTime = 0.f;
                    }
                    break;

                // handle key release events
                case SDLK_LEFT | 0x8000: // Affordance for left arrow key
                case SDLK_a | 0x8000:
                    keyboard.movingLeft = false;
                    if (!keyboard.movingRight) {
                        kinematic.velocity.x = 0;
                    }
                    break;
                case SDLK_RIGHT | 0x8000: // Affordance for right arrow key
                case SDLK_d | 0x8000:
                    keyboard.movingRight = false;
                    if (!keyboard.movingLeft) {
                        kinematic.velocity.x = 0;
                    }
                    break;
            }
        }
    };

public:
    KeyboardSystem() {
        eventCoordinator.subscribe(keyboardHandler, EventType::EntityInput);
    }

    ~KeyboardSystem() {
        // unsubscribe keyboard events when the system is destroyed
        eventCoordinator.unsubscribe(keyboardHandler, EventType::EntityDeath);
    }

    void reset(Entity entity) {
        auto &keyboard = gCoordinator.getComponent<KeyboardMovement>(entity);
        auto &kinematic = gCoordinator.getComponent<CKinematic>(entity);
        keyboard.movingLeft = false;
        keyboard.movingRight = false;
        kinematic.velocity.x = 0;
    }


    void update() const {
        // Process all keyboard events
        for (const auto &event: eventCoordinator.getEvents(EventType::EntityInput)) {
            keyboardHandler(event);
        }
    }
};

#endif //KEYBOARD_HPP
