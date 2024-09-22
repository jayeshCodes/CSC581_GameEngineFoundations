#include <string>
#include <chrono>
#include <iostream>
#include <sstream>
#include <thread>
#include <zmq.hpp>
#include <atomic>
#include <csignal>

#include "constants.hpp"
#include "../core/timeline.hpp"
#include "../generic/safe_queue.hpp"


Timeline globalServerTimeline(nullptr, 1000);

constexpr float FRAME_RATE = 1000.f / 240.f;
SafeQueue<std::array<float, 3>> messageQueue;
std::atomic<bool> running{false};

/**
 * This will switch the game boolean to false killing the game and stopping all threads
 * @param signal - signal to be handled
 */
void signal_handler(int signal) {
    running = false;
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

// initialize the server with the given sockets
void initialize_server(zmq::socket_t &pull_socket, zmq::socket_t &pub_socket) {
    pull_socket.bind("tcp://*:" + std::to_string(PUSH_PULL_SOCKET));
    pub_socket.bind("tcp://*:" + std::to_string(PUB_SUB_SOCKET));

    std::cout << "Server initialized and sockets bound" << std::endl;
}

/**
 * This function will pull the message from the client and add it to the message queue
 * @param pull_socket - socket to pull the message from
 */
void pull_message(zmq::socket_t &pull_socket) {
    while (running) {
        std::array<float, 3> position{};
        if (pull_socket.recv(zmq::buffer(position, sizeof(position)), zmq::recv_flags::dontwait)) {
            messageQueue.enqueue(position);
        }
    }
    std::cout << "Kill pull thread" << std::endl;
}

/**
 * This function will broadcast the message to all the clients
 * @param pub_socket - socket to publish the message
 */
void broadcast(zmq::socket_t &pub_socket) {
    while (running) {
        if (messageQueue.notEmpty()) {
            std::array<float, 3> latestPos = messageQueue.dequeue();
            pub_socket.send(zmq::buffer(latestPos, sizeof(latestPos)), zmq::send_flags::none);
        }
    }
    std::cout << "Kill broadcast thread" << std::endl;
}

int main() {
    using namespace std::chrono_literals;
    register_interrupts();

    zmq::context_t context{1};

    zmq::socket_t listener{context, ZMQ_SUB};
    zmq::socket_t publisher(context, ZMQ_PUB);
    listener.set(zmq::sockopt::subscribe, "");

    initialize_server(listener, publisher);

    globalServerTimeline.start();

    int64_t lastTime = globalServerTimeline.getElapsedTime();
    running = true;

    //Start separate threads for sending and receiving messages so that it doesnt block the main game loop
    std::thread pull_thread(pull_message, std::ref(listener));
    std::thread broadcast_thread(broadcast, std::ref(publisher));

    while (running) {
        int64_t currentTime = globalServerTimeline.getElapsedTime();
        float deltaTime = (currentTime - lastTime) / 1000.f; // convert into seconds
        lastTime = currentTime;
    }

    pull_thread.join();
    broadcast_thread.join();

    std::cout << "Server has shut down!!!" << std::endl;
    return 0;
}
