#include "init.hpp"


// lib/core/init.cpp
void initSDL() {
    //Name the window
    std::string windowName = std::string(ENGINE_NAME) + " " + std::string(ENGINE_VERSION);

    //Some Render flags
    int rendererFlags = SDL_RENDERER_ACCELERATED;

    //Nothing of note to add to the window here
    int windowFlags = SDL_WINDOW_RESIZABLE;

    //If the SDL initialization is unsuccessful
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        std::cout << "Couldn't initialize SDL 2 : " << SDL_GetError();
        exit(1);
    }

    // Initialize TTF
    if (TTF_Init() < 0) {
        std::cout << "Couldn't initialize SDL_ttf: " << TTF_GetError();
        exit(1);
    }

    //If SDL initialized successfully, attempt to create the SDL window.
    app->window = SDL_CreateWindow(windowName.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH,
                                   SCREEN_HEIGHT, windowFlags);

    //If the window pointer is null we have an error
    if (!app->window) {
        std::cout << "Failed to open Window: " << SDL_GetError();
        exit(1);
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

    //Create the renderer for the window
    app->renderer = SDL_CreateRenderer(app->window, -1, rendererFlags);

    //If the renderer failed to initialize (returned pointer is null)
    if (!app->renderer) {
        std::cout << "Failed to create renderer: " << SDL_GetError();
        exit(1);
    }

    // Load the default font
    auto* fontHelper = FontHelper::getInstance();
    if (!fontHelper->loadFont("game_font", "assets/fonts/PressStart2P-Regular.ttf", 24)) {
        std::cout << "Failed to load font" << std::endl;
        // Don't exit here as the game can still run without font
    }
}

void cleanupSDL() {
    SDL_DestroyRenderer(app->renderer);
    SDL_DestroyWindow(app->window);
    TTF_Quit();
    SDL_Quit();
}