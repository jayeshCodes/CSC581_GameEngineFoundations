//
// Created by Jayesh Gajbhar on 10/13/24.
//

#pragma once
#include "../ECS/system.hpp"
#include "../model/components.hpp"
#include "../ECS/coordinator.hpp"
#include <mutex>
#include <algorithm>

extern Coordinator gCoordinator;


// the following collision algorithm is based on the sweep and prune algorithm for broad phase collision detection and,
// AABB collision detection for narrow phase collision detection.
// referenced from the following source:
// Building Collision Simulations: An Introduction to Computer Graphics - https://www.youtube.com/watch?v=eED4bSkYCB8
// Co-authored by github copilot

class CollisionSystem : public System {
public:
    void update() {
        std::vector<Entity> entities = gCoordinator.getEntitiesWithComponent<Collision>();

        // Broad phase collision detection (sweep and prune)
        sweepAndPrune(entities);

        // Narrow phase collision detection and resolution
        narrowPhaseCollisionAndResolution(entities);
    }

private:
    static void sweepAndPrune(std::vector<Entity> &entities) {
        // Sort entities based on their x position
        std::sort(entities.begin(), entities.end(), [](Entity a, Entity b) {
            auto &transformA = gCoordinator.getComponent<Transform>(a);
            auto &transformB = gCoordinator.getComponent<Transform>(b);
            return transformA.x < transformB.x;
        });
    }

    static void narrowPhaseCollisionAndResolution(const std::vector<Entity> &entities) {
        for (size_t i = 0; i < entities.size(); ++i) {
            for (size_t j = i + 1; j < entities.size(); ++j) {
                Entity entityA = entities[i];
                Entity entityB = entities[j];

                auto &transformA = gCoordinator.getComponent<Transform>(entityA);
                auto &transformB = gCoordinator.getComponent<Transform>(entityB);
                auto &collisionA = gCoordinator.getComponent<Collision>(entityA);
                auto &collisionB = gCoordinator.getComponent<Collision>(entityB);


                if (collisionA.layer == collisionB.layer) {
                    continue;
                }

                // Perform AABB collision detection
                if (checkAABBCollision(transformA, transformB)) {
                    // Handle collision
                    if (collisionA.isTrigger) {
                        handleTrigger(entityA, entityB);
                    }
                    if (collisionB.isTrigger) {
                        handleTrigger(entityB, entityA);
                    }
                    if (!collisionA.isTrigger && !collisionB.isTrigger) {
                        resolveCollision(entityA, entityB);
                    }
                }
            }
        }
    }

    static bool checkAABBCollision(const Transform &a, const Transform &b) {
        return (a.x < b.x + b.w &&
                a.x + a.w > b.x &&
                a.y < b.y + b.h &&
                a.y + a.h > b.y);
    }

    static void handleTrigger(Entity triggerEntity, Entity otherEntity) {
        Event event{EventType::EntityTriggered, EntityTriggeredData{triggerEntity, otherEntity}};
        eventCoordinator.emit(std::make_shared<Event>(event));
    }

    static void resolveCollision(Entity entityA, Entity entityB) {
        if (!hasRequiredComponents(entityA) || !hasRequiredComponents(entityB)) {
            // Log an error or handle the missing component case
            if (!hasRequiredComponents(entityA))
                std::cerr << "Error: Entity " << entityA << " is missing required components for collision resolution."
                        << std::endl;
            if (!hasRequiredComponents(entityB))
                std::cerr << "Error: Entity " << entityB << " is missing required components for collision resolution."
                        << std::endl;
            return;
        }
        Event event{EventType::EntityCollided, EntityCollidedData{entityA, entityB}};
        eventCoordinator.emit(std::make_shared<Event>(event));
    }

    static bool hasRequiredComponents(Entity entity) {
        return gCoordinator.hasComponent<Transform>(entity) &&
               gCoordinator.hasComponent<RigidBody>(entity) &&
               gCoordinator.hasComponent<CKinematic>(entity);
    }
};
