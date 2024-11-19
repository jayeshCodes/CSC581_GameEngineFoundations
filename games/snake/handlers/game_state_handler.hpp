//
// Created by Utsav Lal on 11/17/24.
//

#pragma once

// #include <SDL2/SDL_ttf.h>

#include "../../../main.hpp"
#include "../../../lib/ECS/coordinator.hpp"
#include "../../../lib/ECS/system.hpp"
#include "../../../lib/EMS/event_coordinator.hpp"
#include "../../../lib/helpers/colors.hpp"
#include "../helpers/text_helper.hpp"
#include "../model/event.hpp"
#include "../model/components.hpp"

extern Coordinator gCoordinator;
extern EventCoordinator eventCoordinator;
extern int screen_width;
extern int screen_height;

class GameStateHandler : public System {
    bool gameOver = false;
    SDL_Texture *gameOverTextTexture = nullptr;
    SDL_Rect gameOverTextRect{};
    SDL_Texture *line1Texture = nullptr;
    SDL_Rect line1Rect{};
    SDL_Texture *scoreTexture = nullptr;
    SDL_Rect scoreRect{};

    Entity scoreText = INVALID_ENTITY;
    Entity gameOverText = INVALID_ENTITY;
    Entity line1 = INVALID_ENTITY;

    TTF_Font *font = nullptr;
    int score = 0;

    EventHandler gameStartHandler = [this](const std::shared_ptr<Event> (&event)) {
        if (event->type == GameEvents::eventTypeToString(GameEvents::GameStart)) {
            score = 0;
            TextHelper::get_text_and_rect(app->renderer, 10, 10, "0", font, &scoreTexture, &scoreRect,
                                          shade_color::Black);
            scoreText = gCoordinator.createEntity();
            gCoordinator.addComponent(scoreText, Transform{});
            gCoordinator.addComponent(scoreText, Text{"0", scoreTexture, scoreRect});
        }
    };

    EventHandler foodEatenHandler = [this](const std::shared_ptr<Event> (&event)) {
        if (event->type == GameEvents::eventTypeToString(GameEvents::FoodEaten)) {
            score += 1;
            TextHelper::get_text_and_rect(app->renderer, 10, 10, std::to_string(score).c_str(), font, &scoreTexture,
                                          &scoreRect, shade_color::Black);
            gCoordinator.removeComponent<Text>(scoreText);
            gCoordinator.addComponent(scoreText, Text{std::to_string(score), scoreTexture, scoreRect});
        }
    };

    EventHandler gameEndHandler = [this](const std::shared_ptr<Event> (&event)) {
        if (event->type == GameEvents::eventTypeToString(GameEvents::GameEnd)) {
            if (gameOver) return;
            gameOver = true;
            Entity endScreen = gCoordinator.createEntity();
            gCoordinator.addComponent(endScreen, Transform{
                                          0, 0, static_cast<float>(screen_width), static_cast<float>(screen_height), 0,
                                          1
                                      });
            gCoordinator.addComponent(endScreen, Color{shade_color::Black});

            TextHelper::get_text_and_rect(app->renderer, screen_width / 2 - 100, screen_height / 2 - 100, "Game Over",
                                          font,
                                          &gameOverTextTexture, &gameOverTextRect, shade_color::White);
            std::string sText = "Score: " + std::to_string(score);
            TextHelper::get_text_and_rect(app->renderer, screen_width / 2 - 90, screen_height / 2 - 50,
                                          sText.c_str(), font,
                                          &line1Texture, &line1Rect, shade_color::White);

            gCoordinator.addComponent(scoreText, Destroy{0, true, true});

            gameOverText = gCoordinator.createEntity();
            gCoordinator.addComponent(gameOverText, Transform{});
            gCoordinator.addComponent(gameOverText, Text{"Game Over", gameOverTextTexture, gameOverTextRect});

            line1 = gCoordinator.createEntity();
            gCoordinator.addComponent(line1, Transform{});
            gCoordinator.addComponent(line1, Text{"Press Enter/Return to restart", line1Texture, line1Rect});
        }
    };

public:
    GameStateHandler() {
        font = TTF_OpenFont("../games/snake/fonts/NotoSans-Black.ttf", 30);
        eventCoordinator.subscribe(gameEndHandler, GameEvents::eventTypeToString(GameEvents::GameEnd));
        eventCoordinator.subscribe(gameStartHandler, GameEvents::eventTypeToString(GameEvents::GameStart));
        eventCoordinator.subscribe(foodEatenHandler, GameEvents::eventTypeToString(GameEvents::FoodEaten));
    }

    ~GameStateHandler() {
        eventCoordinator.unsubscribe(gameEndHandler, GameEvents::eventTypeToString(GameEvents::GameEnd));
        eventCoordinator.unsubscribe(gameStartHandler, GameEvents::eventTypeToString(GameEvents::GameStart));
        eventCoordinator.unsubscribe(foodEatenHandler, GameEvents::eventTypeToString(GameEvents::FoodEaten));

        TTF_CloseFont(font);
        SDL_DestroyTexture(gameOverTextTexture);
        SDL_DestroyTexture(line1Texture);
        TTF_Quit();
    }
};
