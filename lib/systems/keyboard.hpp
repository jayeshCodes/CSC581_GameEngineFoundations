//
// Created by Jayesh Gajbhar on 10/29/24.
//

#ifndef KEYBOARD_HPP
#define KEYBOARD_HPP

#include "../ECS/coordinator.hpp"
#include "../ECS/system.hpp"
#include "../EMS/event_coordinator.hpp"
#include "../model/components.hpp"

extern Timeline gameTimeline;
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
            auto &dash = gCoordinator.getComponent<Dash>(entity);
            auto &stomp = gCoordinator.getComponent<Stomp>(entity);

            switch (data.key) {
                // handle keypress events
                case SDL_SCANCODE_LEFT: // Affordance for left arrow key
                case SDL_SCANCODE_A: {
                    kinematic.velocity.x = -keyboard.speed;
                    keyboard.movingLeft = true;
                    break;
                }
                case SDL_SCANCODE_RIGHT: // Affordance for right arrow key
                case SDL_SCANCODE_D: {
                    kinematic.velocity.x = keyboard.speed;
                    keyboard.movingRight = true;
                    break;
                case SDL_SCANCODE_SPACE:
                    if (jump.canJump && !jump.isJumping) {
                        kinematic.velocity.y = -jump.initialJumpVelocity;
                        jump.isJumping = true;
                        jump.canJump = false;
                        jump.jumpTime = 0.f;
                    }
                    break;
                }
                // handle key release events
                case SDL_SCANCODE_LEFT | 0x8000: // Affordance for left arrow key
                case SDL_SCANCODE_A | 0x8000:
                    keyboard.movingLeft = false;
                    keyboard.wasLeftReleased = true;
                    if (!dash.isDashing) {
                        kinematic.velocity.x = 0;
                    }
                    break;
                case SDL_SCANCODE_RIGHT | 0x8000: // Affordance for right arrow key
                case SDL_SCANCODE_D | 0x8000:
                    keyboard.movingRight = false;
                    keyboard.wasRightReleased = true;
                    if (!dash.isDashing) {
                        kinematic.velocity.x = 0;
                    }
                    break;
                case SDL_SCANCODE_SPACE | 0x8000:
                    keyboard.wasSpaceReleased = true;
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
};

#endif //KEYBOARD_HPP
