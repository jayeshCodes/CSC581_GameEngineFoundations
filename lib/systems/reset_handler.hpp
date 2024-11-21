//
// Created by Jayesh Gajbhar on 11/22/24.
//

#ifndef RESET_HANDLER_HPP
#define RESET_HANDLER_HPP

#include "bubble_grid.hpp"
#include "grid_generator.hpp"
#include "../ECS/system.hpp"
#include "../model/components.hpp"
#include "../ECS/coordinator.hpp"
#include "../EMS/event_coordinator.hpp"
#include "../model/event.hpp"

extern Coordinator gCoordinator;
extern EventCoordinator eventCoordinator;
extern Timeline gameTimeline;

class ResetHandlerSystem : public System {
private:
    void resetGridMovement() {
        auto entities = gCoordinator.getEntitiesWithComponent<GridMovement>();
        for (auto entity : entities) {
            auto& movement = gCoordinator.getComponent<GridMovement>(entity);
            movement.currentTime = 0.0f;
            movement.isDropping = false;
            movement.showWarning = false;
            movement.currentDropAmount = 0.0f;
            movement.lastWarningToggle = 0.0f;
        }
    }

    void resetBubbleShooter() {
        auto entities = gCoordinator.getEntitiesWithComponent<BubbleShooter>();
        for (auto entity : entities) {
            auto& shooter = gCoordinator.getComponent<BubbleShooter>(entity);
            shooter.isDisabled = false;
            shooter.canShoot = true;
            shooter.isShooting = false;
            shooter.currentAngle = 270.0f; // Point straight up
            shooter.currentReloadTime = 0.0f;
            shooter.lastShootTime = 0;
            shooter.lastDisableTime = 0;
        }
    }

    void resetScore() {
        auto entities = gCoordinator.getEntitiesWithComponent<Score>();
        for (auto entity : entities) {
            auto& score = gCoordinator.getComponent<Score>(entity);
            score.value = 0;
            score.multiplier = 1;
        }
    }

    void clearBubbles() {
        // Mark all existing bubbles for destruction
        auto bubbles = gCoordinator.getEntitiesWithComponent<Bubble>();
        for (auto bubble : bubbles) {
            if (!gCoordinator.hasComponent<Destroy>(bubble)) {
                gCoordinator.addComponent(bubble, Destroy{});
            }
            gCoordinator.getComponent<Destroy>(bubble).destroy = true;
        }

        // Also clear any active projectiles
        auto projectiles = gCoordinator.getEntitiesWithComponent<BubbleProjectile>();
        for (auto projectile : projectiles) {
            if (!gCoordinator.hasComponent<Destroy>(projectile)) {
                gCoordinator.addComponent(projectile, Destroy{});
            }
            gCoordinator.getComponent<Destroy>(projectile).destroy = true;
        }
    }

    void initializeNewGrid() {
        // Reset the grid system
        if (auto gridSystem = gCoordinator.getSystem<BubbleGridSystem>()) {
            gridSystem->reset();
        }

        // Initialize new grid
        auto generators = gCoordinator.getEntitiesWithComponent<GridGenerator>();
        if (!generators.empty()) {
            auto& generator = gCoordinator.getComponent<GridGenerator>(generators[0]);
            generator.needsNewRow = false; // Reset new row flag

            if (auto gridGeneratorSystem = gCoordinator.getSystem<BubbleGridGeneratorSystem>()) {
                gridGeneratorSystem->initializeGrid(generators[0]);
            }
        }
    }

    EventHandler resetHandler = [this](const std::shared_ptr<Event>& event) {
        if (event->type == eventTypeToString(EventType::Reset)) {
            std::cout << "Processing reset event" << std::endl;

            // Clear all existing bubbles and projectiles
            clearBubbles();

            // Reset all systems
            resetGridMovement();
            resetBubbleShooter();
            resetScore();

            // Initialize new grid
            initializeNewGrid();

            // Ensure game timeline is running
            if (gameTimeline.isPaused()) {
                gameTimeline.start();
            }

            // Optional: Play reset sound or animation
            // Event soundEvent{eventTypeToString(EventType::PlaySound), SoundData{"reset"}};
            // eventCoordinator.emit(std::make_shared<Event>(soundEvent));
        }
    };

public:
    ResetHandlerSystem() {
        eventCoordinator.subscribe(resetHandler, eventTypeToString(EventType::Reset));
    }

    ~ResetHandlerSystem() {
        eventCoordinator.unsubscribe(resetHandler, eventTypeToString(EventType::Reset));
    }
};

#endif //RESET_HANDLER_HPP