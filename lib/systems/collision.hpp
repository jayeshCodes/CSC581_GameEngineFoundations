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
    void sweepAndPrune(std::vector<Entity> &entities) {
        // Sort entities based on their x position
        std::sort(entities.begin(), entities.end(), [](Entity a, Entity b) {
            auto &transformA = gCoordinator.getComponent<Transform>(a);
            auto &transformB = gCoordinator.getComponent<Transform>(b);
            return transformA.x < transformB.x;
        });
    }

    void narrowPhaseCollisionAndResolution(const std::vector<Entity> &entities) {
        for (size_t i = 0; i < entities.size(); ++i) {
            for (size_t j = i + 1; j < entities.size(); ++j) {
                Entity entityA = entities[i];
                Entity entityB = entities[j];

                auto &transformA = gCoordinator.getComponent<Transform>(entityA);
                auto &transformB = gCoordinator.getComponent<Transform>(entityB);
                auto &collisionA = gCoordinator.getComponent<Collision>(entityA);
                auto &collisionB = gCoordinator.getComponent<Collision>(entityB);

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

    bool checkAABBCollision(const Transform &a, const Transform &b) {
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
        if (!hasRequiredComponents(entityA) || !hasRequiredComponents(entityB)) {
            // Log an error or handle the missing component case
            if(!hasRequiredComponents(entityA))
                std::cerr << "Error: Entity " << entityA << " is missing required components for collision resolution." << std::endl;
            if (!hasRequiredComponents(entityB))
                std::cerr << "Error: Entity " << entityB << " is missing required components for collision resolution." << std::endl;
            return;
        }
        auto &transformA = gCoordinator.getComponent<Transform>(entityA);
        auto &transformB = gCoordinator.getComponent<Transform>(entityB);
        auto &rigidBodyA = gCoordinator.getComponent<RigidBody>(entityA);
        auto &rigidBodyB = gCoordinator.getComponent<RigidBody>(entityB);
        auto &kinematicA = gCoordinator.getComponent<CKinematic>(entityA);
        auto &kinematicB = gCoordinator.getComponent<CKinematic>(entityB);


        // Check if either object is immovable
        bool immovableA = rigidBodyA.mass < 0;
        bool immovableB = rigidBodyB.mass < 0;

        // Calculate the overlap on both axes
        float overlapX = std::min(transformA.x + transformA.w, transformB.x + transformB.w) -
                         std::max(transformA.x, transformB.x);
        float overlapY = std::min(transformA.y + transformA.h, transformB.y + transformB.h) -
                         std::max(transformA.y, transformB.y);

        // Determine the separation direction (the axis with the smaller overlap)
        if (overlapX < overlapY) {
            // Separate on X-axis
            if (!immovableA && !immovableB) {
                float totalMass = rigidBodyA.mass + rigidBodyB.mass;
                float ratioA = rigidBodyB.mass / totalMass;
                float ratioB = rigidBodyA.mass / totalMass;

                if (transformA.x < transformB.x) {
                    transformA.x -= overlapX * ratioA;
                    transformB.x += overlapX * ratioB;
                } else {
                    transformA.x += overlapX * ratioA;
                    transformB.x -= overlapX * ratioB;
                }
            } else if (!immovableA) {
                transformA.x += (transformA.x < transformB.x) ? -overlapX : overlapX;
            } else if (!immovableB) {
                transformB.x += (transformB.x < transformA.x) ? -overlapX : overlapX;
            }
        } else {
            // Separate on Y-axis
            if (!immovableA && !immovableB) {
                float totalMass = rigidBodyA.mass + rigidBodyB.mass;
                float ratioA = rigidBodyB.mass / totalMass;
                float ratioB = rigidBodyA.mass / totalMass;

                if (transformA.y < transformB.y) {
                    transformA.y -= overlapY * ratioA;
                    transformB.y += overlapY * ratioB;
                } else {
                    transformA.y += overlapY * ratioA;
                    transformB.y -= overlapY * ratioB;
                }
            } else if (!immovableA) {
                transformA.y += (transformA.y < transformB.y) ? -overlapY : overlapY;
            } else if (!immovableB) {
                transformB.y += (transformB.y < transformA.y) ? -overlapY : overlapY;
            }
        }

        // Adjust velocities based on mass and elasticity
        if (!immovableA && !immovableB) {
            // Calculate relative velocity
            SDL_FPoint relativeVelocity = {
                kinematicB.velocity.x - kinematicA.velocity.x,
                kinematicB.velocity.y - kinematicA.velocity.y
            };

            // Calculate collision normal
            SDL_FPoint normal = {
                transformB.x - transformA.x,
                transformB.y - transformA.y
            };
            float normalLength = std::sqrt(normal.x * normal.x + normal.y * normal.y);
            normal.x /= normalLength;
            normal.y /= normalLength;

            // Calculate relative velocity along the normal
            float velocityAlongNormal = relativeVelocity.x * normal.x + relativeVelocity.y * normal.y;

            // Do not resolve if velocities are separating
            if (velocityAlongNormal > 0)
                return;

            // Calculate restitution (elasticity)
            float e = 0.8f; // You might want to make this configurable or part of the RigidBody component

            // Calculate impulse scalar
            float j = -(1 + e) * velocityAlongNormal;
            j /= 1 / rigidBodyA.mass + 1 / rigidBodyB.mass;

            // Apply impulse
            SDL_FPoint impulse = {j * normal.x, j * normal.y};

            kinematicA.velocity.x -= impulse.x / rigidBodyA.mass;
            kinematicA.velocity.y -= impulse.y / rigidBodyA.mass;
            kinematicB.velocity.x += impulse.x / rigidBodyB.mass;
            kinematicB.velocity.y += impulse.y / rigidBodyB.mass;
        } else {
            // At least one object is immovable
            Entity movableEntity = immovableA ? entityB : entityA;
            Entity immovableEntity = immovableA ? entityA : entityB;

            auto& movableTransform = immovableA ? transformB : transformA;
            auto& immovableTransform = immovableA ? transformA : transformB;
            auto& movableKinematic = immovableA ? kinematicB : kinematicA;

            // Determine the direction of collision
            bool fromTop = movableTransform.y + movableTransform.h - overlapY <= immovableTransform.y;
            bool fromLeft = movableTransform.x + movableTransform.w - overlapX <= immovableTransform.x;

            const float epsilon = 0.01f; // Small value to prevent floating point errors

            if (fromTop && overlapY < overlapX) {
                // Movable object is on top of immovable object
                movableTransform.y = immovableTransform.y - movableTransform.h - epsilon;
                if (movableKinematic.velocity.y > 0) {
                    movableKinematic.velocity.y = 0;  // Only stop downward movement
                }
            } else if (fromLeft) {
                // Movable object is to the left of immovable object
                movableTransform.x = immovableTransform.x - movableTransform.w - epsilon;
                if (movableKinematic.velocity.x > 0) {
                    movableKinematic.velocity.x = 0;  // Only stop rightward movement
                }
            } else if (!fromTop && overlapY < overlapX) {
                // Movable object is below immovable object
                movableTransform.y = immovableTransform.y + immovableTransform.h + epsilon;
                if (movableKinematic.velocity.y < 0) {
                    movableKinematic.velocity.y = 0;  // Only stop upward movement
                }
            } else {
                // Movable object is to the right of immovable object
                movableTransform.x = immovableTransform.x + immovableTransform.w + epsilon;
                if (movableKinematic.velocity.x < 0) {
                    movableKinematic.velocity.x = 0;  // Only stop leftward movement
                }
            }

            // Update the Jump component if it exists
            if (gCoordinator.hasComponent<Jump>(movableEntity)) {
                auto& jump = gCoordinator.getComponent<Jump>(movableEntity);
                if (fromTop && overlapY < overlapX) {
                    jump.canJump = true;  // Allow jumping when on top of a platform
                }
            }
        }
    }

    bool hasRequiredComponents(Entity entity) {
        return gCoordinator.hasComponent<Transform>(entity) &&
               gCoordinator.hasComponent<RigidBody>(entity) &&
               gCoordinator.hasComponent<CKinematic>(entity);
    }
};
