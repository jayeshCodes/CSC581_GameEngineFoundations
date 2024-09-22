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

#include "../animation/controller/moveBetween2Points.hpp"
#include "../enum/message_type.hpp"
#include "../generic/safe_queue.hpp"

const SDL_Color blueColor = {0, 0, 255, 255};
const SDL_Color redColor = {255, 0, 0, 255};
const SDL_Color greenColor = {0, 255, 0, 255};

Timeline anchorTimeline(nullptr, 1000); // normal tic value of 1
std::atomic<bool> running{false};

SafeQueue<std::array<float, 2> > characterMessageQueue;
SafeQueue<std::array<float, 2> > platformMessageQueue;

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

void receive_messages(zmq::socket_t &socket) {
    while (running) {
        std::array<float, 3> position{};
        if (socket.recv(zmq::buffer(position, sizeof(position)), zmq::recv_flags::dontwait)) {
            switch (static_cast<MessageType>(position[0])) {
                case MessageType::CHARACTER:
                    characterMessageQueue.enqueue({position[1], position[2]});
                    break;
                case MessageType::PLATFORM:
                    platformMessageQueue.enqueue({position[1], position[2]});
                    break;
            }
        }
    }
    std::cout << "Stopping network thread" << std::endl;
}

void process_messages(const std::unique_ptr<Rectangle> &character, const std::unique_ptr<Rectangle> &platform) {
    while (running) {
        if (characterMessageQueue.notEmpty()) {
            const std::array<float, 2> pos = characterMessageQueue.dequeue();
            character->rect.x = pos[0];
            character->rect.y = pos[1];
        }
        if(platformMessageQueue.notEmpty()) {
            const std::array<float, 2> pos = platformMessageQueue.dequeue();
            platform->rect.x = pos[0];
            platform->rect.y = pos[1];
        }
    }
}


int main(int argc, char *argv[]) {
    // Call the initialization functions
    std::cout << ENGINE_NAME << " v" << ENGINE_VERSION << " initializing" << std::endl;
    std::cout << "Created by Utsav and Jayesh" << std::endl;
    std::cout << std::endl;
    initSDL();
    anchorTimeline.start();
    Timeline gameTimeline(&anchorTimeline, 1); // normal tic value of 1
    gameTimeline.start();
    running = true;

    register_interrupts();

    // Create Rectangle instance
    auto movementRect = Factory::createRectangle(blueColor, {SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f, 100, 100}, false,
                                                 1, 1);
    auto platform = Factory::createRectangle({255, 0, 0, 255}, {300, SCREEN_HEIGHT / 2.f, SCREEN_WIDTH / 2.f, 100},
                                             true, 100000.f, 0.8);

    auto notNetworkPlatform = Factory::createRectangle({0, 255, 0, 255}, {200, SCREEN_HEIGHT / 4.f, SCREEN_WIDTH / 2.f, 20},
                                             true, 100000.f, 0.8);
    MoveBetween2Points m(100.f, 400.f, LEFT, 2, gameTimeline);

    // Initialize networking
    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_SUB);
    socket.connect("tcp://localhost:" + std::to_string(PUB_SUB_SOCKET));
    socket.set(zmq::sockopt::subscribe, "");

    float positions[2] = {0, 0};
    running = true;
    std::thread netThread(receive_messages, std::ref(socket));
    std::thread movementThread(process_messages, std::ref(movementRect), std::ref(platform));

    const float FRAME_RATE_LIMIT = 1000.f / 120.0f;

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
        temporalInput(gameTimeline);

        SDL_FPoint direction = getKeyPress();

        m.moveBetween2Points(*notNetworkPlatform);

        movementRect->draw();
        platform->draw();
        notNetworkPlatform->draw();
        notNetworkPlatform->update(deltaTime);

        // Present the resulting scene
        presentScene();

        if (deltaTime < FRAME_RATE_LIMIT) {
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(FRAME_RATE_LIMIT - deltaTime)));
        }
    }

    netThread.join();
    movementThread.join();
    cleanupSDL();
    std::cout << "Closing " << ENGINE_NAME << " Engine" << std::endl;
    return 0;
}
