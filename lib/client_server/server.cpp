#include "../network/client_server/server.hpp"

#include <chrono>
#include <iostream>
#include <zmq.hpp>
#include <atomic>
#include <csignal>

#include "../core/timeline.hpp"
#include "../generic/safe_queue.hpp"
#include "../../main.hpp"
#include "../game/GameManager.hpp"
#include "../objects/shapes/rectangle.hpp"


Timeline globalServerTimeline(nullptr, 1000);

/**
 * This will switch the game boolean to false killing the game and stopping all threads
 * @param signal - signal to be handled
 */
void signal_handler(int signal) {
    GameManager::getInstance()->gameRunning = false;
    std::cout << "Stopping server..." << std::endl;
}

/**
 * Registering interrupts helps to kill all the threads and closing the server gracefully
 */
void register_interrupts() {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
    std::signal(SIGABRT, signal_handler);
    std::signal(SIGQUIT, signal_handler);
}


int main(int argc, char *argv[]) {
    using namespace std::chrono_literals;
    register_interrupts();

    zmq::context_t context{1};
    GameManager::getInstance()->gameRunning = true;

    Server server;
    server.Initialize(context);
    server.Start(globalServerTimeline);

    globalServerTimeline.start();



    while (GameManager::getInstance()->gameRunning) {
        // This is an empty while loop to keep the server running
    }

    std::cout << "Server has shut down!!!" << std::endl;
    return 0;
}
