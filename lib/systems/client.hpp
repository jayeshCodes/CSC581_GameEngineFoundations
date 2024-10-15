//
// Created by Utsav Lal on 10/15/24.
//

#pragma once

#include <iostream>
#include <unordered_set>
#include <zmq.hpp>

#include "../ECS/coordinator.hpp"
#include "../ECS/system.hpp"
#include "../enum/message_type.hpp"
#include "../model/components.hpp"

extern Coordinator gCoordinator;

class ClientSystem : public System {
    std::map<Entity, Transform> previous;
    bool connected = false;

    void sync(zmq::socket_t &client_socket) {
        std::vector<float> message;
        message.reserve(10);
        message.emplace_back(Message::SYNC);
        for (int i = 0; i < 9; i++) {
            message.emplace_back(0);
        }
        std::string entity_id = client_socket.get(zmq::sockopt::routing_id) + std::to_string(-123);
        client_socket.send(zmq::buffer(entity_id), zmq::send_flags::sndmore);
        client_socket.send(zmq::buffer(message), zmq::send_flags::none);
        std::cout << "Sending SYNC" << std::endl;
        connected = true;
    }

    void send_update(zmq::socket_t &client_socket) {
        for (auto entity: entities) {
            auto &transform = gCoordinator.getComponent<Transform>(entity);
            auto &color = gCoordinator.getComponent<Color>(entity);
            auto &clientEntity = gCoordinator.getComponent<ClientEntity>(entity);

            if (previous[entity].equal(transform) && clientEntity.noOfTimes == 0) {
                continue;
            }
            clientEntity.noOfTimes--;
            clientEntity.noOfTimes = std::max(0, clientEntity.noOfTimes);
            std::vector<float> message;
            message.reserve(10);
            message.emplace_back(Message::UPDATE);
            message.emplace_back(entity);
            message.emplace_back(transform.x);
            message.emplace_back(transform.y);
            message.emplace_back(transform.h);
            message.emplace_back(transform.w);
            message.emplace_back(color.color.r);
            message.emplace_back(color.color.g);
            message.emplace_back(color.color.b);
            message.emplace_back(color.color.a);

            previous[entity] = transform;

            std::string entity_id = client_socket.get(zmq::sockopt::routing_id) + std::to_string(entity);
            client_socket.send(zmq::buffer(entity_id), zmq::send_flags::sndmore);
            client_socket.send(zmq::buffer(message), zmq::send_flags::none);
        }
    }

public:
    void update(zmq::socket_t &client_socket) {
        if (!connected) {
            sync(client_socket);
        }
        send_update(client_socket);
    }
};
