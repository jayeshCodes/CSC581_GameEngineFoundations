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

            // Get current time for double-tap detection
            auto currentTime = gameTimeline.getElapsedTime();

            switch (data.key) {
                // handle keypress events
                case SDLK_LEFT: // Affordance for left arrow key
                case SDLK_a: {
                    kinematic.velocity.x = -keyboard.speed;
                    keyboard.movingLeft = true;

                    // Double-tap detection for left dash
                    if (keyboard.wasLeftReleased) {
                        auto timeSinceLastTap = (currentTime - keyboard.lastLeftTapTime) / 1000.f;


                        if (timeSinceLastTap <= KeyboardMovement::doubleTapThreshold &&
                            !dash.isDashing && dash.cooldownTimeRemaining <= 0) {
                            // Trigger dash
                            dash.isDashing = true;
                            dash.dashTimeRemaining = dash.dashDuration;
                            kinematic.velocity.x = -dash.dashSpeed;
                        }
                        keyboard.lastLeftTapTime = currentTime;
                        keyboard.wasLeftReleased = false;
                    }
                    break;
                }
                case SDLK_RIGHT: // Affordance for right arrow key
                case SDLK_d: {
                    kinematic.velocity.x = keyboard.speed;
                    keyboard.movingRight = true;

                    // Double-tap detection for right dash
                    if (keyboard.wasRightReleased) {
                        auto timeSinceLastTap = (currentTime - keyboard.lastRightTapTime) / 1000.f;


                        if (timeSinceLastTap <= KeyboardMovement::doubleTapThreshold &&
                            !dash.isDashing && dash.cooldownTimeRemaining <= 0) {
                            // Trigger dash
                            dash.isDashing = true;
                            dash.dashTimeRemaining = dash.dashDuration;
                            kinematic.velocity.x = dash.dashSpeed;
                        }
                        keyboard.lastRightTapTime = currentTime;
                        keyboard.wasRightReleased = false;
                    }
                    break;
                case SDLK_SPACE:
                    if (jump.canJump && !jump.isJumping) {
                        kinematic.velocity.y = -jump.initialJumpVelocity;
                        jump.isJumping = false;
                        jump.canJump = false;
                        jump.jumpTime = 0.f;
                    }
                    break;
                }
                // handle key release events
                case SDLK_LEFT | 0x8000: // Affordance for left arrow key
                case SDLK_a | 0x8000:
                    keyboard.movingLeft = false;
                    keyboard.wasLeftReleased = true;
                    if (!keyboard.movingRight && !dash.isDashing) {
                        kinematic.velocity.x = 0;
                    }
                    break;
                case SDLK_RIGHT | 0x8000: // Affordance for right arrow key
                case SDLK_d | 0x8000:
                    keyboard.movingRight = false;
                    keyboard.wasRightReleased = true;
                    if (!keyboard.movingLeft && !dash.isDashing) {
                        kinematic.velocity.x = 0;
                    }
                    break;
                case SDLK_SPACE | 0x8000:
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
