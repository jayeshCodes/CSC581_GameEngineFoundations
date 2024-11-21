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
    static constexpr float SNAP_THRESHOLD = GRID_SIZE * 0.8f;
    static constexpr float BOUNDARY_WIDTH = 10.0f;
    static constexpr float LEFT_BOUNDARY = GRID_OFFSET_X - BOUNDARY_WIDTH - 17.f;
    static constexpr float RIGHT_BOUNDARY = GRID_OFFSET_X + (GRID_COLS * GRID_SIZE) - 10.f;
    static constexpr float COLLISION_THRESHOLD = 32.0f;


    bool checkBoundaryCollision(Transform &transform, float &velocityX) {
        if (transform.x <= LEFT_BOUNDARY + BOUNDARY_WIDTH) {
            transform.x = LEFT_BOUNDARY + BOUNDARY_WIDTH;
            velocityX = std::abs(velocityX); // Bounce right
            return true;
        }
        if (transform.x + transform.w >= RIGHT_BOUNDARY) {
            transform.x = RIGHT_BOUNDARY - transform.w;
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
                std::cout << "Collision detected! Distance: " << distance << std::endl;
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
            transform.x += projectile.velocity.x * dt;
            transform.y += projectile.velocity.y * dt;

            checkBoundaryCollision(transform, projectile.velocity.x);

            SDL_FPoint snapPos;
            bool shouldSnap = false;

            if (transform.y <= GRID_OFFSET_Y) {
                snapPos = calculateExactGridPosition(transform.x, GRID_OFFSET_Y);
                shouldSnap = true;
            } else if (checkBubbleCollision(transform, snapPos)) {
                shouldSnap = true;
            }

            if (shouldSnap) {
                if (snapPos.x >= LEFT_BOUNDARY + BOUNDARY_WIDTH &&
                    snapPos.x <= RIGHT_BOUNDARY - transform.w) {
                    transform.x = snapPos.x;
                    transform.y = snapPos.y;
                    projectile.isMoving = false;
                    projectile.velocity = {0, 0};

                    // Add Bubble component before adding to grid
                    if (!gCoordinator.hasComponent<Bubble>(entity)) {
                        Bubble bubble;
                        bubble.isActive = true;
                        bubble.radius = transform.w / 2;
                        gCoordinator.addComponent(entity, bubble);
                    }

                    if (auto gridSystem = gCoordinator.getSystem<BubbleGridSystem>()) {
                        gridSystem->addBubble(entity);
                    }
                }
            }
        }
    }
};

#endif //BUBBLE_MOVEMENT_HPP
