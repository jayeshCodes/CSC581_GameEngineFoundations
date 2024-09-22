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
#include <csignal>

const SDL_Color blueColor = {0, 0, 255, 255};
const SDL_Color redColor = {255, 0, 0, 255};
const SDL_Color greenColor = {0, 255, 0, 255};

Timeline anchorTimeline(nullptr, 1000); // normal tic value of 1
std::atomic<bool> running{false};

void signal_handler(int signal) {
    running = false;
    std::cout << "Stopping game..." << std::endl;
}

void register_interrupts() {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
    std::signal(SIGABRT, signal_handler);
    std::signal(SIGQUIT, signal_handler);
    std::signal(SIGKILL, signal_handler);
}

void networkThread(zmq::socket_t &socket, float *positions) {
    while (running) {
        socket.recv(zmq::buffer(positions, sizeof(float) * 2), zmq::recv_flags::dontwait);
    }
    std::cout << "Stopping network thread" << std::endl;
}

int main(int argc, char *argv[]) {
    // Call the initialization functions
    std::cout << ENGINE_NAME << " v" << ENGINE_VERSION << " initializing" << std::endl;
    std::cout << "Created by Utsav and Jayesh" << std::endl;
    std::cout << std::endl;
    initSDL();
    anchorTimeline.start();
    running = true;

    register_interrupts();

    // Create Rectangle instance
    auto movementRect = Factory::createRectangle(blueColor, {SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f, 100, 100}, false, 1, 1);

    // Initialize networking
    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_SUB);
    socket.connect("tcp://localhost:" + std::to_string(PUB_SUB_SOCKET));
    socket.set(zmq::sockopt::subscribe, "");

    float positions[2] = {0, 0};
    running = true;
    std::thread netThread(networkThread, std::ref(socket), positions);

    const float FRAME_RATE_LIMIT = 1000.f / 120.0f;
    Timeline gameTimeline(&anchorTimeline, 1); // normal tic value of 1
    gameTimeline.start();
    int64_t lastTime = gameTimeline.getElapsedTime();

    while (running) {
        Uint32 currentTime = gameTimeline.getElapsedTime();
        float deltaTime = (currentTime - lastTime) / 1000.0f; // Convert to seconds
        lastTime = currentTime;

        if (deltaTime > FRAME_RATE_LIMIT) {
            deltaTime = FRAME_RATE_LIMIT;
        }

        // Prep the scene
        prepareScene();

        // Process input
        doInput();

        SDL_FPoint direction = getKeyPress();

        // Update object position
        movementRect->rect.x = positions[0];
        movementRect->rect.y = positions[1];

        movementRect->draw();
        movementRect->update(deltaTime);

        // Present the resulting scene
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