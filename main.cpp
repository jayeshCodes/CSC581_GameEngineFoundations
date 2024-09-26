#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>

#include "main.hpp"
#include "lib/core/physics/collision.hpp"
#include "lib/core/physics/gravity.hpp"
#include "lib/core/physics/keyMovement.hpp"
#include "lib/game/GameManager.hpp"
#include "lib/objects/factory.hpp"
#include "lib/objects/shapes/rectangle.hpp"
#include "lib/animation/controller/moveBetween2Points.hpp"
#include "lib/core/timeline.hpp"
#include "lib/core/input.hpp"  // Make sure this is included

// Since no anchor this will be global time. The TimeLine class counts in microseconds and hence tic_interval of 1000 ensures this class counts in milliseconds
Timeline anchorTimeline(nullptr, 1000);
Timeline gameTimeline(&anchorTimeline, 1);

std::vector<std::unique_ptr<Rectangle> > rectangles;
MoveBetween2Points m(100.f, 400.f, LEFT, 2, gameTimeline);
Gravity gravity(0, 1.0);
Collision collision;
KeyMovement key_movement(300, 300);

void physics_thread() {
    // Initialize game timeline
    int64_t lastTime = gameTimeline.getElapsedTime();
    float deltaTime = 0;

    while (GameManager::getInstance()->gameRunning) {
        int64_t currentTime = gameTimeline.getElapsedTime();
        deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime; {
            // Game logic
            gravity.calculate(*rectangles[1]);
            collision.calculate(*rectangles[1], rectangles);
            collision.calculate(*rectangles[0], rectangles);

            for (const auto &rectangle: rectangles) {
                if (!anchorTimeline.isPaused())
                    rectangle->update(deltaTime);
            }
        }
    }
}

void keyboard_movement() {
    int64_t lastTime = gameTimeline.getElapsedTime();
    float deltaTime = 0;

    while (GameManager::getInstance()->gameRunning) {
        int64_t currentTime = gameTimeline.getElapsedTime();
        deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;
        // Game logic
        SDL_FPoint direction = getKeyPress();
        key_movement.calculate(*rectangles[0], direction);

        for (const auto &rectangle: rectangles) {
            if (!anchorTimeline.isPaused())
                rectangle->update(deltaTime);
        }
    }
}

void platform_movement() {
    int64_t lastTime = gameTimeline.getElapsedTime();
    float deltaTime = 0;

    while (GameManager::getInstance()->gameRunning) {
        int64_t currentTime = gameTimeline.getElapsedTime();
        deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime; {
            // Game logic
            m.moveBetween2Points(*rectangles[2]);

            for (const auto &rectangle: rectangles) {
                if (!anchorTimeline.isPaused())
                    rectangle->update(deltaTime);
            }
        }
    }
}

int main(int argc, char *argv[]) {
    std::cout << ENGINE_NAME << " v" << ENGINE_VERSION << " initializing" << std::endl;
    std::cout << "Created by Utsav and Jayesh" << std::endl;
    std::cout << std::endl;
    initSDL();
    GameManager::getInstance()->gameRunning = true;

    anchorTimeline.start();
    gameTimeline.start();

    // Create 4 Rectangle instances
    rectangles.push_back(Factory::createRectangle({0, 255, 255, 255}, {100, 100, 100, 100}, true, 0.1, 0.8));
    rectangles.push_back(Factory::createRectangle({255, 255, 0, 255}, {300, 100, 100, 100}, true, 0.001, 0.8));
    rectangles.push_back(Factory::createRectangle({255, 0, 0, 255}, {300, SCREEN_HEIGHT / 2.f, SCREEN_WIDTH / 2.f, 100},
                                                  true, 100000.f, 0.8));
    rectangles.push_back(Factory::createRectangle({255, 0, 255, 255}, {1000, 100, 100, 100}, false, 1.f, 0.8));

    std::thread logicThread(physics_thread);
    std::thread keyboardThread(keyboard_movement);
    std::thread platformThread(platform_movement);

    // Do not multithread this functionality. Causes exception because SDL_Event is not thread safe and needs to run in the main thread
    while (GameManager::getInstance()->gameRunning) {
        doInput();
        temporalInput(gameTimeline);

        prepareScene();

        for (const auto &rectangle: rectangles) {
            rectangle->draw();
        }

        presentScene();
    }

    logicThread.join();
    keyboardThread.join();
    platformThread.join();

    cleanupSDL();
    std::cout << "Closing " << ENGINE_NAME << " Engine" << std::endl;
    return 0;
}
