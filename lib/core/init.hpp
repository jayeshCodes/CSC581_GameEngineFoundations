#pragma once
#include "structs.hpp"
#include "defs.hpp"
#include <SDL2/SDL.h>
#include <iostream>

// SDL render and window context
extern App* app;

// Initialize SDL rendering window
void initSDL();