#pragma once
#include "structs.hpp"
#include "defs.hpp"
#include <SDL.h>
#include <iostream>

// SDL render and window context
extern App *app;

// Initialize SDL rendering window
void initSDL(int, int);

void cleanupSDL();
