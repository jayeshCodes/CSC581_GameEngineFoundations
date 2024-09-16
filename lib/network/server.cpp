#include <string>
#include <chrono>
#include <iostream>
#include <sstream>
#include <thread>
#include <zmq.hpp>

#include "constants.hpp"
#include "../core/timeline.hpp"

// initialize the server with the given sockets
void initialize_server(zmq::socket_t &pull_socket, zmq::socket_t &pub_socket) {
    pull_socket.bind("tcp://*:" + std::to_string(PUSH_PULL_SOCKET));
    pub_socket.bind("tcp://*:" + std::to_string(PUB_SUB_SOCKET));

    std::cout << "Server initialized and sockets bound" << std::endl;
}

void pull_from_p1(zmq::socket_t &pull_socket, zmq::message_t &request) {
    pull_socket.recv(request, zmq::recv_flags::none);
}

void broadcast(zmq::socket_t &pub_socket, zmq::message_t &message) {
    pub_socket.send(message, zmq::send_flags::none);
}

Timeline globalServerTimeline(nullptr, 1000);

const float FRAME_RATE = 1000.f / 240.f;

int main() {
    using namespace std::chrono_literals;

    zmq::context_t context{1};

    zmq::socket_t listener{context, ZMQ_SUB};
    zmq::socket_t publisher(context, ZMQ_PUB);
    listener.set(zmq::sockopt::subscribe, "");

    initialize_server(listener, publisher);

    globalServerTimeline.start();

    int64_t lastTime = globalServerTimeline.getElapsedTime();

    while (true) {
        int64_t currentTime = globalServerTimeline.getElapsedTime();
        float deltaTime = (currentTime - lastTime) / 1000.f; // convert into seconds
        lastTime = currentTime;

        // if (deltaTime > FRAME_RATE) {
        //     deltaTime = FRAME_RATE;
        // }
        zmq::message_t request;
        pull_from_p1(listener, request);
        broadcast(publisher, request);

        // if (deltaTime < FRAME_RATE) {
        //     std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(FRAME_RATE - deltaTime)));
        // }
    }

    return 0;
}
