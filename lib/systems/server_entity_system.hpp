//
// Created by Utsav Lal on 10/12/24.
//

#pragma once
#include "../ECS/coordinator.hpp"
#include "../ECS/system.hpp"
#include "../enum/message_type.hpp"
#include "../model/components.hpp"


extern Coordinator gCoordinator;

/**
 * This is a system that will manage all the entities that are to be sent to the client.
 * Any entity that has a Transform, Color, Destroy and ServerEntity component will be sent to all the clients.
 */
class ServerEntitySystem : public System {
    const int MESSAGE_SIZE = 11;

public:
    void update(zmq::socket_t *socket) const {
        std::vector<float> request;
        request.reserve(MESSAGE_SIZE * MAX_ENTITIES);
        for (const auto entity: entities) {
            auto &[x, y, h, w, orientation, scale] = gCoordinator.getComponent<Transform>(entity);
            auto &[slot, destroy, isSent] = gCoordinator.getComponent<Destroy>(entity);
            auto &[color] = gCoordinator.getComponent<Color>(entity);
            if (destroy & !isSent) {
                request.insert(request.end(), {
                                   Message::DESTROY,
                                   static_cast<float>(slot),
                                   static_cast<float>(entity),
                                   0, 0, 0, 0, 0, 0, 0
                               });
                isSent = true;
            } else {
                request.insert(request.end(), {
                                   Message::UPDATE,
                                   static_cast<float>(slot),
                                   static_cast<float>(entity),
                                   x, y, w, h,
                                   static_cast<float>(color.r),
                                   static_cast<float>(color.g),
                                   static_cast<float>(color.b),
                                   static_cast<float>(color.a)
                               });
            }
        }
        request.insert(request.end(), (MESSAGE_SIZE * MAX_ENTITIES - request.size()), Message::END);
        zmq::message_t message(request.data(), request.size() * sizeof(float));
        socket->send(message, zmq::send_flags::none);
    }
};
