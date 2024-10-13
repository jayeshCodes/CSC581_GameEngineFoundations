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

#include "../helpers/ecs_helpers.hpp"

#define Q_SIZE 100000
#define SERVER_PORT 6000

template<typename T>
class ServerSystem : public System {
private:
    zmq::socket_t connection_socket;

public:
    // Constructor initialization for PackedMap

    ServerSystem() : System() {
        // Initialize the server socket
    }

    void initialize(zmq::context_t &context) {
        connection_socket = zmq::socket_t(context, ZMQ_REP);
        connection_socket.bind("tcp://*:" + std::to_string(SERVER_PORT));
    }

};

#endif // SERVER_HPP
