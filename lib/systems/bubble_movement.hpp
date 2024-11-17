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
    static constexpr float GRID_OFFSET_X = (SCREEN_WIDTH - (8 * GRID_SIZE)) / 2.0f;
    static constexpr float GRID_OFFSET_Y = 32.0f;

public:
    void update(float dt) {
        for (auto const &entity: entities) {
            try {
                auto &projectile = gCoordinator.getComponent<BubbleProjectile>(entity);
                if (!projectile.isMoving) {
                    continue;
                }

                auto &transform = gCoordinator.getComponent<Transform>(entity);
                auto &kinematic = gCoordinator.getComponent<CKinematic>(entity);

                // Apply velocity to position directly
                transform.x += projectile.velocity.x * dt;
                transform.y += projectile.velocity.y * dt;

                // Check if bubble should snap to grid
                if (transform.y <= GRID_OFFSET_Y) {
                    std::cout << "\n=== Bubble Hit Grid Line ===" << std::endl;
                    std::cout << "Original position: (" << transform.x << ", " << transform.y << ")" << std::endl;

                    // Ensure bubble doesn't go above grid
                    transform.y = GRID_OFFSET_Y;

                    snapToGrid(entity);
                    projectile.isMoving = false;
                    kinematic.velocity = {0, 0};
                }
            } catch (const std::exception &e) {
                std::cout << "Error processing entity " << entity << ": " << e.what() << std::endl;
            }
        }
    }

private:
    void snapToGrid(Entity entity) {
        try {
            auto& transform = gCoordinator.getComponent<Transform>(entity);

            // Calculate nearest grid position relative to grid origin
            float relativeX = transform.x - GRID_OFFSET_X;
            float gridX = std::round(relativeX / GRID_SIZE) * GRID_SIZE + GRID_OFFSET_X;

            // Record old position for debugging
            float oldX = transform.x;
            float oldY = transform.y;

            // Update position
            transform.x = gridX;
            transform.y = GRID_OFFSET_Y;  // Always snap to first row

            std::cout << "Snapped position from (" << oldX << ", " << oldY
                     << ") to (" << transform.x << ", " << transform.y << ")" << std::endl;

            if (auto gridSystem = gCoordinator.getSystem<BubbleGridSystem>()) {
                gridSystem->addBubble(entity);
            }
        } catch (const std::exception& e) {
            std::cout << "Error in snapToGrid: " << e.what() << std::endl;
        }
    }
};

#endif //BUBBLE_MOVEMENT_HPP
