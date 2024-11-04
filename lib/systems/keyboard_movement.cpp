//
// Created by Utsav Lal on 10/6/24.
//

#include "../ECS/coordinator.hpp"
#include "../ECS/system.hpp"
#include "../model/components.hpp"
#include "../EMS/event_coordinator.hpp"
#include <thread>

extern Coordinator gCoordinator;
extern EventCoordinator eventCoordinator;

class KeyboardMovementSystem : public System {
private:
    // Track previous key states to detect releases
    bool prevAState = false;
    bool prevDState = false;
    bool prevSpaceState = false;

public:
    void update() {
        for (const auto entity: entities) {
            auto &transform = gCoordinator.getComponent<Transform>(entity);
            auto &kinematic = gCoordinator.getComponent<CKinematic>(entity);
            auto &keyboard = gCoordinator.getComponent<KeyboardMovement>(entity);
            auto &jump = gCoordinator.getComponent<Jump>(entity);
            auto &dash = gCoordinator.getComponent<Dash>(entity);
            auto &stomp = gCoordinator.getComponent<Stomp>(entity);

            const Uint8 *state = SDL_GetKeyboardState(nullptr);

            // Handle key presses
            if (state[SDL_SCANCODE_A]) {
                if (!prevAState) {
                    // Only emit when key is first pressed
                    Event event{EventType::EntityInput, EntityInputData{entity, SDLK_a}};
                    eventCoordinator.emit(std::make_shared<Event>(event));
                }
                prevAState = true;
            } else if (prevAState) {
                // Key was released
                Event event{EventType::EntityInput, EntityInputData{entity, SDLK_a | 0x8000}};
                eventCoordinator.emit(std::make_shared<Event>(event));
                prevAState = false;
            }

            if (state[SDL_SCANCODE_D]) {
                if (!prevDState) {
                    // Only emit when key is first pressed
                    Event event{EventType::EntityInput, EntityInputData{entity, SDLK_d}};
                    eventCoordinator.emit(std::make_shared<Event>(event));
                }
                prevDState = true;
            } else if (prevDState) {
                // Key was released
                Event event{EventType::EntityInput, EntityInputData{entity, SDLK_d | 0x8000}};
                eventCoordinator.emit(std::make_shared<Event>(event));
                prevDState = false;
            }

            if (state[SDL_SCANCODE_SPACE] && !jump.isJumping && jump.canJump) {
                Event event{EventType::EntityInput, EntityInputData{entity, SDLK_SPACE}};
                eventCoordinator.emit(std::make_shared<Event>(event));
            }

            // Update dash timing
            if (dash.isDashing) {
                dash.dashTimeRemaining -= 1.0f / 60.0f; // Assuming 60 FPS, adjust as needed
                if (dash.dashTimeRemaining <= 0) {
                    dash.isDashing = false;
                    dash.cooldownTimeRemaining = dash.dashCooldown;
                    // Reset velocity to normal movement speed if still moving
                    if (keyboard.movingLeft) {
                        kinematic.velocity.x = -keyboard.speed;
                    } else if (keyboard.movingRight) {
                        kinematic.velocity.x = keyboard.speed;
                    } else {
                        kinematic.velocity.x = 0;
                    }
                }
            }

            // Update cooldown
            if (dash.cooldownTimeRemaining > 0) {
                dash.cooldownTimeRemaining -= 1.0f / 60.0f; // Assuming 60 FPS, adjust as needed
            }
        }
    }
};
