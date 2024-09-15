#include <string>
#include <chrono>
#include <iostream>
#include <sstream>
#include <zmq.hpp>

#include "constants.hpp"

// initialize the server with the given sockets
void initialize_server(zmq::socket_t &pull_socket, zmq::socket_t &pub_socket) {
    pull_socket.bind("tcp://*:" + std::to_string(PUSH_PULL_SOCKET));
    pub_socket.bind("tcp://*:" + std::to_string(PUB_SUB_SOCKET));

    std::cout << "Server initialized and sockets bound" << std::endl;
}

// PULL the message and check if it is sync
void connect_p1(zmq::socket_t &pull_socket, bool &p1_connected) {
    zmq::message_t request;
    pull_socket.recv(request, zmq::recv_flags::none);
    if (std::strcmp(request.to_string().c_str(), SYNC) == 0) {
        p1_connected = true;
        std::cout << "P1 connected" << std::endl;
    }
}

void pull_from_p1(zmq::socket_t &pull_socket, zmq::message_t &request) {
    pull_socket.recv(request, zmq::recv_flags::none);
    std::cout << "Received message from P1" << std::endl;
}

void broadcast(zmq::socket_t &pub_socket, zmq::message_t &message) {
    std::cout << "message: " << message.to_string() << std::endl;
    pub_socket.send(message, zmq::send_flags::none);
    std::cout << "Sending message to all listeners" << std::endl;
}

int main() {
    using namespace std::chrono_literals;

    zmq::context_t context{1};

    zmq::socket_t p1_socket{context, zmq::socket_type::pull};
    zmq::socket_t publisher(context, zmq::socket_type::pub);

    initialize_server(p1_socket, publisher);

    while (true) {
        zmq::message_t request;
        pull_from_p1(p1_socket, request);
        broadcast(publisher, request);
    }

    return 0;
}
