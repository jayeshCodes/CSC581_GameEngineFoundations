#pragma once
#include "structs.hpp"
#include "defs.hpp"
#include <SDL.h>
#include <iostream>
#include "../helpers/font_helper.hpp"

// SDL render and window context
extern App *app;

// Initialize SDL rendering window
void initSDL();

void cleanupSDL();
