#include "main.hpp"
#include <memory>

#include "lib/core/timer.hpp"
#include "lib/core/timer.hpp"
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

    // Create Rectangle instance
    Rectangle rectangle({0,255,0,255}, {100,100,100,100});

    //  init timer
    Timer fpsTimer;
    fpsTimer.start();

    while (gameRunning) {
        //Prep the scene
        prepareScene();

        //Process input
        doInput();

        //modify the game world here

        // Draw the rectangle
        rectangle.draw();

        // Update the rectangle
        rectangle.update();

        //Present the resulting scene
        presentScene();

        // Control frame rate
        Uint32 frameTicks = fpsTimer.elapsed();
        if (frameTicks < 16) {
            SDL_Delay(16 - frameTicks);  // ~60 FPS
        }
        fpsTimer.start();
    }

    cleanupSDL();
    std::cout << "Closing " << ENGINE_NAME << " Engine" << std::endl;
    return 0;
}
