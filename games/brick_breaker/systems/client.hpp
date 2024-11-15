//
// Created by Utsav Lal on 11/13/24.
//

#pragma once

#include <iostream>
#include <unordered_set>
#include <zmq.hpp>

#include "../../../lib/ECS/coordinator.hpp"
#include "../../../lib/helpers/network_helper.hpp"
#include "../../../lib/strategy/send_strategy.hpp"


extern Coordinator gCoordinator;

class ClientSystem : public System {
    bool connected = false;

public:
    void update(zmq::socket_t &client_socket, Send_Strategy *send_strategy) {
        if (connected) return;
        for (int i = 0; i < 5; i++) {
            NetworkHelper::sendMessageClient(client_socket, "PING", send_strategy->get_message(0, Message::UPDATE));
        }
        connected = true;
    }
};
