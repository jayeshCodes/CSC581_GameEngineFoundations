//
// Created by Utsav Lal on 10/15/24.
//

#pragma once

#include <iostream>
#include <unordered_set>
#include <zmq.hpp>

#include "../ECS/coordinator.hpp"
#include "../ECS/system.hpp"
#include "../helpers/network_helper.hpp"
#include "../model/components.hpp"
#include "../strategy/send_strategy.hpp"
#include "../model/event.hpp"

extern Coordinator gCoordinator;

class ClientSystem : public System {
    std::map<Entity, Transform> previous;
    bool isReplaying = false;

    EventHandler startReplayHandler = [this](const std::shared_ptr<Event> &event) {
        if (event->type == eventTypeToString(EventType::StartReplaying)) {
            isReplaying = true;
        }
    };

    EventHandler stopReplayHandler = [this](const std::shared_ptr<Event> &event) {
        if (event->type == eventTypeToString(EventType::StopReplaying)) {
            isReplaying = false;
        }
    };

public:
    ClientSystem() {
        eventCoordinator.subscribe(startReplayHandler, eventTypeToString(EventType::StartReplaying));
        eventCoordinator.subscribe(stopReplayHandler, eventTypeToString(EventType::StopReplaying));
    }

    ~ClientSystem() {
        eventCoordinator.unsubscribe(startReplayHandler, eventTypeToString(EventType::StartReplaying));
        eventCoordinator.unsubscribe(stopReplayHandler, eventTypeToString(EventType::StartReplaying));
    }

    void update(zmq::socket_t &client_socket, Send_Strategy *send_strategy) {
        if(isReplaying) return;
        for (auto entity: entities) {
            auto &client_entity = gCoordinator.getComponent<ClientEntity>(entity);
            if (!client_entity.synced) {
                return;
            }
            auto &transform = gCoordinator.getComponent<Transform>(entity);
            auto &clientEntity = gCoordinator.getComponent<ClientEntity>(entity);
            if (previous[entity].equal(transform) && clientEntity.noOfTimes == 0) {
                continue;
            }
            clientEntity.noOfTimes--;
            clientEntity.noOfTimes = std::max(0, clientEntity.noOfTimes);
            auto message = send_strategy->get_message(entity, Message::UPDATE);
            previous[entity] = transform;

            Event positionChangedEvent{
                eventTypeToString(EventType::PositionChanged),
                PositionChangedData{entity, send_strategy->get_message(entity, Message::UPDATE)}
            };
            eventCoordinator.emitServer(client_socket, std::make_shared<Event>(positionChangedEvent));
        }
    }
};
