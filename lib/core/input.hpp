#pragma once
#include <SDL.h>
#include "timeline.hpp"

extern bool gameRunning;

void doInput();

SDL_FPoint getKeyPress();

void temporalInput(Timeline &gameTimeline);