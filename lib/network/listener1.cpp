//
// Created by Utsav Lal on 9/2/24.
//
//
// Created by Utsav Lal on 9/1/24.
//
#include <SDL_pixels.h>
#include <thread>
#include <zmq.hpp>

#include "constants.hpp"
#include "../../main.hpp"
#include "../core/timer.hpp"
#include "../core/physics/collision.hpp"
#include "../core/physics/keyMovement.hpp"
#include "../objects/factory.hpp"
#include "../objects/shapes/rectangle.hpp"
#include <sstream>

bool gameRunning = false;

const SDL_Color blueColor = {0, 0, 255, 255};
const SDL_Color redColor = {255, 0, 0, 255};
const SDL_Color greenColor = {0, 255, 0, 255};

Timeline anchorTimeline(nullptr, 1000); // normal tic value of 1


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

    //  init timer
    Timeline gameTimeline(&anchorTimeline, 1); // normal tic value of 1
    gameTimeline.start();

    int64_t lastTime = gameTimeline.getElapsedTime();

    auto movementRect = Factory::createRectangle(blueColor, {SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f, 100, 100}, false,
                                                 1, 1);

    //initialize networking
    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_SUB);
    socket.connect("tcp://localhost:" + std::to_string(PUB_SUB_SOCKET));
    socket.set(zmq::sockopt::subscribe, "");

    const float FRAME_RATE_LIMIT = 1000.f / 120.0f;

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

        zmq::message_t request;

        float positions[2];


        socket.recv(zmq::buffer(positions, sizeof(positions)), zmq::recv_flags::none);
        float xPos = positions[0], yPos = positions[1];

        movementRect->rect.x = xPos;
        movementRect->rect.y = yPos;

        movementRect->draw();

        movementRect->update(deltaTime);
        //Present the resulting scene
        presentScene();

        if (deltaTime < FRAME_RATE_LIMIT) {
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(FRAME_RATE_LIMIT - deltaTime)));
        }
    }

    cleanupSDL();
    std::cout << "Closing " << ENGINE_NAME << " Engine" << std::endl;
    return 0;
}
