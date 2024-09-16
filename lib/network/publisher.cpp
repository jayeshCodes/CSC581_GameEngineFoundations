//
// Created by Utsav Lal on 9/15/24.
//

#include "publisher.hpp"

#include <iostream>

Publisher::Publisher(zmq::context_t context) {
    this->socket = zmq::socket_t(context, ZMQ_PUB);
}

void Publisher::connect(const std::string &address) {
    this->socket.connect(address);
    this->is_connected = true;
}

void Publisher::publish(const std::string &message) {
    if (this->is_connected) {
        this->socket.send(zmq::buffer(message), zmq::send_flags::none);
    } else {
        std::cout << "Publisher not connected" << std::endl;
    }
}
