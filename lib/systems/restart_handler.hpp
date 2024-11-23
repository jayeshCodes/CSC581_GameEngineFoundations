//
// Created by Jayesh Gajbhar on 11/23/24.
//

#ifndef RESTART_HANDLER_HPP
#define RESTART_HANDLER_HPP

#include "../ECS/coordinator.hpp"
#include "../ECS/system.hpp"
#include "../model/components.hpp"
#include "../model/event.hpp"
#include "../EMS/event_coordinator.hpp"
#include "../core/defs.hpp"

extern Coordinator gCoordinator;
extern EventCoordinator eventCoordinator;
extern std::string mainCharID;

class RestartHandlerSystem : public System {
private:
    const float INITIAL_Y = SCREEN_HEIGHT - 240.0f;
    const float INITIAL_X = SCREEN_WIDTH / 2;
    const float PLAYER_SPEED = 200.0f;

    void resetGame() {
        // Reset camera position
        for (auto& [id, entity] : gCoordinator.getEntityIds()) {
            if (gCoordinator.hasComponent<Camera>(entity)) {
                auto& camera = gCoordinator.getComponent<Camera>(entity);
                camera.x = 0;
                camera.y = 0;
                camera.zoom = 1.0f;
                camera.rotation = 0.0f;
            }
        }

        // Find and reset player
        for (auto& [id, entity] : gCoordinator.getEntityIds()) {
            if (id == mainCharID) {
                // Reset transform
                auto& transform = gCoordinator.getComponent<Transform>(entity);
                transform.x = INITIAL_X;
                transform.y = INITIAL_Y;

                // Reset kinematics
                if (gCoordinator.hasComponent<CKinematic>(entity)) {
                    auto& kinematic = gCoordinator.getComponent<CKinematic>(entity);
                    kinematic.velocity = {0, 0};
                    kinematic.acceleration = {0, 0};
                    kinematic.rotation = 0;
                    kinematic.angular_acceleration = 0;
                }

                // Reset keyboard movement
                if (gCoordinator.hasComponent<KeyboardMovement>(entity)) {
                    auto& movement = gCoordinator.getComponent<KeyboardMovement>(entity);
                    movement.speed = PLAYER_SPEED;
                    movement.movingLeft = false;
                    movement.movingRight = false;
                    movement.movingUp = false;
                    movement.movingDown = false;
                }

                // Reset jump if it exists
                if (gCoordinator.hasComponent<Jump>(entity)) {
                    auto& jump = gCoordinator.getComponent<Jump>(entity);
                    jump.isJumping = false;
                    jump.canJump = true;
                    jump.jumpTime = 0;
                }

                // Reset score if it exists
                if (gCoordinator.hasComponent<Score>(entity)) {
                    auto& score = gCoordinator.getComponent<Score>(entity);
                    score.score = 0;
                    score.highestY = INITIAL_Y;
                }

                // Reset respawnable if it exists
                if (gCoordinator.hasComponent<Respawnable>(entity)) {
                    auto& respawnable = gCoordinator.getComponent<Respawnable>(entity);
                    respawnable.isDead = false;
                }
            }
        }

        // Destroy all monsters
        std::vector<Entity> monstersToDestroy;
        for (auto& [id, entity] : gCoordinator.getEntityIds()) {
            if (gCoordinator.hasComponent<Monster>(entity)) {
                monstersToDestroy.push_back(entity);
            }
        }

        // Destroy monsters outside the loop to avoid modifying collection while iterating
        for (auto entity : monstersToDestroy) {
            gCoordinator.destroyEntity(entity);
        }

        // Reset platform spawner if it exists
        for (auto& entity : entities) {
            if (gCoordinator.hasComponent<PlatformSpawner>(entity)) {
                auto& spawner = gCoordinator.getComponent<PlatformSpawner>(entity);
                // Reset any spawner-specific state
                // Add any specific platform spawner reset logic here
            }
        }

        // Emit game started event if needed
        Event gameStartedEvent{eventTypeToString(EventType::GameStarted), {}};
        eventCoordinator.emit(std::make_shared<Event>(gameStartedEvent));
    }

    EventHandler restartHandler = [this](const std::shared_ptr<Event>& event) {
        if (event->type == eventTypeToString(EventType::GameRestart)) {
            resetGame();
        }
    };

public:
    RestartHandlerSystem() {
        eventCoordinator.subscribe(restartHandler, eventTypeToString(EventType::GameRestart));
    }

    ~RestartHandlerSystem() {
        eventCoordinator.unsubscribe(restartHandler, eventTypeToString(EventType::GameRestart));
    }
};

#endif
