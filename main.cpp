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
const float TARGET_FRAME_TIME = 1000.f / 60.f; // 60 fps

// Since no anchor this will be global time. The TimeLine class caluclate microseconds and hence tic_interval of 1000 ensures this class calculates in milliseconds
Timeline anchorTimeline(nullptr, 1000); // normal tic value of 1

int main(int argc, char *argv[]) {
    //Call the initialization functions
    std::cout << ENGINE_NAME << " v" << ENGINE_VERSION << " initializing" << std::endl;
    std::cout << "Created by Utsav and Jayesh" << std::endl;
    std::cout << std::endl;
    initSDL();
    gameRunning = true;

    // creating global game clock
    anchorTimeline.start();

    // creating a timeline linked to the anchor, with a tic value of 1 (1 millisecond)
    // If we reduce it to 0.1, the game will run 10 times faster
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
    int64_t lastTime = gameTimeline.getElapsedTime();
    float deltaTime = 0;

    while (gameRunning) {
        // timeline implementation
        int64_t currentTime = gameTimeline.getElapsedTime(); // init current time in ticks
        deltaTime = (currentTime - lastTime) / 1000.f; // convert into seconds for smooth movement animations
        lastTime = currentTime;

        //Prep the scene
        prepareScene();

        //Process input
        doInput();

        // process temporal input
        // TODO: fix temporalInput() in input.cpp
        // temporalInput(gameTimeline);

        // Event handling for input
        SDL_Event event;

        //modify the game world here
        SDL_FPoint direction = getKeyPress();
        key_movement.calculate(*rectangles[0], direction);

        // gravity.calculate(*rectangles[0]);
        gravity.calculate(*rectangles[1]);

        collision.calculate(*rectangles[1], rectangles);
        collision.calculate(*rectangles[0], rectangles);

        m.moveBetween2Points(*rectangles[2]);


        for (const auto &rectangle: rectangles) {
            if (!anchorTimeline.isPaused())
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
