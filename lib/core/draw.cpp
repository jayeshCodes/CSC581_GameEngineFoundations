#include "draw.hpp"

void prepareScene(void)
{
    //Sets the background to blue
    SDL_SetRenderDrawColor(app->renderer, 255, 255, 255, 255);
    //Clears the renderer
    SDL_RenderClear(app->renderer);
}

void presentScene(void)
{
    //Render the scene 
    SDL_RenderPresent(app->renderer);
}