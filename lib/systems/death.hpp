//
// Created by Jayesh Gajbhar on 10/14/24.
//

#pragma once
#include "../ECS/system.hpp"
#include "../model/components.hpp"
#include "../ECS/coordinator.hpp"

extern Coordinator gCoordinator;
extern EventCoordinator eventCoordinator;
extern std::string mainCharID;

class DeathSystem : public System {
    const float DEATH_OFFSET = 200.0f;  // How far below camera view to trigger death

    float getCameraY() const {
        for (auto& [id, entity] : gCoordinator.getEntityIds()) {
            if (gCoordinator.hasComponent<Camera>(entity)) {
                return gCoordinator.getComponent<Camera>(entity).y;
            }
        }
        return 0.0f;
    }

public:
    void update() const {
        float cameraY = getCameraY();
        float deathY = cameraY + SCREEN_HEIGHT + DEATH_OFFSET;

        for (auto& entity : entities) {
            if (!gCoordinator.getEntityIds().contains(gCoordinator.getEntityKey(entity))) {
                continue;
            }

            auto& transform = gCoordinator.getComponent<Transform>(entity);
            auto& respawnable = gCoordinator.getComponent<Respawnable>(entity);

            // Check if entity has fallen too far below camera view
            if (transform.y > deathY && !respawnable.isDead) {
                std::cout << "Entity fell below death line! Camera Y: " << cameraY
                         << " Entity Y: " << transform.y
                         << " Death line: " << deathY << std::endl;

                // If this is the main character, trigger game over
                if (gCoordinator.getEntityKey(entity) == mainCharID) {
                    std::cout << "Main character died - triggering game over" << std::endl;

                    Event gameOverEvent{
                        eventTypeToString(EventType::GameOver),
                        GameOverData{entity, INVALID_ENTITY}  // No monster involved in fall death
                    };
                    eventCoordinator.emit(std::make_shared<Event>(gameOverEvent));
                }

                respawnable.isDead = true;
            }

            // Also check if the player has collided with any monster
            if (gCoordinator.getEntityKey(entity) == mainCharID &&
                gCoordinator.hasComponent<Collision>(entity)) {

                auto& playerCollision = gCoordinator.getComponent<Collision>(entity);
                if (playerCollision.isCollider) {
                    for (auto& [id, otherEntity] : gCoordinator.getEntityIds()) {
                        if (gCoordinator.hasComponent<Monster>(otherEntity) &&
                            gCoordinator.hasComponent<Collision>(otherEntity)) {

                            auto& monsterCollision = gCoordinator.getComponent<Collision>(otherEntity);
                            if (monsterCollision.isCollider && checkCollision(transform,
                                gCoordinator.getComponent<Transform>(otherEntity))) {

                                std::cout << "Player collided with monster - triggering game over" << std::endl;

                                Event gameOverEvent{
                                    eventTypeToString(EventType::GameOver),
                                    GameOverData{entity, otherEntity}
                                };
                                eventCoordinator.emit(std::make_shared<Event>(gameOverEvent));
                                respawnable.isDead = true;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

private:
    bool checkCollision(const Transform& a, const Transform& b) const {
        return (a.x < b.x + b.w &&
                a.x + a.w > b.x &&
                a.y < b.y + b.h &&
                a.y + a.h > b.y);
    }
};
