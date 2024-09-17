#pragma once
#include <SDL.h>
#include "timeline.hpp"
#include <mutex>

extern bool gameRunning;
extern std::mutex runningMutex;

void doInput();

SDL_FPoint getKeyPress();

void temporalInput(Timeline &gameTimeline);

extern bool isGameRunning();
extern void setGameRunning(bool running);