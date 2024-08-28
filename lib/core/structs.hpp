#pragma once
#include <SDL.h>
#include <string>


//A struct to contain the pointers to the SDL Renderer and SDL window.
typedef struct {
	SDL_Renderer* renderer;
	SDL_Window* window;
} App;
