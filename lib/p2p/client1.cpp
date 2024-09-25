//
// Created by Utsav Lal on 9/23/24.
//
#include <thread>
#include <zmq.hpp>

#include "../network/p2p/host.hpp"
#include "../core/physics/keyMovement.hpp"
#include "../enum/message_type.hpp"
#include "../game/GameManager.hpp"
#include "../helpers/colors.hpp"
#include "../helpers/constants.hpp"
#include "../helpers/network_constants.hpp"
#include "../objects/factory.hpp"
#include "../../main.hpp"
#include "../objects/shapes/rectangle.hpp"

Timeline anchorTimeline(nullptr, 1000.f);

int main(int argc, char *argv[]) {
    std::cout << ENGINE_NAME << " v" << ENGINE_VERSION << " initializing" << std::endl;
    std::cout << "Created by Utsav and Jayesh" << std::endl;
    std::cout << std::endl;
    initSDL();
    anchorTimeline.start();
    GameManager::getInstance()->gameRunning = true;
    GameManager::getInstance()->isServer = true;

    // Create Rectangle instance

    // Launching basic systems of Shade Engine
    KeyMovement key_movement(500, 500);

    //  init timer
    Timeline gameTimeline(&anchorTimeline, 1); // normal tic value of 1
    gameTimeline.start();


    auto char1 = Factory::createRectangle(shade_color::Blue,
                                          {SCREEN_WIDTH / 3.f - 250.f, SCREEN_HEIGHT / 2.f, 100, 100}, false,
                                          1, 1);
    auto char2 = Factory::createRectangle(shade_color::Green,
                                          {SCREEN_WIDTH * 2 / 3.f - 250.f, SCREEN_HEIGHT / 2.f, 100, 100},
                                          false,
                                          1, 1);
    auto char3 = Factory::createRectangle(shade_color::Red,
                                          {SCREEN_WIDTH * 3 / 3.f - 250.f, SCREEN_HEIGHT / 2.f, 100, 100},
                                          false,
                                          1, 1);

    auto platform = Factory::createRectangle(shade_color::Yellow,
                                             {SCREEN_WIDTH / 2.f, SCREEN_HEIGHT * 3 / 4.f, 100, 50}, false,
                                             1, 1);

    //initialize networking
    zmq::context_t context(1);

    Host host(std::to_string(CHAR2), std::to_string(CHAR3), context, std::to_string((CHAR1)));
    host.start(char2, char3, char1, platform, anchorTimeline);


    int64_t lastTime = gameTimeline.getElapsedTime();
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

        key_movement.calculate(*char1, direction);

        std::array<float, 2> positions = {char1->rect.x, char1->rect.y};
        host.send_queue.enqueue({P2PMessageType::P2P_CHARACTER_1, positions[0], positions[1]});

        char1->draw();
        char2->draw();
        char3->draw();
        platform->draw();

        char1->update(deltaTime);


        //Present the resulting scene
        presentScene();

        // Wait till we achieve desired frame rate
        if (deltaTime < engine_constants::FRAME_RATE) {
            std::this_thread::sleep_for(
                std::chrono::milliseconds(static_cast<int>(engine_constants::FRAME_RATE - deltaTime)));
        }
    }

    cleanupSDL();
    std::cout << "Closing " << ENGINE_NAME << " Engine" << std::endl;
    return 0;
}
