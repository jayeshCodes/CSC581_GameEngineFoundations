//
// Created by Utsav Lal on 10/15/24.
//

#pragma once

#include <iostream>
#include <unordered_set>
#include <zmq.hpp>

#include "../ECS/coordinator.hpp"
#include "../ECS/system.hpp"
#include "../enum/enum.hpp"
#include "../helpers/network_helper.hpp"
#include "../model/components.hpp"
#include "../strategy/send_strategy.hpp"

extern Coordinator gCoordinator;

class ClientSystem : public System {
    std::map<Entity, Transform> previous;
    bool connected = false;

    void sync(zmq::socket_t &client_socket, Send_Strategy *send_strategy) {
        auto message = send_strategy->get_message(INVALID_ENTITY, Message::SYNC);

        //This entity id doesnt matter
        std::string entity_id = client_socket.get(zmq::sockopt::routing_id) + std::to_string(-123);
        NetworkHelper::sendMessageClient(client_socket, entity_id, message);
        std::cout << "Sending SYNC" << std::endl;
        connected = true;
    }

    void send_position_update(zmq::socket_t &client_socket, Send_Strategy *send_strategy) {
        for (auto entity: entities) {
            auto &client_entity = gCoordinator.getComponent<ClientEntity>(entity);
            if(!client_entity.synced) {
                std::cout << "here?" << std::endl;
                return;
            }
            auto &transform = gCoordinator.getComponent<Transform>(entity);
            auto &color = gCoordinator.getComponent<Color>(entity);
            auto &clientEntity = gCoordinator.getComponent<ClientEntity>(entity);
            if (previous[entity].equal(transform) && clientEntity.noOfTimes == 0) {
                continue;
            }
            clientEntity.noOfTimes--;
            clientEntity.noOfTimes = std::max(0, clientEntity.noOfTimes);
            auto message = send_strategy->get_message(entity, Message::UPDATE);
            previous[entity] = transform;

            std::string entity_id = gCoordinator.getEntityKey(entity);
            NetworkHelper::sendMessageClient(client_socket, entity_id, message);
        }
    }

public:
    void update(zmq::socket_t &client_socket, Send_Strategy *send_strategy) {
        // if (!connected) {
        //     sync(client_socket, send_strategy);
        // }
        send_position_update(client_socket, send_strategy);
    }
};
