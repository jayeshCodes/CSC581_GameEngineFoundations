//
// Created by Jayesh Gajbhar on 11/17/24.
//

#ifndef BUBBLE_GRID_MOVEMENT_HPP
#define BUBBLE_GRID_MOVEMENT_HPP

#include "../ECS/system.hpp"
#include "../model/components.hpp"
#include "../ECS/coordinator.hpp"
#include "../EMS/event_coordinator.hpp"
#include "../model/event.hpp"
#include "../core/defs.hpp"

extern Coordinator gCoordinator;
extern EventCoordinator eventCoordinator;

class BubbleGridMovementSystem : public System {
    static constexpr float WARNING_FLASH_INTERVAL = 0.5f;

public:
    void update(float dt) {
        for (auto entity: entities) {
            auto &movement = gCoordinator.getComponent<GridMovement>(entity);

            if (!movement.isDropping) {
                movement.currentTime += dt;

                // Handle warning phase
                if (movement.dropInterval - movement.currentTime <= movement.warningTime) {
                    movement.lastWarningToggle += dt;
                    if (movement.lastWarningToggle >= WARNING_FLASH_INTERVAL) {
                        movement.showWarning = !movement.showWarning;
                        movement.lastWarningToggle = 0;
                    }
                }

                // Check if it's time to drop
                if (movement.currentTime >= movement.dropInterval) {
                    movement.isDropping = true;
                    movement.currentDropAmount = 0.0f;
                    movement.currentTime = 0.0f;
                }
            } else {
                // Handle dropping
                float dropThisFrame = movement.dropSpeed * dt;
                movement.currentDropAmount += dropThisFrame;

                // Move all bubbles down
                auto bubbles = gCoordinator.getEntitiesWithComponent<Bubble>();
                for (auto bubbleEntity: bubbles) {
                    auto &transform = gCoordinator.getComponent<Transform>(bubbleEntity);
                    transform.y += dropThisFrame;
                }

                // Check if drop is complete
                if (movement.currentDropAmount >= movement.dropDistance) {
                    movement.isDropping = false;
                    movement.showWarning = false;

                    // Generate new row at the top
                    auto generators = gCoordinator.getEntitiesWithComponent<GridGenerator>();
                    if (!generators.empty()) {
                        auto &generator = gCoordinator.getComponent<GridGenerator>(generators[0]);
                        generator.needsNewRow = true;
                    }

                    // Check for game over
                    auto bubbles = gCoordinator.getEntitiesWithComponent<Bubble>();
                    for (auto bubble: bubbles) {
                        auto &transform = gCoordinator.getComponent<Transform>(bubble);
                        if (transform.y + transform.h >= SCREEN_HEIGHT - 100) {
                            Event gameOverEvent{eventTypeToString(EventType::GameOver), {}};
                            eventCoordinator.emit(std::make_shared<Event>(gameOverEvent));
                            return;
                        }
                    }
                }
            }
        }
    }
};

#endif //BUBBLE_GRID_MOVEMENT_HPP
