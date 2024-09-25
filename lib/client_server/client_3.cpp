//
// Created by Utsav Lal on 9/2/24.
//
#include <SDL_pixels.h>
#include <thread>
#include <zmq.hpp>

#include "../helpers/network_constants.hpp"
#include "../../main.hpp"
#include "../core/timer.hpp"
#include "../core/physics/collision.hpp"
#include "../core/physics/keyMovement.hpp"
#include "../enum/message_type.hpp"
#include "../game/GameManager.hpp"
#include "../generic/safe_queue.hpp"
#include "../helpers/colors.hpp"
#include "../helpers/constants.hpp"
#include "../network/client_server/client.hpp"
#include "../objects/factory.hpp"
#include "../objects/shapes/rectangle.hpp"


Timeline anchorTimeline(nullptr, 1000); // normal tic value of 1


int main(int argc, char *argv[]) {
    //Call the initialization functions
    std::cout << ENGINE_NAME << " v" << ENGINE_VERSION << " initializing" << std::endl;
    std::cout << "Created by Utsav and Jayesh" << std::endl;
    std::cout << std::endl;
    initSDL();
    anchorTimeline.start();
    GameManager::getInstance()->gameRunning = true;

    // Create Rectangle instance

    // Launching basic systems of Shade Engine
    KeyMovement key_movement(500, 500);

    //  init timer
    Timeline gameTimeline(&anchorTimeline, 1); // normal tic value of 1
    gameTimeline.start();

    int64_t lastTime = gameTimeline.getElapsedTime();

    auto char_1 = Factory::createRectangle(shade_color::Blue, {SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f, 100, 100},
                                           false,
                                           1, 1);
    auto char_2 = Factory::createRectangle(shade_color::Green, {SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f, 100, 100},
                                           false,
                                           1, 1);
    auto char_3 = Factory::createRectangle(shade_color::Yellow, {SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f, 100, 100},
                                           false,
                                           1, 1);

    auto platform = Factory::createRectangle({255, 0, 0, 255}, {300, SCREEN_HEIGHT * 3 / 4.f, SCREEN_WIDTH / 2.f, 100},
                                             true, 100000.f, 0.8);

    //initialize networking
    zmq::context_t context(1);
    Client client;
    client.Initialize(PUB_3, SUB_3, context);
    client.Start(char_1, char_2, char_3, platform);

    while (GameManager::getInstance()->gameRunning) {
        Uint32 currentTime = gameTimeline.getElapsedTime();
        float deltaTime = (currentTime - lastTime) / 1000.0f; // Convert to seconds
        lastTime = currentTime;

        if (deltaTime > engine_constants::FRAME_RATE) {
            deltaTime = engine_constants::FRAME_RATE;
        }

        //Prep the scene
        prepareScene();

        //Process input
        doInput();
        temporalInput(gameTimeline);

        SDL_FPoint direction = getKeyPress();

        key_movement.calculate(*char_3, direction);

        client.messageQueue.enqueue({MessageType::CHAR_3, char_3->rect.x, char_3->rect.y});

        char_1->draw();
        char_2->draw();
        char_3->draw();
        platform->draw();

        char_3->update(deltaTime);
        //Present the resulting scene
        presentScene();

        if (deltaTime < engine_constants::FRAME_RATE) {
            std::this_thread::sleep_for(
                std::chrono::milliseconds(static_cast<int>(engine_constants::FRAME_RATE - deltaTime)));
        }
    }

    cleanupSDL();
    std::cout << "Closing " << ENGINE_NAME << " Engine" << std::endl;
    return 0;
}
