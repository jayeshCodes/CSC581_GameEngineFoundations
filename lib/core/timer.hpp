//
// Created by Jayesh Gajbhar on 8/28/24.
//

#ifndef TIMER_H
#define TIMER_H
#include <SDL_stdinc.h>

class Timer {
public:
    Timer() : startTicks(0), pausedTicks(0), paused(false), started(false) {
    }

    void start();

    void stop();

    void pause();

    void unpause();

    Uint32 elapsed();

    bool isStarted() const;

    bool isPaused() const;

private:
    Uint32 startTicks;
    Uint32 pausedTicks;
    bool paused;
    bool started;
};

#endif //TIMER_H
