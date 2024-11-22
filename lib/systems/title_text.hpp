//
// Created by Jayesh Gajbhar on 11/22/24.
//

#ifndef TITLE_TEXT_HPP
#define TITLE_TEXT_HPP

#include <SDL.h>
#include <SDL_ttf.h>
#include <string>
#include <memory>
#include <unordered_map>
#include "../core/structs.hpp"
#include "../ECS/system.hpp"
#include "../ECS/coordinator.hpp"
#include "../helpers/font_helper.hpp"
#include "../model/components.hpp"
#include "../core/defs.hpp"

extern App *app;
extern Coordinator gCoordinator;
extern Timeline gameTimeline;


class IntroScreenSystem : public System {
private:
    bool gameStarted = false;
    const Uint8 *keyboardState = nullptr;
    FontHelper *fontHelper;

public:
    IntroScreenSystem() {
        fontHelper = FontHelper::getInstance();
        // Load the fonts we'll need
        fontHelper->loadFont("title", "assets/fonts/Dokdo-Regular.ttf", 48);
        fontHelper->loadFont("subtitle", "assets/fonts/Dokdo-Regular.ttf", 24);
    }

    void update() { // co-authored by GitHub Copilot
        keyboardState = SDL_GetKeyboardState(nullptr);

        for (auto entity: entities) {
            auto &intro = gCoordinator.getComponent<IntroScreen>(entity);

            if (!intro.isActive) continue;

            // Create semi-transparent dark overlay
            SDL_SetRenderDrawBlendMode(app->renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(app->renderer, 0, 0, 0, 128); // Semi-transparent black
            SDL_Rect overlay = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
            SDL_RenderFillRect(app->renderer, &overlay);

            // Calculate center positions
            int titleX = (SCREEN_WIDTH / 2) - 120; // Approximate width offset
            int titleY = (SCREEN_HEIGHT / 2) - 100;

            int subtitleX = (SCREEN_WIDTH / 2) - 100;
            int subtitleY = (SCREEN_HEIGHT / 2) + 50;

            // Draw title with glow effect
            SDL_Color glowColor = {255, 215, 0, 255}; // Golden glow
            fontHelper->renderText("Moodle Jump", "title", glowColor,
                                   titleX, titleY);

            // Draw subtitle with pulsing effect
            float pulse = (sin(gameTimeline.getElapsedTime() / 500.0f) + 1.0f) / 2.0f;
            Uint8 alpha = static_cast<Uint8>(128 + (127 * pulse));

            fontHelper->renderText("Press Enter to Start", "subtitle",
                                   SDL_Color{255, 255, 255, alpha},
                                   subtitleX, subtitleY);

            // Check for Enter key press
            if (keyboardState[SDL_SCANCODE_RETURN] && !gameStarted) {
                intro.isActive = false;
                gameStarted = true;
            }
        }

        // Reset blend mode
        SDL_SetRenderDrawBlendMode(app->renderer, SDL_BLENDMODE_NONE);
    }

    bool hasGameStarted() const {
        return gameStarted;
    }
};


#endif //TITLE_TEXT_HPP
