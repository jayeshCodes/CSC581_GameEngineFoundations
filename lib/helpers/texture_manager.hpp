//
// Created by Jayesh Gajbhar on 11/21/24.
//

#ifndef TEXTURE_MANAGER_HPP
#define TEXTURE_MANAGER_HPP

#include <SDL.h>
#include <SDL_image.h>
#include <string>
#include <unordered_map>
#include <memory>
#include "../generic/singleton.hpp"

class TextureManager : public Singleton<TextureManager> {
    friend class Singleton<TextureManager>;

private:
    std::unordered_map<std::string, SDL_Texture *> textureMap;
    SDL_Renderer *renderer;

    TextureManager() = default;

public:
    void init(SDL_Renderer *r) {
        renderer = r;
        // Initialize SDL_image
        int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG; // Support for PNG and JPG images
        if (!(IMG_Init(imgFlags) & imgFlags)) {
            printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        }
    }

    ~TextureManager() {
        cleanup();
    }

    SDL_Texture* loadTexture(const std::string& path) {
        auto it = textureMap.find(path);
        if (it != textureMap.end()) {
            return it->second;
        }

        // Load new texture
        SDL_Texture* texture = IMG_LoadTexture(renderer, path.c_str());
        if (texture == nullptr) {
            printf("Failed to load texture: %s\nSDL_image Error: %s\nSDL Error: %s\n",
                path.c_str(),
                IMG_GetError(),
                SDL_GetError());
            return nullptr;
        }

        // Store and return texture
        textureMap[path] = texture;
        return texture;
    }
    void cleanup() {
        for (auto &pair: textureMap) {
            if (pair.second) {
                SDL_DestroyTexture(pair.second);
            }
        }
        textureMap.clear();
        IMG_Quit();
    }
};

#endif //TEXTURE_MANAGER_HPP
