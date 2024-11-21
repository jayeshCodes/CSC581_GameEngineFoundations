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
    static constexpr float SHOOTER_RESET_DELAY = 0.5f; // Delay before re-enabling shooter
    static constexpr int MAX_ROWS = 24;
    static constexpr int COLS = 15;
    static constexpr float GRID_SIZE = 32.0f;
    static constexpr float GRID_OFFSET_X = (SCREEN_WIDTH - (COLS * GRID_SIZE)) / 2.0f;
    static constexpr float GRID_OFFSET_Y = 32.0f;
    static constexpr float OVERLAP_THRESHOLD = GRID_SIZE * 0.8f;

public:
    void updateBubbleGridPositions() {
        if (auto gridSystem = gCoordinator.getSystem<BubbleGridSystem>()) {
            gridSystem->resetGrid(); // Clear existing grid first

            auto bubbles = gCoordinator.getEntitiesWithComponent<Bubble>();
            for (auto bubbleEntity : bubbles) {
                auto& transform = gCoordinator.getComponent<Transform>(bubbleEntity);
                auto& bubble = gCoordinator.getComponent<Bubble>(bubbleEntity);

                // Calculate new position
                int row = static_cast<int>(round((transform.y - GRID_OFFSET_Y) / GRID_SIZE));
                int col = static_cast<int>(round((transform.x - GRID_OFFSET_X) / GRID_SIZE));
                if (row % 2 == 1) {
                    col = static_cast<int>(round((transform.x - GRID_OFFSET_X - GRID_SIZE / 2) / GRID_SIZE));
                }

                bubble.row = row;
                bubble.col = col;

                // Update grid with new position
                gridSystem->updateGridPosition(bubbleEntity, row, col);
            }

            // Check all bubbles for matches after grid is rebuilt
            for (auto bubbleEntity : bubbles) {
                auto matches = gridSystem->findMatchingBubbles(bubbleEntity);
                if (matches.size() >= 3) {
                    Event matchEvent{eventTypeToString(EventType::BubbleMatch), BubbleMatchData{matches}};
                    eventCoordinator.emit(std::make_shared<Event>(matchEvent));
                }
            }
        }
    }



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
                    // Disable shooter
                    Event disableShooterEvent{
                        eventTypeToString(EventType::DisableShooter),
                        DisableShooterData{eventTimeline.getElapsedTime()}
                    };
                    eventCoordinator.emit(std::make_shared<Event>(disableShooterEvent));
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
                    updateBubbleGridPositions();
                    // Queue reset event with delay
                    int64_t resetTime = eventTimeline.getElapsedTime() +
                                        static_cast<int64_t>(SHOOTER_RESET_DELAY * 1000);
                    Event resetShooterEvent{
                        eventTypeToString(EventType::ResetShooter),
                        ResetShooterData{resetTime}
                    };
                    eventCoordinator.queueEvent(
                        std::make_shared<Event>(resetShooterEvent),
                        resetTime,
                        Priority::HIGH
                    );

                    if (auto gridSystem = gCoordinator.getSystem<BubbleGridSystem>()) {
                        // gridSystem->handleGridDrop();
                    }

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
