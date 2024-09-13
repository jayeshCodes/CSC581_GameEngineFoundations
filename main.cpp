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
#include "lib/animation/controller/moveBetween2Points.hpp"
#include "lib/core/timeline.hpp"

bool gameRunning = false;


int main(int argc, char *argv[]) {
    //Call the initialization functions
    std::cout << ENGINE_NAME << " v" << ENGINE_VERSION << " initializing" << std::endl;
    std::cout << "Created by Utsav and Jayesh" << std::endl;
    std::cout << std::endl;
    initSDL();
    gameRunning = true;

    // creating global game clock
    Timeline anchorTimeline(nullptr, 1); // normal tic value of 1
    anchorTimeline.start();

    // creating a timeline linked to the anchor
    Timeline gameTimeline(&anchorTimeline, 1); // normal tic value of 1
    gameTimeline.start();

    // Create Rectangle instance
    std::vector<std::unique_ptr<Rectangle> > rectangles;
    rectangles.push_back(Factory::createRectangle({0, 255, 255, 255}, {100, 100, 100, 100}, true, 0.1, 0.8));
    rectangles.push_back(Factory::createRectangle({255, 255, 0, 255}, {300, 100, 100, 100}, true, 0.001, 0.8));
    rectangles.push_back(Factory::createRectangle({255, 0, 0, 255}, {300, SCREEN_HEIGHT / 2.f, SCREEN_WIDTH / 2.f, 100},
                                                  true, 100000.f, 0.8));
    rectangles.push_back(Factory::createRectangle({255, 0, 255, 255}, {1000, 100, 100, 100}, false, 1.f, 0.8));

    MoveBetween2Points m(100.f, 400.f, LEFT, 2);

    // Launching basic systems of Shade Engine
    Gravity gravity(0, 1.0);
    Collision collision;
    KeyMovement key_movement(300, 300);


    // Uint32 lastTime = SDL_GetTicks();
    int64_t lastTime = gameTimeline.getTime();
    float deltaTime = 0;

    while (gameRunning) {
        // Uint32 currentTime = SDL_GetTicks();
        // deltaTime = (currentTime - lastTime) / 1000.0f;
        // lastTime = currentTime;

        // timeline implementation
        int64_t currentTime = gameTimeline.getTime(); // init current time in ticks
        deltaTime = (currentTime - lastTime) / 1000.0f; // convert into seconds for smooth movement animations
        lastTime = currentTime;

        std::cout << "current time: " << currentTime << std::endl;
        std::cout << "last time: " << lastTime << std::endl;
        std::cout << "delta time: " << deltaTime << std::endl;

        //Prep the scene
        prepareScene();

        //Process input
        doInput();

        //modify the game world here

        SDL_FPoint direction = getKeyPress();
        key_movement.calculate(*rectangles[0], direction);

        // gravity.calculate(*rectangles[0]);
        gravity.calculate(*rectangles[1]);

        collision.calculate(*rectangles[1], rectangles);
        collision.calculate(*rectangles[0], rectangles);

        m.moveBetween2Points(*rectangles[2]);

        for (const auto &rectangle: rectangles) {
            rectangle->update(deltaTime);
            rectangle->draw();
        }


        //Present the resulting scene
        presentScene();
    }

    cleanupSDL();
    std::cout << "Closing " << ENGINE_NAME << " Engine" << std::endl;
    return 0;
}
