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
#include "../generic/safe_queue.hpp"
#include "../helpers/colors.hpp"
#include "../objects/factory.hpp"
#include "../objects/shapes/rectangle.hpp"


Timeline anchorTimeline(nullptr, 1000); // normal tic value of 1
SafeQueue<std::array<float, 3> > messageQueue;

void sendPosition(zmq::socket_t &socket) {
    while (gameRunning) {
        if (messageQueue.notEmpty()) {
            std::array<float, 3> position = messageQueue.dequeue();
            socket.send(zmq::buffer(position, sizeof(position)), zmq::send_flags::none);
        }
    }
}


int main(int argc, char *argv[]) {
    //Call the initialization functions
    std::cout << ENGINE_NAME << " v" << ENGINE_VERSION << " initializing" << std::endl;
    std::cout << "Created by Utsav and Jayesh" << std::endl;
    std::cout << std::endl;
    initSDL();
    anchorTimeline.start();
    gameRunning = true;

    // Create Rectangle instance

    // Launching basic systems of Shade Engine
    KeyMovement key_movement(500, 500);

    //  init timer
    Timeline gameTimeline(&anchorTimeline, 1); // normal tic value of 1
    gameTimeline.start();

    int64_t lastTime = gameTimeline.getElapsedTime();

    auto movementRect = Factory::createRectangle(shade_color::Blue, {SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f, 100, 100}, false,
                                                 1, 1);

    //initialize networking
    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_PUB);
    socket.connect("tcp://localhost:" + std::to_string(PUSH_PULL_SOCKET));

    const float FRAME_RATE_LIMIT = 1000.f / 120.0f;

    // Start thread for sending position
    std::thread netThread(sendPosition, std::ref(socket));

    while (gameRunning) {
        Uint32 currentTime = gameTimeline.getElapsedTime();
        float deltaTime = (currentTime - lastTime) / 1000.0f; // Convert to seconds
        lastTime = currentTime;

        if (deltaTime > FRAME_RATE_LIMIT) {
            deltaTime = FRAME_RATE_LIMIT;
        }

        //Prep the scene
        prepareScene();

        //Process input
        doInput();

        SDL_FPoint direction = getKeyPress();

        key_movement.calculate(*movementRect, direction);

        // std::string message = std::to_string(movementRect->rect.x) + " " + std::to_string(movementRect->rect.y);
        std::array<float, 2> positions = {movementRect->rect.x, movementRect->rect.y};
        messageQueue.enqueue({MessageType::CHARACTER, positions[0], positions[1]});

        movementRect->draw();

        movementRect->update(deltaTime);
        //Present the resulting scene
        presentScene();

        if (deltaTime < FRAME_RATE_LIMIT) {
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(FRAME_RATE_LIMIT - deltaTime)));
        }
    }
    netThread.join();
    cleanupSDL();
    std::cout << "Closing " << ENGINE_NAME << " Engine" << std::endl;
    return 0;
}
