//
// Created by Utsav Lal on 11/18/24.
//

#pragma once
#include <SDL_render.h>
#include <SDL2/SDL_ttf.h>

namespace TextHelper {
    inline void get_text_and_rect(SDL_Renderer *renderer, int x, int y, const char *text,
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
}
