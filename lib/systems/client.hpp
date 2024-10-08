//
// Created by Utsav Lal on 10/7/24.
//

#ifndef CLIENT_HPP
#define CLIENT_HPP
#include <zmq.hpp>
#include <iostream>

#include "../enum/message_type.hpp"

class ClientSystem : public System {
private:
    zmq::socket_t connect_socket;
    zmq::socket_t listen_socket;
    int slot = -1;

public:
    void initialize(zmq::context_t &context) {
        connect_socket = zmq::socket_t(context, ZMQ_REQ);
        connect_socket.connect("tcp://localhost:8000");

        listen_socket = zmq::socket_t(context, ZMQ_SUB);

    }

    void connect_server(float socket_number) {
        std::cout << "connecting to server" << std::endl;
        listen_socket.bind("tcp://*:" + std::to_string(socket_number));
        listen_socket.set(zmq::sockopt::subscribe, "");

        std::array<float, 2> request{Message::CONNECT, socket_number};
        connect_socket.send(zmq::buffer(request, sizeof(request)), zmq::send_flags::none);


        std::array<float, 1> reply{};
        connect_socket.recv(zmq::buffer(reply, sizeof(reply)), zmq::recv_flags::none);
        std::cout << "Connected to server slot number: " << reply[0] << std::endl;
        slot = static_cast<int>(reply[0]);
    }

    void receive_message() {
        zmq::message_t message;
        listen_socket.recv(message, zmq::recv_flags::none);
        std::string msg = std::string(static_cast<char *>(message.data()), message.size());
    }

    void disconnect() {
        std::array<float, 2> request{Message::DISCONNECT, static_cast<float>(slot)};
        connect_socket.send(zmq::buffer(request, sizeof(request)), zmq::send_flags::none);

        std::array<float, 1> reply{};
        connect_socket.recv(zmq::buffer(reply, sizeof(reply)), zmq::recv_flags::none);
        std::cout << "Disconnected from server" << std::endl;

        if (reply[0] == Message::DISCONNECT) {
            connect_socket.close();
            listen_socket.close();
        }
    }
};
#endif //CLIENT_HPP
