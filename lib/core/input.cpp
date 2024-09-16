#include "input.hpp"
#include "init.hpp"
#include "../game/GameManager.hpp"
#include <thread>


extern Timeline anchorTimeline;


/*
* This function is boilerplate, and the contents currently read the events thrown by SDL and look for the SDL quit event, terminating the program.
* This file is where code to read the keyboard state and determine which keys are pressed should sit.
*/
void doInput() {
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
 * This function will detect key press. WASD for movement. O for changing the scaling mode
 * @return SDL_Point
 */
SDL_FPoint getKeyPress() {
    const Uint8 *state = SDL_GetKeyboardState(nullptr);

    if (state[SDL_SCANCODE_O]) {
        GameManager::getInstance()->scaleWithScreenSize = !GameManager::getInstance()->scaleWithScreenSize;
        std::cout << "Changing scale type" << std::endl;
        SDL_Delay(200);
        return {0, 0};
    }


    // Only process movement if the game is not paused
    if (state[SDL_SCANCODE_A]) return {-1, 0};
    if (state[SDL_SCANCODE_D]) return {1, 0};
    if (state[SDL_SCANCODE_W]) return {0, -1};
    if (state[SDL_SCANCODE_S]) return {0, 1};

    return {0, 0};
}


// TODO : implement temporalInput properly, game crashes in the current implementation
void temporalInput(Timeline &gameTimeline) {
    const Uint8 *state = SDL_GetKeyboardState(nullptr);
    if (state[SDL_SCANCODE_ESCAPE]) {
        if (gameTimeline.isPaused()) {
            gameTimeline.start();
            std::cout << "Game resumed" << std::endl;
        } else {
            gameTimeline.pause();
            std::cout << "Game paused" << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(200)));
    }

    if (state[SDL_SCANCODE_1]) {
        gameTimeline.changeTic(1.0f);
    }
    if (state[SDL_SCANCODE_2]) {
        gameTimeline.changeTic(2.0f);
    }
    if (state[SDL_SCANCODE_3]) {
        gameTimeline.changeTic(0.5f);
    }
}
