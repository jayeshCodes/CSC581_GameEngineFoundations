#pragma once
#include "structs.hpp"
#include <SDL.h>

// SDL render and window context
extern App* app;

// Prepare scene to be rendered to window
void prepareScene(SDL_Color backgroundColor);
// Render scene to window
void presentScene(void);