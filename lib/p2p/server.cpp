//
// Created by Jayesh Gajbhar on 9/22/24.
//

#include "server.hpp"

#include "../../../../../opt/homebrew/include/zmq.hpp"


int main() {
    zmq::context_t context(1);
    zmq::socket_t pub_socket(context, ZMQ_PUB);
    zmq::socket_t rep_socket(context, ZMQ_REP);

    pub_socket.bind("tcp://*:" + std::to_string(PUB_SUB_SOCKET));
    rep_socket.bind("tcp://*:" + std::to_string(REP_REQ_SOCKET));

    // Main server loop
    while (running) {
        // Handle any server-specific tasks
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    platform_thread.join();
    discovery_thread.join();

    return 0;
}
