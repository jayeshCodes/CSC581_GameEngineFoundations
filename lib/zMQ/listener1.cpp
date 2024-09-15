//
// Created by Utsav Lal on 9/13/24.
//

#include <iostream>
#include <zmq.hpp>

#include "constants.hpp"

int main() {
    zmq::context_t context{1};

    zmq::socket_t sub_socket{context, zmq::socket_type::sub};

    sub_socket.connect("tcp://localhost:" + std::to_string(PUB_SUB_SOCKET));

    sub_socket.set(zmq::sockopt::subscribe, "");

    while(true) {
        zmq::message_t request;
        sub_socket.recv(request, zmq::recv_flags::none);
        std::cout << request.to_string() << std::endl;
    }

}
