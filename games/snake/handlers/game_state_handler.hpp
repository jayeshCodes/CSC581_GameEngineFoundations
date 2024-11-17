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
#include "../helpers/quantizer.hpp"
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

    TTF_Font *font = nullptr;

    static void get_text_and_rect(SDL_Renderer *renderer, int x, int y, const char *text,
                                  TTF_Font *font, SDL_Texture **texture, SDL_Rect *rect, SDL_Color color) {
        SDL_Surface *surface = TTF_RenderText_Solid(font, text, color);
        *texture = SDL_CreateTextureFromSurface(renderer, surface);
        int text_width = surface->w;
        int text_height = surface->h;
        SDL_FreeSurface(surface);
        rect->x = x;
        rect->y = y;
        rect->w = text_width;
        rect->h = text_height;
    }

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

            get_text_and_rect(app->renderer, screen_width / 2 - 100, screen_height / 2 - 100, "Game Over", font,
                              &gameOverTextTexture, &gameOverTextRect, shade_color::White);
            get_text_and_rect(app->renderer, screen_width / 2 - 140, screen_height / 2 - 50, "Press R to restart", font,
                              &line1Texture, &line1Rect, shade_color::White);


            Entity gameOverText = gCoordinator.createEntity();
            gCoordinator.addComponent(gameOverText, Transform{});
            gCoordinator.addComponent(gameOverText, Text{"Game Over", gameOverTextTexture, gameOverTextRect});

            Entity line1 = gCoordinator.createEntity();
            gCoordinator.addComponent(line1, Transform{});
            gCoordinator.addComponent(line1, Text{"Press R to restart", line1Texture, line1Rect});
        }
    };

public:
    GameStateHandler() {
        font = TTF_OpenFont("../games/snake/fonts/NotoSans-Black.ttf", 30);
        eventCoordinator.subscribe(gameEndHandler, GameEvents::eventTypeToString(GameEvents::GameEnd));
    }

    ~GameStateHandler() {
        eventCoordinator.unsubscribe(gameEndHandler, GameEvents::eventTypeToString(GameEvents::GameEnd));
        TTF_CloseFont(font);
        SDL_DestroyTexture(gameOverTextTexture);
        SDL_DestroyTexture(line1Texture);
        TTF_Quit();
    }
};
