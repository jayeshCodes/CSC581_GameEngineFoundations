//
// Created by Jayesh Gajbhar on 10/13/24.
//

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
    void sweepAndPrune(std::vector<Entity>& entities) {
        // Sort entities based on their x position
        std::sort(entities.begin(), entities.end(), [](Entity a, Entity b) {
            auto& transformA = gCoordinator.getComponent<Transform>(a);
            auto& transformB = gCoordinator.getComponent<Transform>(b);
            return transformA.x < transformB.x;
        });
    }

    void narrowPhaseCollisionAndResolution(const std::vector<Entity>& entities) {
        for (size_t i = 0; i < entities.size(); ++i) {
            for (size_t j = i + 1; j < entities.size(); ++j) {
                Entity entityA = entities[i];
                Entity entityB = entities[j];

                auto& transformA = gCoordinator.getComponent<Transform>(entityA);
                auto& transformB = gCoordinator.getComponent<Transform>(entityB);
                auto& collisionA = gCoordinator.getComponent<Collision>(entityA);
                auto& collisionB = gCoordinator.getComponent<Collision>(entityB);

                // Check if both entities are colliders
                if (!collisionA.isCollider || !collisionB.isCollider) {
                    continue;
                }

                // Perform AABB collision detection
                if (checkAABBCollision(transformA, transformB)) {
                    // Handle collision
                    if (collisionA.isTrigger || collisionB.isTrigger) {
                        handleTrigger(entityA, entityB);
                    } else {
                        resolveCollision(entityA, entityB);
                    }
                }
            }
        }
    }

    bool checkAABBCollision(const Transform& a, const Transform& b) {
        return (a.x < b.x + b.w &&
                a.x + a.w > b.x &&
                a.y < b.y + b.h &&
                a.y + a.h > b.y);
    }

    void handleTrigger(Entity triggerEntity, Entity otherEntity) {
        // Implement trigger logic here
        // For example, you might want to notify a game event system
        // or directly modify component states
    }

    void resolveCollision(Entity entityA, Entity entityB) {
        auto& transformA = gCoordinator.getComponent<Transform>(entityA);
        auto& transformB = gCoordinator.getComponent<Transform>(entityB);

        // Calculate the overlap on both axes
        float overlapX = std::min(transformA.x + transformA.w, transformB.x + transformB.w) -
                         std::max(transformA.x, transformB.x);
        float overlapY = std::min(transformA.y + transformA.h, transformB.y + transformB.h) -
                         std::max(transformA.y, transformB.y);

        // Determine the separation direction (the axis with the smaller overlap)
        if (overlapX < overlapY) {
            // Separate on X-axis
            if (transformA.x < transformB.x) {
                transformA.x -= overlapX / 2;
                transformB.x += overlapX / 2;
            } else {
                transformA.x += overlapX / 2;
                transformB.x -= overlapX / 2;
            }
        } else {
            // Separate on Y-axis
            if (transformA.y < transformB.y) {
                transformA.y -= overlapY / 2;
                transformB.y += overlapY / 2;
            } else {
                transformA.y += overlapY / 2;
                transformB.y -= overlapY / 2;
            }
        }

        // If the entities have Kinematic components, adjust their velocities
        if (gCoordinator.hasComponent<CKinematic>(entityA) && gCoordinator.hasComponent<CKinematic>(entityB)) {
            auto& kinematicA = gCoordinator.getComponent<CKinematic>(entityA);
            auto& kinematicB = gCoordinator.getComponent<CKinematic>(entityB);

            // Simple elastic collision response
            SDL_FPoint tempVelocity = kinematicA.velocity;
            kinematicA.velocity = kinematicB.velocity;
            kinematicB.velocity = tempVelocity;
        }
    }
};