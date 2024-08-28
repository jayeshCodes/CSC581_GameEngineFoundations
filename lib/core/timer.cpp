//
// Created by Jayesh Gajbhar on 8/28/24.
//

#include "timer.hpp"
#include <SDL.h>


void Timer::start() {
    startTicks = SDL_GetTicks();
    started = true;
    paused = false;
    pausedTicks = 0;
}

void Timer::stop() {
    startTicks = 0;
    pausedTicks = 0;
    paused = false;
    started = false;
}

void Timer::pause() {
    if (started && !paused) {
        paused = true;
        pausedTicks = SDL_GetTicks();
        startTicks = 0;
    }
}

void Timer::unpause() {
    if (started && paused) {
        paused = false;
        startTicks = SDL_GetTicks();
        pausedTicks = 0;
    }
}

Uint32 Timer::elapsed() {
    if (started) {
        if (paused) {
            return pausedTicks;
        } else {
            return SDL_GetTicks() - startTicks;
        }
    }
    return 0;
}

bool Timer::isStarted() const {
    return started;
}

bool Timer::isPaused() const {
    return paused;
}
