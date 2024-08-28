#include "main.hpp"
#include <memory>

#include "lib/objects/factory.hpp"
#include "lib/objects/shapes/rectangle.hpp"

bool gameRunning = false;


int main(int argc, char *argv[]) {
    //Call the initialization functions
    std::cout << ENGINE_NAME << " v" << ENGINE_VERSION << " initializing" << std::endl;
    std::cout << "Created by Utsav and Jayesh" << std::endl;
    std::cout << std::endl;
    initSDL();
    gameRunning = true;


    while (gameRunning) {
        //Prep the scene
        prepareScene();

        //Process input
        doInput();

        //modify the game world here


        //Present the resulting scene
        presentScene();

        //Inserting 16ms delay at the end for a budget frame-limiter.
        SDL_Delay(16);
    }

    cleanupSDL();
    std::cout << "Closing " << ENGINE_NAME << " Engine" << std::endl;
    return 0;
}
