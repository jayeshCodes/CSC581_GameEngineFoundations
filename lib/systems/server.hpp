#ifndef SERVER_HPP
#define SERVER_HPP

#include "../ECS/system.hpp"
#include <zmq.hpp>
#include <iostream>
#include <map>
#include "../enum/message_type.hpp"
#include "../generic/packed_map.hpp"
#include "../generic/safe_queue.hpp"
#include <shared_mutex>

#define Q_SIZE 100000
#define SERVER_PORT 8000

template<typename T>
class ServerSystem : public System {
private:
    zmq::socket_t connect_socket;
    PackedMap<int, std::shared_ptr<zmq::socket_t>> client_map;
    mutable std::shared_mutex map_mutex; // Shared mutex for concurrent reads

public:
    // Constructor initialization for PackedMap
    ServerSystem() : client_map(10, nullptr) {}

    void initialize(zmq::context_t &context) {
        connect_socket = zmq::socket_t(context, ZMQ_REP);
        connect_socket.bind("tcp://*:" + std::to_string(SERVER_PORT));
    }

    void connect_client(zmq::context_t &context) {
        std::cout << "Searching for clients" << std::endl;
        std::array<float, 2> request{};
        auto result = connect_socket.recv(zmq::buffer(request, sizeof(request)), zmq::recv_flags::none);

        if (request[0] == Message::CONNECT) {
            std::shared_ptr<zmq::socket_t> client = std::make_shared<zmq::socket_t>(context, ZMQ_PUB);
            client->connect("tcp://localhost:" + std::to_string(request[1]));

            // Exclusive lock for write operation (inserting client)
            std::unique_lock<std::shared_mutex> lock(map_mutex);
            const float slot = client_map.insert(client);
            std::array<float, 1> reply{slot};
            connect_socket.send(zmq::buffer(reply, sizeof(reply)), zmq::send_flags::none);
            std::cout << "Client connected at slot " << slot << std::endl;
        }

        if (request[0] == Message::DISCONNECT) {
            int slot = static_cast<int>(request[1]);

            {
                // Exclusive lock for write operation (removing client)
                std::unique_lock<std::shared_mutex> lock(map_mutex);
                if (client_map[slot]) {
                    client_map[slot]->close();
                    client_map.remove(slot);
                }
            }

            std::array<float, 1> reply{Message::DISCONNECT};
            connect_socket.send(zmq::buffer(reply, sizeof(reply)), zmq::send_flags::none);
            std::cout << "Client disconnected from slot " << slot << std::endl;
        }
    }

    // Thread-safe send_message function with shared lock for reads
    void send_message() {
        zmq::message_t message("message", 7);

        // Shared lock for concurrent reads
        std::shared_lock<std::shared_mutex> lock(map_mutex);
        for (int i = 0; i < client_map.size(); i++) {
            if (client_map[i] != nullptr) {
                client_map[i]->send(message, zmq::send_flags::dontwait);
            }
        }
    }
};

#endif // SERVER_HPP
