#include "main.hpp"
#include <memory>

#include "lib/core/timer.hpp"
#include "lib/core/timer.hpp"
#include "lib/core/physics/collision.hpp"
#include "lib/core/physics/gravity.hpp"
#include "lib/core/physics/keyMovement.hpp"
#include "lib/game/GameManager.hpp"
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

    // Launching basic systems of Shade Engine
    Gravity gravity(0, 9.8);
    Collision collision;
    KeyMovement key_movement(10, 10);

    //  init timer
    Timer fpsTimer;
    fpsTimer.start();

    Uint32 lastTime = SDL_GetTicks();
    float deltaTime;

    while (gameRunning) {
        Uint32 currentTime = SDL_GetTicks();
        deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        //Prep the scene
        prepareScene();

        //Process input
        doInput();

        //modify the game world here

        SDL_FPoint direction = getKeyPress();


        //Present the resulting scene
        presentScene();

        fpsTimer.start();
    }

    cleanupSDL();
    std::cout << "Closing " << ENGINE_NAME << " Engine" << std::endl;
    return 0;
}
