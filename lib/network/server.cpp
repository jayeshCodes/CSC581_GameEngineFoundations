#include <string>
#include <chrono>
#include <iostream>
#include <sstream>
#include <thread>
#include <zmq.hpp>
#include <atomic>
#include <csignal>

#include "constants.hpp"
#include "../animation/controller/moveBetween2Points.hpp"
#include "../core/timeline.hpp"
#include "../enum/message_type.hpp"
#include "../generic/safe_queue.hpp"
#include "../objects/factory.hpp"
#include "../../main.hpp"
#include "../objects/shapes/rectangle.hpp"


Timeline globalServerTimeline(nullptr, 1000);

constexpr float FRAME_RATE = 1000.f / 240.f;
SafeQueue<std::array<float, 3> > messageQueue;

/**
 * This will switch the game boolean to false killing the game and stopping all threads
 * @param signal - signal to be handled
 */
void signal_handler(int signal) {
    gameRunning = false;
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
    while (gameRunning) {
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
    while (gameRunning) {
        if (messageQueue.notEmpty()) {
            std::array<float, 3> latestPos = messageQueue.dequeue();
            pub_socket.send(zmq::buffer(latestPos, sizeof(latestPos)), zmq::send_flags::none);
        }
    }
    std::cout << "Kill broadcast thread" << std::endl;
}

void platform_movement(std::unique_ptr<Rectangle> &platform) {
    Timeline platformTimeline(&globalServerTimeline, 1);
    int64_t lastTime = platformTimeline.getElapsedTime();
    MoveBetween2Points m(100.f, 400.f, LEFT, 2, platformTimeline);
    while (gameRunning) {
        int64_t currentTime = platformTimeline.getElapsedTime();
        float dT = (currentTime - lastTime) / 1000.f;
        lastTime = currentTime;

        if(dT > FRAME_RATE) {
            dT = FRAME_RATE;
        }

        m.moveBetween2Points(*platform);
        platform->update(dT);
        messageQueue.enqueue({MessageType::PLATFORM, platform->rect.x, platform->rect.y});

        if (dT < FRAME_RATE) {
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(FRAME_RATE - dT)));
        }
    }

    std::cout << "Kill platform thread" << std::endl;
}

int main(int argc, char* argv[]) {
    using namespace std::chrono_literals;
    register_interrupts();

    zmq::context_t context{1};

    zmq::socket_t listener{context, ZMQ_SUB};
    zmq::socket_t publisher(context, ZMQ_PUB);
    listener.set(zmq::sockopt::subscribe, "");

    initialize_server(listener, publisher);

    globalServerTimeline.start();

    int64_t lastTime = globalServerTimeline.getElapsedTime();
    gameRunning = true;

    auto platform = Factory::createRectangle({255, 0, 0, 255}, {300, SCREEN_HEIGHT / 2.f, SCREEN_WIDTH / 2.f, 100},
                                             true, 100000.f, 0.8);

    //Start separate threads for sending and receiving messages so that it doesn't block the main game loop
    std::thread pull_thread(pull_message, std::ref(listener));
    std::thread broadcast_thread(broadcast, std::ref(publisher));
    std::thread platform_thread(platform_movement, std::ref(platform));

    while (gameRunning) {
        int64_t currentTime = globalServerTimeline.getElapsedTime();
        float deltaTime = (currentTime - lastTime) / 1000.f; // convert into seconds
        lastTime = currentTime;
    }

    pull_thread.join();
    broadcast_thread.join();
    platform_thread.join();

    std::cout << "Server has shut down!!!" << std::endl;
    return 0;
}
