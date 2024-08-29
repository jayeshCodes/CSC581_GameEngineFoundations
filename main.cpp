#include "main.hpp"
#include <memory>

#include "lib/core/timer.hpp"
#include "lib/core/timer.hpp"
#include "lib/core/physics/collision.hpp"
#include "lib/core/physics/gravity.hpp"
#include "lib/core/physics/keyMovement.hpp"
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

    Gravity gravity(0, 9.8);
    Collision collision;
    KeyMovement key_movement(10, 10);

    std::vector<std::unique_ptr<Rectangle> > rectangles;
    std::unique_ptr<Factory> factory;

    rectangles.push_back(factory->createRectangle({0, 255, 0, 255}, {100, 0, 100, 100}, true, 1.0f, 0.5f));
    rectangles.push_back(factory->createRectangle({255, 0, 0, 255}, {100, 250, 500, 100}, true, 100000.0f, 0.5f));


    //  init timer
    Timer fpsTimer;
    fpsTimer.start();

    Uint32 lastTime = SDL_GetTicks();
    Uint32 currentTime;
    float deltaTime;

    while (gameRunning) {
        currentTime = SDL_GetTicks();
        deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        //Prep the scene
        prepareScene();

        //Process input
        doInput();

        //modify the game world here


        // Uncomment to add keypress velocity
        // SDL_FPoint direction = getKeyPress();
        // key_movement.calculate(*rectangles[0], direction);

        gravity.calculate(*rectangles[0]);

        for (const auto &rectangle: rectangles) {
            // add physics
            collision.calculate(*rectangle, rectangles);
            rectangle->update(deltaTime);
            rectangle->draw();
        }
        // rectangle.update(deltaTime);
        // rectangle2.update(deltaTime);
        std::cout << rectangles[0]->velocity.y << std::endl;

        //Present the resulting scene
        presentScene();

        fpsTimer.start();
    }

    cleanupSDL();
    std::cout << "Closing " << ENGINE_NAME << " Engine" << std::endl;
    return 0;
}
