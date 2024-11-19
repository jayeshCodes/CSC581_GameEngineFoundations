//
// Created by Jayesh Gajbhar on 11/14/24.
//

#ifndef BUBBLE_MOVEMENT_HPP
#define BUBBLE_MOVEMENT_HPP

#include "../ECS/system.hpp"
#include "../model/components.hpp"
#include "../ECS/coordinator.hpp"
#include <iostream>

extern Coordinator gCoordinator;

class BubbleMovementSystem : public System {
private:
    static constexpr float GRID_SIZE = 32.0f;
    static constexpr float GRID_COLS = 16;
    static constexpr float GRID_OFFSET_X = (SCREEN_WIDTH - (GRID_COLS * GRID_SIZE)) / 2.0f;
    static constexpr float GRID_OFFSET_Y = 32.0f;

    bool checkBoundaryCollision(Transform &transform, float &velocityX) {
        // Left boundary
        if (transform.x <= GRID_OFFSET_X) {
            transform.x = GRID_OFFSET_X;
            velocityX = std::abs(velocityX); // Bounce right
            return true;
        }
        // Right boundary
        if (transform.x + transform.w >= GRID_OFFSET_X + (GRID_COLS * GRID_SIZE)) {
            transform.x = GRID_OFFSET_X + (GRID_COLS * GRID_SIZE) - transform.w;
            velocityX = -std::abs(velocityX); // Bounce left
            return true;
        }
        return false;
    }

    bool checkBubbleCollision(const Transform &transform) {
        auto bubbles = gCoordinator.getEntitiesWithComponent<Bubble>();
        float margin = GRID_SIZE * 0.8f; // Collision radius

        for (auto entity: bubbles) {
            auto &otherTransform = gCoordinator.getComponent<Transform>(entity);

            // Calculate distance between centers
            float dx = transform.x - otherTransform.x;
            float dy = transform.y - otherTransform.y;
            float distance = std::sqrt(dx * dx + dy * dy);

            if (distance <= margin) {
                return true;
            }
        }
        return false;
    }

public:
    void update(float dt) {
        for (auto const &entity: entities) {
            try {
                if (!gCoordinator.hasComponent<BubbleProjectile>(entity)) continue;

                auto &projectile = gCoordinator.getComponent<BubbleProjectile>(entity);
                if (!projectile.isMoving) continue;

                auto &transform = gCoordinator.getComponent<Transform>(entity);

                // Store previous position
                float prevX = transform.x;
                float prevY = transform.y;

                // Update position
                transform.x += projectile.velocity.x * dt;
                transform.y += projectile.velocity.y * dt;

                // Check boundary collisions
                if (checkBoundaryCollision(transform, projectile.velocity.x)) {
                    transform.x = prevX; // Revert X movement
                }

                // Check bubble collisions
                if (checkBubbleCollision(transform)) {
                    // Revert to previous position
                    transform.x = prevX;
                    transform.y = prevY;

                    projectile.isMoving = false;
                    projectile.velocity = {0, 0};

                    if (auto gridSystem = gCoordinator.getSystem<BubbleGridSystem>()) {
                        gridSystem->addBubble(entity);
                    }
                    continue;
                }

                // Check top boundary separately
                if (transform.y <= GRID_OFFSET_Y) {
                    transform.y = GRID_OFFSET_Y;
                    projectile.isMoving = false;
                    projectile.velocity = {0, 0};

                    // Snap to grid
                    float relativeX = transform.x - GRID_OFFSET_X;
                    transform.x = std::round(relativeX / GRID_SIZE) * GRID_SIZE + GRID_OFFSET_X;

                    if (auto gridSystem = gCoordinator.getSystem<BubbleGridSystem>()) {
                        gridSystem->addBubble(entity);
                    }
                }
            } catch (const std::exception &e) {
                std::cerr << "Error processing entity " << entity << ": " << e.what() << std::endl;
            }
        }
    }
};

#endif //BUBBLE_MOVEMENT_HPP
