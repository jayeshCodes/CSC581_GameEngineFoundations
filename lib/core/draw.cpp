#include "draw.hpp"

void prepareScene(SDL_Color backgroundColor)
{
    //Sets the background to blue
    SDL_SetRenderDrawColor(app->renderer, backgroundColor.r, backgroundColor.g, backgroundColor.g, backgroundColor.a);
    //Clears the renderer
    SDL_RenderClear(app->renderer);
}

void presentScene(void)
{
    //Render the scene 
    SDL_RenderPresent(app->renderer);
}