//
// Created by Jayesh Gajbhar on 9/22/24.
//

#include "server.hpp"


int main(int argc, char *argv[]) {
    zmq::context_t context(1);
    zmq::socket_t pub_socket(context, ZMQ_PUB);
    zmq::socket_t rep_socket(context, ZMQ_REP);
    auto platform = Factory::createRectangle({255, 0, 0, 255}, {300, SCREEN_HEIGHT / 2.f, SCREEN_WIDTH / 2.f, 100},
                                             true, 100000.f, 0.8);


    std::thread platform_thread(platform_movement, std::ref(platform));
    std::thread discovery_thread(handle_peer_discovery, std::ref(rep_socket));

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
