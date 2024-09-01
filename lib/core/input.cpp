#include "input.hpp"
#include "init.hpp"
#include "../game/GameManager.hpp"


/*
* This function is boilerplate, and the contents currently read the events thrown by SDL and look for the SDL quit event, terminating the program.
* This file is where code to read the keyboard state and determine which keys are pressed should sit.
*/
void doInput(void) {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                gameRunning = false;
                break;

            default:
                break;
        }
    }
}

/**
 * This function will detect key press. WASD for movement. P for changing the scaling mode
 * @return SDL_Point
 */
SDL_FPoint getKeyPress() {
    const Uint8 *state = SDL_GetKeyboardState(nullptr);
    if (state[SDL_SCANCODE_P]) {
        GameManager::getInstance()->scaleWithScreenSize = !GameManager::getInstance()->scaleWithScreenSize;
        std::cout << "Changing scale type" << std::endl;

        // Apparently this prevents debouncing effect.
        SDL_Delay(200);
        return {0, 0};
    }
    if (state[SDL_SCANCODE_A]) {
        return {-1, 0};
    }
    if (state[SDL_SCANCODE_D]) {
        return {1, 0};
    }
    if (state[SDL_SCANCODE_W]) {
        return {0, -1};
    }
    if (state[SDL_SCANCODE_S]) {
        return {0, 1};
    }
    return {0, 0};
}
