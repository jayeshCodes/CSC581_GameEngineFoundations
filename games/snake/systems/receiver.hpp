//
// Created by Utsav Lal on 11/13/24.
//

#pragma once

#include <iostream>

#include "../../../lib/ECS/system.hpp"
#include "../../../lib/helpers/network_helper.hpp"
#include "../../../lib/strategy/send_strategy.hpp"

class ReceiverSystem : public System {

    void handleMessage(Send_Strategy *send_strategy, zmq::message_t &copy, std::string &entity_id) {

    }

public:
    void update(zmq::socket_t &socket, Send_Strategy *send_strategy) {
        zmq::pollitem_t items[] = {{static_cast<void *>(socket), 0, ZMQ_POLLIN, 0}};
        zmq::poll(items, 1, std::chrono::milliseconds(17));
        if (items[0].revents & ZMQ_POLLIN) {
            zmq::message_t copy;
            std::string entity_id;
            NetworkHelper::receiveMessageClient(socket, copy, entity_id);
            handleMessage(send_strategy, copy, entity_id);
        }
    }
};
