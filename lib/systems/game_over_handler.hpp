//
// Created by Jayesh Gajbhar on 11/23/24.
//

#ifndef GAME_OVER_HANDLER_HPP
#define GAME_OVER_HANDLER_HPP

#include "../ECS/coordinator.hpp"
#include "../ECS/system.hpp"
#include "../helpers/font_helper.hpp"
#include "../model/components.hpp"
#include "../model/event.hpp"
#include "../EMS/event_coordinator.hpp"
#include "../core/defs.hpp"

extern Coordinator gCoordinator;
extern EventCoordinator eventCoordinator;

struct GameOver {
    bool isGameOver = false;
    float alpha = 0;  // For fade effect
};

class GameOverHandlerSystem : public System {
private:
    FontHelper* fontHelper;
    float fadeAlpha = 0;
    const float FADE_SPEED = 255.0f;  // Fade speed in alpha per second

    void handleGameOver(Entity player) {
        if (!isGameOver) {
            isGameOver = true;

            // Stop player movement
            if (gCoordinator.hasComponent<CKinematic>(player)) {
                auto& kinematic = gCoordinator.getComponent<CKinematic>(player);
                kinematic.velocity = {0, 0};
                kinematic.acceleration = {0, 0};
            }

            // Disable player input
            if (gCoordinator.hasComponent<KeyboardMovement>(player)) {
                auto& movement = gCoordinator.getComponent<KeyboardMovement>(player);
                movement.speed = 0;
            }
        }
    }

    EventHandler gameOverHandler = [this](const std::shared_ptr<Event>& event) {
        if (event->type == eventTypeToString(EventType::GameOver)) {
            const GameOverData& data = event->data;
            handleGameOver(data.player);
        }
    };

public:
    bool isGameOver = false;
    GameOverHandlerSystem() {
        fontHelper = FontHelper::getInstance();
        fontHelper->loadFont("gameover", "assets/fonts/Dokdo-Regular.ttf", 48);
        eventCoordinator.subscribe(gameOverHandler, eventTypeToString(EventType::GameOver));
    }

    ~GameOverHandlerSystem() {
        eventCoordinator.unsubscribe(gameOverHandler, eventTypeToString(EventType::GameOver));
    }

    void update(float dt) {
        if (!isGameOver) return;

        // Update fade effect
        fadeAlpha = std::min(fadeAlpha + (FADE_SPEED * dt), 255.0f);

        // Draw semi-transparent black overlay
        SDL_SetRenderDrawBlendMode(app->renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(app->renderer, 0, 0, 0, static_cast<Uint8>(fadeAlpha * 0.5f));
        SDL_Rect overlay = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
        SDL_RenderFillRect(app->renderer, &overlay);

        // Draw game over text
        if (fadeAlpha > 50) {  // Start showing text after slight fade
            std::string text = "Game Over";
            int textX = (SCREEN_WIDTH / 2) - 100;  // Approximate center
            int textY = (SCREEN_HEIGHT / 2) - 24;  // Approximate center

            // Draw game over text with current alpha
            SDL_Color textColor = {255, 0, 0, static_cast<Uint8>(fadeAlpha)};
            fontHelper->renderText(text, "gameover", textColor, textX, textY);

            // Draw restart instruction
            std::string restartText = "Press R to Restart";
            SDL_Color restartColor = {255, 255, 255, static_cast<Uint8>(fadeAlpha)};
            fontHelper->renderText(restartText, "subtitle", restartColor,
                                 textX - 20, textY + 60);

            // Check for restart input
            const Uint8* keystate = SDL_GetKeyboardState(nullptr);
            if (keystate[SDL_SCANCODE_R]) {
                restartGame();
            }
        }
    }

    void restartGame() {
        // Reset game state
        isGameOver = false;
        fadeAlpha = 0;

        // Emit restart event
        Event restartEvent{eventTypeToString(EventType::GameRestart), {}};
        eventCoordinator.emit(std::make_shared<Event>(restartEvent));
    }

    bool getIsGameOver() const {
        return isGameOver;
    }
};

#endif
