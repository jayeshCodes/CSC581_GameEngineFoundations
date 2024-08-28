#include "main.hpp"
#include <memory>

#include "lib/core/timer.hpp"
#include "lib/core/timer.hpp"
#include "lib/core/physics/gravity.hpp"
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
    Gravity gravity(0, 9.8);


    //  init timer
    Timer fpsTimer;
    fpsTimer.start();

    Uint32 lastTime = SDL_GetTicks();
    Uint32 currentTime;
    float deltaTime;

    while (gameRunning) {
        // Control frame rate
        // Uint32 frameTicks = fpsTimer.elapsed();
        // if (frameTicks < 16) {
        //     SDL_Delay(16 - frameTicks);  // ~60 FPS
        // }

        currentTime = SDL_GetTicks();
        deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        //Prep the scene
        prepareScene();

        //Process input
        doInput();

        //modify the game world here

        // Draw the rectangle
        rectangle.draw();

        // Update the rectangle
        gravity.calculate(rectangle);
        rectangle.update(deltaTime);

        std::cout << deltaTime << std::endl;

        //Present the resulting scene
        presentScene();

        fpsTimer.start();
    }

    cleanupSDL();
    std::cout << "Closing " << ENGINE_NAME << " Engine" << std::endl;
    return 0;
}
