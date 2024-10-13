//
// Created by Utsav Lal on 10/12/24.
//

#pragma once
#include "../ECS/coordinator.hpp"
#include "../ECS/system.hpp"
#include "../enum/message_type.hpp"
#include "../model/components.hpp"


extern Coordinator gCoordinator;

class ServerSystem : public System {
public:
    void update(zmq::socket_t *socket) const {
        std::vector<float> request;
        request.reserve(10 * MAX_ENTITIES);
        for (const auto entity: entities) {
            auto &[x, y, h, w, orientation, scale] = gCoordinator.getComponent<Transform>(entity);
            auto &[color] = gCoordinator.getComponent<Color>(entity);
            request.insert(request.end(), {
                               Message::UPDATE,
                               static_cast<float>(entity),
                               x, y, w, h,
                               static_cast<float>(color.r),
                               static_cast<float>(color.g),
                               static_cast<float>(color.b),
                               static_cast<float>(color.a)
                           });
        }
        request.insert(request.end(), (10 * MAX_ENTITIES - request.size()), Message::END);
        zmq::message_t message(request.data(), request.size() * sizeof(float));
        socket->send(message, zmq::send_flags::none);
    }
};
