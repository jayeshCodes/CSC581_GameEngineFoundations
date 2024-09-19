#include <thread>
#include <mutex>
#include <condition_variable>
#include "main.hpp"
#include <memory>

#include "lib/core/timer.hpp"
#include "lib/core/physics/collision.hpp"
#include "lib/core/physics/gravity.hpp"
#include "lib/core/physics/keyMovement.hpp"
#include "lib/game/GameManager.hpp"
#include "lib/objects/factory.hpp"
#include "lib/objects/shapes/rectangle.hpp"
#include "lib/animation/controller/moveBetween2Points.hpp"
#include "lib/core/timeline.hpp"
#include "lib/core/input.hpp"  // Make sure this is included

const float TARGET_FRAME_TIME = 1000.f / 60.f; // 60 fps

// Since no anchor this will be global time. The TimeLine class counts in microseconds and hence tic_interval of 1000 ensures this class counts in milliseconds
Timeline anchorTimeline(nullptr, 1);
Timeline gameTimeline(&anchorTimeline, 1);

std::vector<std::unique_ptr<Rectangle>> rectangles;
MoveBetween2Points m(100.f, 400.f, LEFT, 2);
Gravity gravity(0, 1.0);
Collision collision;
KeyMovement key_movement(300, 300);

void gameLogicThread() {
    // Initialize game timeline
    int64_t lastTime = gameTimeline.getElapsedTime();
    float deltaTime = 0;

    while (gameRunning) {
        int64_t currentTime = gameTimeline.getElapsedTime();
        deltaTime = (currentTime - lastTime) / 1000000.0f;
        lastTime = currentTime;
        {
            // Game logic
            SDL_FPoint direction = getKeyPress();
            key_movement.calculate(*rectangles[0], direction);
            gravity.calculate(*rectangles[1]);
            collision.calculate(*rectangles[1], rectangles);
            collision.calculate(*rectangles[0], rectangles);
            m.moveBetween2Points(*rectangles[2]);

            for (const auto &rectangle : rectangles) {
                if (!anchorTimeline.isPaused())
                    rectangle->update(deltaTime);
            }
        }
    }
}

void renderThread() {
    while (gameRunning) {

        prepareScene();

        for (const auto &rectangle : rectangles) {
            rectangle->draw();
        }

        presentScene();
    }
}

void inputThread() {

}

int main(int argc, char *argv[]) {
    std::cout << ENGINE_NAME << " v" << ENGINE_VERSION << " initializing" << std::endl;
    std::cout << "Created by Utsav and Jayesh" << std::endl;
    std::cout << std::endl;
    initSDL();
    gameRunning = true;

    anchorTimeline.start();
    gameTimeline.start();

    // Create Rectangle instances
    rectangles.push_back(Factory::createRectangle({0, 255, 255, 255}, {100, 100, 100, 100}, true, 0.1, 0.8));
    rectangles.push_back(Factory::createRectangle({255, 255, 0, 255}, {300, 100, 100, 100}, true, 0.001, 0.8));
    rectangles.push_back(Factory::createRectangle({255, 0, 0, 255}, {300, SCREEN_HEIGHT / 2.f, SCREEN_WIDTH / 2.f, 100}, true, 100000.f, 0.8));
    rectangles.push_back(Factory::createRectangle({255, 0, 255, 255}, {1000, 100, 100, 100}, false, 1.f, 0.8));

    std::thread logicThread(gameLogicThread);
    std::thread renderingThread(renderThread);

    // Do not multithread this functionality. Causes exception
    while (gameRunning) {
        doInput();
        temporalInput(gameTimeline);
    }


    logicThread.join();
    renderingThread.join();

    cleanupSDL();
    std::cout << "Closing " << ENGINE_NAME << " Engine" << std::endl;
    return 0;
}