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
#include "../strategy/send_strategy.hpp"

extern Coordinator gCoordinator;

class ClientSystem : public System {
    std::map<Entity, Transform> previous;
    bool connected = false;

    static void check_type_and_send(const std::variant<std::vector<float>, std::string> &message,
                                    zmq::socket_t &client_socket) {
        if (std::holds_alternative<std::string>(message)) {
            const auto str = std::get<std::string>(message);
            client_socket.send(zmq::buffer(str), zmq::send_flags::none);
        } else {
            auto vec = std::get<std::vector<float> >(message);
            client_socket.send(zmq::buffer(vec), zmq::send_flags::none);
        }
    }

    void sync(zmq::socket_t &client_socket, Send_Strategy *send_strategy) {
        Transform empty_transform{0, 0, 0, 0};
        Color empty_color{0, 0, 0, 0};
        auto message = send_strategy->get_message(0, empty_transform, empty_color, Message::SYNC);

        //This entity id doesnt matter
        std::string entity_id = client_socket.get(zmq::sockopt::routing_id) + std::to_string(-123);
        client_socket.send(zmq::buffer(entity_id), zmq::send_flags::sndmore);
        check_type_and_send(message, client_socket);
        std::cout << "Sending SYNC" << std::endl;
        connected = true;
    }

    void send_update(zmq::socket_t &client_socket, Send_Strategy *send_strategy) {
        for (auto entity: entities) {
            auto &transform = gCoordinator.getComponent<Transform>(entity);
            auto &color = gCoordinator.getComponent<Color>(entity);
            auto &clientEntity = gCoordinator.getComponent<ClientEntity>(entity);

            if (previous[entity].equal(transform) && clientEntity.noOfTimes == 0) {
                continue;
            }
            clientEntity.noOfTimes--;
            clientEntity.noOfTimes = std::max(0, clientEntity.noOfTimes);
            auto message = send_strategy->get_message(entity, transform, color, Message::UPDATE);
            previous[entity] = transform;

            std::string entity_id = gCoordinator.getEntityKey(entity);
            client_socket.send(zmq::buffer(entity_id), zmq::send_flags::sndmore);
            check_type_and_send(message, client_socket);
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
