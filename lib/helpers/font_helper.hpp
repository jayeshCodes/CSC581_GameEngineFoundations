//
// Created by Jayesh Gajbhar on 11/21/24.
//

#ifndef FONT_HELPER_HPP
#define FONT_HELPER_HPP
#pragma once

#include <SDL.h>
#include <SDL_ttf.h>
#include <string>
#include <memory>
#include <unordered_map>
#include "../core/structs.hpp"

// Forward declare App struct and extern app variable
extern App* app;

class FontHelper {
private:
    static FontHelper* instance;
    std::unordered_map<std::string, TTF_Font*> fonts;
    bool initialized;

    FontHelper() : initialized(false) {
        if (TTF_Init() == -1) {
            printf("TTF_Init failed: %s\n", TTF_GetError());
            return;
        }
        initialized = true;
    }

public:
    static FontHelper* getInstance() {
        if (instance == nullptr) {
            instance = new FontHelper();
        }
        return instance;
    }

    ~FontHelper() {
        for (auto& [name, font] : fonts) {
            if (font != nullptr) {
                TTF_CloseFont(font);
            }
        }
        TTF_Quit();
    }

    bool loadFont(const std::string& name, const std::string& path, int size) {
        if (!initialized) return false;

        TTF_Font* font = TTF_OpenFont(path.c_str(), size);
        if (!font) {
            printf("Failed to load font: %s\n", TTF_GetError());
            return false;
        }

        fonts[name] = font;
        return true;
    }

    void renderText(const std::string& text, const std::string& fontName, SDL_Color color, int x, int y) {
        if (!initialized || fonts.find(fontName) == fonts.end()) return;

        SDL_Surface* surface = TTF_RenderText_Blended(fonts[fontName], text.c_str(), color);
        if (!surface) {
            printf("Failed to render text: %s\n", TTF_GetError());
            return;
        }

        SDL_Texture* texture = SDL_CreateTextureFromSurface(app->renderer, surface);
        if (!texture) {
            SDL_FreeSurface(surface);
            printf("Failed to create texture: %s\n", SDL_GetError());
            return;
        }

        SDL_Rect destRect = {
            x,
            y,
            surface->w,
            surface->h
        };

        SDL_RenderCopy(app->renderer, texture, NULL, &destRect);

        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);
    }
};

#endif //FONT_HELPER_HPP