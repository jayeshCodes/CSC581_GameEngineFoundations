//
// Created by Utsav Lal on 10/12/24.
//

#pragma once
#include <zmq.hpp>

#include "../ECS/coordinator.hpp"
#include "../enum/message_type.hpp"
#include "../generic/packed_map.hpp"

extern Coordinator gCoordinator;

class ServerSystem : public System {
    PackedMap<int, zmq::socket_t *> clients = PackedMap<int, zmq::socket_t *>(MAX_ENTITIES, nullptr);

public:
    void update(zmq::context_t &context, zmq::socket_t &connect_socket) {
        std::array<float, 2> message{};
        if (connect_socket.recv(zmq::buffer(message), zmq::recv_flags::none)) {
            if (message[0] == Message::CONNECT) {
                const int slot = clients.insert(new zmq::socket_t(context, zmq::socket_type::sub));
                const int port = static_cast<int>(message[1]);
                clients[slot]->bind("tcp://localhost:" + std::to_string(port));
                std::array<float, 2> response = {Message::CONNECTED, static_cast<float>(slot)};
                connect_socket.send(zmq::buffer(response), zmq::send_flags::none);
                std::cout << "Client connected: " << slot << std::endl;
            } else if (message[0] == Message::DISCONNECT) {
                const int slot = static_cast<int>(message[1]);
                clients[slot]->close();
                delete clients[slot];
                clients.remove(slot);
                std::array<float, 1> response = {Message::DISCONNECTED};
                connect_socket.send(zmq::buffer(response), zmq::send_flags::none);
                std::cout << "Client disconnected: " << slot << std::endl;
            }
        }
    }
};
