#include "input.hpp"
#include "init.hpp"


/*
* This function is boilerplate, and the contents currently read the events thrown by SDL and look for the SDL quit event, terminating the program.
* This file is where code to read the keyboard state and determine which keys are pressed should sit.
*/
void doInput(void)
{

    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
            gameRunning = false;
            break;

        default:
            break;
        }
    }
}