//
// Created by Jayesh Gajbhar on 11/23/24.
//

#ifndef MONSTER_COLLISION_HPP
#define MONSTER_COLLISION_HPP

#include "../ECS/coordinator.hpp"
#include "../ECS/system.hpp"
#include "../model/components.hpp"
#include "../model/event.hpp"
#include "../EMS/event_coordinator.hpp"

extern Coordinator gCoordinator;
extern EventCoordinator eventCoordinator;


class MonsterCollisionSystem : public System {
private:
    bool checkCollision(const Transform &a, const Transform &b, float tolerance = 20.0f) {
        // Shrink the collision boxes by the tolerance amount
        float ax = a.x + tolerance;
        float ay = a.y + tolerance;
        float aw = a.w - (2 * tolerance);
        float ah = a.h - (2 * tolerance);

        float bx = b.x + tolerance;
        float by = b.y + tolerance;
        float bw = b.w - (2 * tolerance);
        float bh = b.h - (2 * tolerance);

        return (ax < bx + bw &&
                ax + aw > bx &&
                ay < by + bh &&
                ay + ah > by);
    }

public:
    void update() {
        // First find the player
        Entity player = INVALID_ENTITY;
        for (auto &entity: entities) {
            if (gCoordinator.hasComponent<MainChar>(entity)) {
                player = entity;
                break;
            }
        }

        // If no player found, return
        if (player == INVALID_ENTITY) {
            return;
        }

        auto &playerTransform = gCoordinator.getComponent<Transform>(player);

        // Then check collision with all monsters
        for (auto &entity: entities) {
            if (!gCoordinator.hasComponent<Monster>(entity)) {
                continue;
            }

            auto &monsterTransform = gCoordinator.getComponent<Transform>(entity);

            if (checkCollision(playerTransform, monsterTransform)) {
                Event gameOverEvent{
                    eventTypeToString(EventType::GameOver),
                    GameOverData{player, entity}
                };
                eventCoordinator.emit(std::make_shared<Event>(gameOverEvent));
                return;
            }
        }
    }
};

#endif
