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
        Transform empty_transform{0, 0, 0, 0};
        Color empty_color{0, 0, 0, 0};
        Collision empty_collision{0, 0};
        RigidBody empty_rigid_body{0};
        auto message = send_strategy->get_message(0, empty_transform, empty_color, Message::SYNC, empty_rigid_body,
                                                  empty_collision);

        //This entity id doesnt matter
        std::string entity_id = client_socket.get(zmq::sockopt::routing_id) + std::to_string(-123);
        NetworkHelper::sendMessageClient(client_socket, entity_id, message);
        std::cout << "Sending SYNC" << std::endl;
        connected = true;
    }

    void send_update(zmq::socket_t &client_socket, Send_Strategy *send_strategy) {
        for (auto entity: entities) {
            auto &transform = gCoordinator.getComponent<Transform>(entity);
            auto &color = gCoordinator.getComponent<Color>(entity);
            auto &clientEntity = gCoordinator.getComponent<ClientEntity>(entity);
            Collision collision{false, false};
            RigidBody rigid_body{0};
            if (gCoordinator.hasComponent<Collision>(entity)) {
                collision = gCoordinator.getComponent<Collision>(entity);
            }
            if (gCoordinator.hasComponent<RigidBody>(entity)) {
                rigid_body = gCoordinator.getComponent<RigidBody>(entity);
            }

            if (previous[entity].equal(transform) && clientEntity.noOfTimes == 0) {
                continue;
            }
            clientEntity.noOfTimes--;
            clientEntity.noOfTimes = std::max(0, clientEntity.noOfTimes);
            auto message = send_strategy->get_message(entity, transform, color, Message::UPDATE, rigid_body, collision);
            previous[entity] = transform;

            std::string entity_id = gCoordinator.getEntityKey(entity);
            NetworkHelper::sendMessageClient(client_socket, entity_id, message);
        }
    }

public:
    void update(zmq::socket_t &client_socket, Send_Strategy *send_strategy) {
        if (!connected) {
            sync(client_socket, send_strategy);
        }
        send_update(client_socket, send_strategy);
    }
};
