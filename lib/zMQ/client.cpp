#include <string>
#include <iostream>
#include <thread>

#include <zmq.hpp>

#include "constants.hpp"
#include "../helpers/random.hpp"

int main() {
    // initialize the zmq context with a single IO thread
    zmq::context_t context{1};

    zmq::socket_t push_socket{context, zmq::socket_type::push};

    push_socket.connect("tcp://localhost:" + std::to_string(PUSH_PULL_SOCKET));

    std::cout << "Connected to server" << std::endl;

    while(true) {
        std::string message = "MESSAGE";
        push_socket.send(zmq::buffer(message));
        std::cout << "Sending message: " << message << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }



    return 0;
}
