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
        if (event->type == eventTypeToString(EventType::EntityInput)) {
            const EntityInputData &data = event->data;
            auto entity = data.entity;
            auto &kinematic = gCoordinator.getComponent<CKinematic>(entity);
            auto &keyboard = gCoordinator.getComponent<KeyboardMovement>(entity);

            // Check if this is a key release event
            bool isKeyRelease = (data.key & 0x8000) != 0;
            SDL_Scancode key = static_cast<SDL_Scancode>(data.key & ~0x8000);

            Sprite *sprite = gCoordinator.hasComponent<Sprite>(entity)
                                 ? &gCoordinator.getComponent<Sprite>(entity)
                                 : nullptr;
            Dash *dash = gCoordinator.hasComponent<Dash>(entity) ? &gCoordinator.getComponent<Dash>(entity) : nullptr;
            Jump *jump = gCoordinator.hasComponent<Jump>(entity) ? &gCoordinator.getComponent<Jump>(entity) : nullptr;
            Stomp *stomp = gCoordinator.hasComponent<Stomp>(entity)
                               ? &gCoordinator.getComponent<Stomp>(entity)
                               : nullptr;

            switch (key) {
                case SDL_SCANCODE_LEFT:
                case SDL_SCANCODE_A:
                    if (isKeyRelease) {
                        keyboard.movingLeft = false;
                        keyboard.wasLeftReleased = true;
                        if (!dash || !dash->isDashing) {
                            kinematic.velocity.x = keyboard.movingRight ? keyboard.speed : 0;
                        }
                    } else {
                        if (sprite) sprite->flipX = false;
                        kinematic.velocity.x = -keyboard.speed;
                        keyboard.movingLeft = true;
                    }
                    break;

                case SDL_SCANCODE_RIGHT:
                case SDL_SCANCODE_D:
                    if (isKeyRelease) {
                        keyboard.movingRight = false;
                        keyboard.wasRightReleased = true;
                        if (!dash || !dash->isDashing) {
                            kinematic.velocity.x = keyboard.movingLeft ? -keyboard.speed : 0;
                        }
                    } else {
                        if (sprite) sprite->flipX = true;
                        kinematic.velocity.x = keyboard.speed;
                        keyboard.movingRight = true;
                    }
                    break;

                case SDL_SCANCODE_SPACE:
                    if (isKeyRelease) {
                        keyboard.wasSpaceReleased = true;
                    } else if (jump && jump->canJump && !jump->isJumping) {
                        jump->isJumping = true;
                        jump->canJump = false;
                        jump->jumpTime = 0.f;
                    }
                    break;

                case SDL_SCANCODE_8:
                    if (!isKeyRelease) {
                        Event startReplayEvent{eventTypeToString(EventType::StartRecording), {}};
                        eventCoordinator.emit(std::make_shared<Event>(startReplayEvent));
                    }
                    break;

                case SDL_SCANCODE_9:
                    if (!isKeyRelease) {
                        Event stopReplayEvent{eventTypeToString(EventType::StopRecording), {}};
                        eventCoordinator.emit(std::make_shared<Event>(stopReplayEvent));
                    }
                    break;

                case SDL_SCANCODE_0:
                    if (!isKeyRelease) {
                        Event replayReplayEvent{eventTypeToString(EventType::StartReplaying), {}};
                        eventCoordinator.emit(std::make_shared<Event>(replayReplayEvent));
                    }
                    break;
                case SDL_SCANCODE_P:
                    if (!isKeyRelease) {
                        if (!gameTimeline.isPaused()) {
                            gameTimeline.pause();
                        } else {
                            gameTimeline.start();
                        }
                    }
            }
        }
    };

public:
    KeyboardSystem() {
        eventCoordinator.subscribe(keyboardHandler, eventTypeToString(EventType::EntityInput));
    }

    ~KeyboardSystem() {
        eventCoordinator.unsubscribe(keyboardHandler, eventTypeToString(EventType::EntityInput));
    }
};

#endif //KEYBOARD_HPP
