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
    static constexpr float GRID_COLS = 15;
    static constexpr float GRID_OFFSET_X = (SCREEN_WIDTH - (GRID_COLS * GRID_SIZE)) / 2.0f;
    static constexpr float GRID_OFFSET_Y = 32.0f;
    static constexpr float SNAP_THRESHOLD = GRID_SIZE * 0.8f; // Threshold for snapping
    static constexpr float COLLISION_THRESHOLD = GRID_SIZE * 0.85f; // Threshold for bubble collision

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

    SDL_FPoint calculateExactGridPosition(float x, float y) {
        // Get relative position within grid
        float relativeX = x - GRID_OFFSET_X;
        float relativeY = y - GRID_OFFSET_Y;

        // Calculate row and column
        int row = static_cast<int>(round(relativeY / GRID_SIZE));
        int col = static_cast<int>(round(relativeX / GRID_SIZE));

        // Adjust position for odd rows
        if (row % 2 == 1) {
            // For odd rows, offset by half grid size
            col = static_cast<int>(round((relativeX - GRID_SIZE / 2) / GRID_SIZE));
            return {
                GRID_OFFSET_X + (col * GRID_SIZE) + GRID_SIZE / 2,
                GRID_OFFSET_Y + (row * GRID_SIZE)
            };
        }

        // Even rows align normally
        return {
            GRID_OFFSET_X + (col * GRID_SIZE),
            GRID_OFFSET_Y + (row * GRID_SIZE)
        };
    }

    bool checkBubbleCollision(const Transform &transform, SDL_FPoint &snapPos) {
        auto bubbles = gCoordinator.getEntitiesWithComponent<Bubble>();
        bool collision = false;
        float closestDistance = std::numeric_limits<float>::max();
        SDL_FPoint bestSnapPos{transform.x, transform.y};

        for (auto entity: bubbles) {
            auto &otherTransform = gCoordinator.getComponent<Transform>(entity);

            float dx = transform.x - otherTransform.x;
            float dy = transform.y - otherTransform.y;
            float distance = std::sqrt(dx * dx + dy * dy);

            if (distance < COLLISION_THRESHOLD) {
                collision = true;
                if (distance < closestDistance) {
                    closestDistance = distance;

                    // Calculate potential snap positions around collision point
                    std::vector<SDL_FPoint> potentialPositions;
                    float angles[] = {0, M_PI / 3, 2 * M_PI / 3, M_PI, 4 * M_PI / 3, 5 * M_PI / 3};

                    for (float angle: angles) {
                        float snapX = otherTransform.x + GRID_SIZE * cos(angle);
                        float snapY = otherTransform.y + GRID_SIZE * sin(angle);
                        potentialPositions.push_back(calculateExactGridPosition(snapX, snapY));
                    }

                    // Find closest valid snap position
                    float minSnapDistance = std::numeric_limits<float>::max();
                    for (const auto &pos: potentialPositions) {
                        float snapDx = transform.x - pos.x;
                        float snapDy = transform.y - pos.y;
                        float snapDistance = std::sqrt(snapDx * snapDx + snapDy * snapDy);

                        if (snapDistance < minSnapDistance) {
                            minSnapDistance = snapDistance;
                            bestSnapPos = pos;
                        }
                    }
                }
            }
        }

        if (collision) {
            snapPos = bestSnapPos;
        }
        return collision;
    }

    SDL_FPoint calculateSnapPosition(float x, float y) {
        // Calculate the nearest grid position
        float relativeX = x - GRID_OFFSET_X;
        float relativeY = y - GRID_OFFSET_Y;

        // Calculate row and column
        int row = static_cast<int>(round((relativeY) / GRID_SIZE));
        int col = static_cast<int>(round((relativeX) / GRID_SIZE));

        // Adjust for odd rows
        if (row % 2 == 1) {
            relativeX -= GRID_SIZE / 2.0f;
            col = static_cast<int>(round((relativeX) / GRID_SIZE));
            relativeX = col * GRID_SIZE + GRID_SIZE / 2.0f;
        } else {
            relativeX = col * GRID_SIZE;
        }

        return {
            GRID_OFFSET_X + relativeX,
            GRID_OFFSET_Y + (row * GRID_SIZE)
        };
    }

public:
    void update(float dt) {
        for (auto const &entity: entities) {
            if (!gCoordinator.hasComponent<BubbleProjectile>(entity)) continue;

            auto &projectile = gCoordinator.getComponent<BubbleProjectile>(entity);
            if (!projectile.isMoving) continue;

            auto &transform = gCoordinator.getComponent<Transform>(entity);

            // Update position
            transform.x += projectile.velocity.x * dt;
            transform.y += projectile.velocity.y * dt;

            // Handle boundary collisions
            if (transform.x <= GRID_OFFSET_X ) {
                transform.x = GRID_OFFSET_X;
                projectile.velocity.x = std::abs(projectile.velocity.x);
            } else if (transform.x >= GRID_OFFSET_X + (GRID_COLS * GRID_SIZE) - GRID_SIZE) {
                transform.x = GRID_OFFSET_X + (GRID_COLS * GRID_SIZE) - GRID_SIZE;
                projectile.velocity.x = -std::abs(projectile.velocity.x);
            }

            SDL_FPoint snapPos;
            bool shouldSnap = false;

            // Check for top boundary
            if (transform.y <= GRID_OFFSET_Y) {
                snapPos = calculateExactGridPosition(transform.x, GRID_OFFSET_Y);
                shouldSnap = true;
            }
            // Check for bubble collisions
            else if (checkBubbleCollision(transform, snapPos)) {
                shouldSnap = true;
            }

            if (shouldSnap) {
                // Ensure the snap position is valid
                if (snapPos.x >= GRID_OFFSET_X &&
                    snapPos.x <= GRID_OFFSET_X + (GRID_COLS * GRID_SIZE) - GRID_SIZE) {
                    transform.x = snapPos.x;
                    transform.y = snapPos.y;

                    projectile.isMoving = false;
                    projectile.velocity = {0, 0};

                    if (auto gridSystem = gCoordinator.getSystem<BubbleGridSystem>()) {
                        gridSystem->addBubble(entity);
                    }
                }
            }
        }
    }
};

#endif //BUBBLE_MOVEMENT_HPP
