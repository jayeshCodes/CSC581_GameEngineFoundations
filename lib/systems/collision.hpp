//
// Created by Jayesh Gajbhar on 10/13/24.
//

#pragma once
class CollisionSystem : public System {
public:
    void update() {
        try {
            // Get all entities with Collision component
            std::vector<Entity> entities = gCoordinator.getEntitiesWithComponent<Collision>();

            if (entities.empty()) {
                return;
            }

            // Broad phase

            sweepAndPrune(entities);

            // Narrow phase

            narrowPhaseCollisionAndResolution(entities);
        } catch (const std::exception &e) {
            std::cerr << "Error in collision system update: " << e.what() << std::endl;
        }
    }

private:
    static void sweepAndPrune(std::vector<Entity> &entities) {
        try {
            // Sort entities based on their x position
            std::sort(entities.begin(), entities.end(), [](Entity a, Entity b) {
                if (!gCoordinator.hasComponent<Transform>(a) || !gCoordinator.hasComponent<Transform>(b)) {
                    return false;
                }
                auto &transformA = gCoordinator.getComponent<Transform>(a);
                auto &transformB = gCoordinator.getComponent<Transform>(b);
                return transformA.x < transformB.x;
            });
        } catch (const std::exception &e) {
            std::cerr << "Error in sweep and prune: " << e.what() << std::endl;
        }
    }

    static void narrowPhaseCollisionAndResolution(const std::vector<Entity> &entities) {
        for (size_t i = 0; i < entities.size(); ++i) {
            for (size_t j = i + 1; j < entities.size(); ++j) {
                try {
                    Entity entityA = entities[i];
                    Entity entityB = entities[j];

                    // Verify all required components exist
                    if (!hasRequiredComponents(entityA) || !hasRequiredComponents(entityB)) {
                        continue;
                    }

                    auto &transformA = gCoordinator.getComponent<Transform>(entityA);
                    auto &transformB = gCoordinator.getComponent<Transform>(entityB);
                    auto &collisionA = gCoordinator.getComponent<Collision>(entityA);
                    auto &collisionB = gCoordinator.getComponent<Collision>(entityB);

                    // Skip if same layer
                    if (collisionA.layer == collisionB.layer) {
                        continue;
                    }

                    // Check collision
                    if (checkAABBCollision(transformA, transformB)) {
                        // Only emit collision event for moving objects (e.g., bubbles) colliding with boundaries
                        bool isABubble = gCoordinator.hasComponent<BubbleProjectile>(entityA);
                        bool isBBubble = gCoordinator.hasComponent<BubbleProjectile>(entityB);

                        if (isABubble || isBBubble) {
                            Event event{
                                eventTypeToString(EventType::EntityCollided),
                                EntityCollidedData{entityA, entityB}
                            };
                            eventCoordinator.emit(std::make_shared<Event>(event));
                        }
                    }
                } catch (const std::exception &e) {
                    std::cerr << "Error processing collision pair: " << e.what() << std::endl;
                }
            }
        }
    }

    static bool hasRequiredComponents(Entity entity) {
        return gCoordinator.hasComponent<Transform>(entity) &&
               gCoordinator.hasComponent<Collision>(entity);
    }

    static bool checkAABBCollision(const Transform &a, const Transform &b) {
        return (a.x < b.x + b.w &&
                a.x + a.w > b.x &&
                a.y < b.y + b.h &&
                a.y + a.h > b.y);
    }
};
