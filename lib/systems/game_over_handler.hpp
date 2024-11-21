//
// Created by Jayesh Gajbhar on 11/22/24.
//

#ifndef GAME_OVER_HANDLER_HPP
#define GAME_OVER_HANDLER_HPP

#include "bubble_grid.hpp"
#include "grid_generator.hpp"
#include "../ECS/system.hpp"
#include "../model/components.hpp"
#include "../ECS/coordinator.hpp"
#include "../EMS/event_coordinator.hpp"
#include "../model/event.hpp"
#include "../helpers/font_helper.hpp"
#include "../core/defs.hpp"

extern Coordinator gCoordinator;
extern EventCoordinator eventCoordinator;
extern Timeline gameTimeline;

class GameOverHandlerSystem : public System {
private:
    bool isGameOver = false;
    static constexpr float MESSAGE_X = SCREEN_WIDTH * 3 / 4.0f - 70.0f;
    static constexpr float MESSAGE_Y = SCREEN_HEIGHT / 2.0f;
    SDL_Color textColor = {255, 0, 0, 255}; // Red color for game over

    EventHandler gameOverHandler = [this](const std::shared_ptr<Event>& event) {
        if (event->type == eventTypeToString(EventType::GameOver)) {
            isGameOver = true;
            gameTimeline.pause();

            // Disable shooter
            auto shooters = gCoordinator.getEntitiesWithComponent<BubbleShooter>();
            for (auto shooter : shooters) {
                auto& shooterComp = gCoordinator.getComponent<BubbleShooter>(shooter);
                shooterComp.isDisabled = true;
            }
        }
    };

    void resetGame() {
        isGameOver = false;
        gameTimeline.start();

        // Clear all existing bubbles
        auto bubbles = gCoordinator.getEntitiesWithComponent<Bubble>();
        for (auto bubble : bubbles) {
            if (!gCoordinator.hasComponent<Destroy>(bubble)) {
                gCoordinator.addComponent(bubble, Destroy{});
            }
            gCoordinator.getComponent<Destroy>(bubble).destroy = true;
        }

        // Reset grid system
        if (auto gridSystem = gCoordinator.getSystem<BubbleGridSystem>()) {
            gridSystem->reset();
        }

        // Reset score
        auto scoreEntities = gCoordinator.getEntitiesWithComponent<Score>();
        if (!scoreEntities.empty()) {
            auto& score = gCoordinator.getComponent<Score>(scoreEntities[0]);
            score.value = 0;
            score.multiplier = 1;
        }

        // Re-initialize grid
        auto generators = gCoordinator.getEntitiesWithComponent<GridGenerator>();
        if (!generators.empty() && gCoordinator.getSystem<BubbleGridGeneratorSystem>()) {
            gCoordinator.getSystem<BubbleGridGeneratorSystem>()->initializeGrid(generators[0]);
        }

        // Reset and enable shooter
        auto shooters = gCoordinator.getEntitiesWithComponent<BubbleShooter>();
        for (auto shooter : shooters) {
            auto& shooterComp = gCoordinator.getComponent<BubbleShooter>(shooter);
            shooterComp.isDisabled = false;
            shooterComp.canShoot = true;
            shooterComp.currentAngle = 270.0f; // Reset to straight up
        }

        // Emit reset event
        Event resetEvent{eventTypeToString(EventType::Reset), {}};
        eventCoordinator.emit(std::make_shared<Event>(resetEvent));
    }

    void handleInput() {
        const Uint8* keyState = SDL_GetKeyboardState(nullptr);
        if (keyState[SDL_SCANCODE_RETURN] && isGameOver) {
            resetGame();
        }
    }

public:
    GameOverHandlerSystem() {
        eventCoordinator.subscribe(gameOverHandler, eventTypeToString(EventType::GameOver));
    }

    ~GameOverHandlerSystem() {
        eventCoordinator.unsubscribe(gameOverHandler, eventTypeToString(EventType::GameOver));
    }

    void update() {
        if (isGameOver) {
            handleInput();

            // Render game over message
            auto* fontHelper = FontHelper::getInstance();
            if (fontHelper) {
                fontHelper->renderText(
                    "Game Over!",
                    "game_font",
                    textColor,
                    MESSAGE_X,
                    MESSAGE_Y
                );

                fontHelper->renderText(
                    "Press Enter to",
                    "game_font",
                    textColor,
                    MESSAGE_X,
                    MESSAGE_Y + 40
                );
                fontHelper->renderText(
                    "Restart",
                    "game_font",
                    textColor,
                    MESSAGE_X,
                    MESSAGE_Y + 40 + 40
                );
            }
        }
    }
};

#endif //GAME_OVER_HANDLER_HPP