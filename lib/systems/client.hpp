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
    std::mutex updateMutex;

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
        if (isReplaying) return;

        std::lock_guard<std::mutex> lock(updateMutex);

        // Create a safe copy of entities to iterate over
        std::vector<Entity> currentEntities(entities.begin(), entities.end());

        for (auto entity: currentEntities) {
            try {
                // Verify entity still exists and has required components
                if (!gCoordinator.hasComponent<ClientEntity>(entity) ||
                    !gCoordinator.hasComponent<Transform>(entity)) {
                    // Clean up our tracking for this entity
                    previous.erase(entity);
                    continue;
                }

                auto &client_entity = gCoordinator.getComponent<ClientEntity>(entity);
                if (!client_entity.synced) {
                    continue;
                }

                auto &transform = gCoordinator.getComponent<Transform>(entity);

                // Check if we need to send an update
                if (previous.find(entity) == previous.end() ||
                    !previous[entity].equal(transform) ||
                    client_entity.noOfTimes > 0) {
                    auto message = send_strategy->get_message(entity, Message::UPDATE);
                    previous[entity] = transform;

                    if (client_entity.noOfTimes > 0) {
                        client_entity.noOfTimes--;
                    }

                    Event positionChangedEvent{
                        eventTypeToString(EventType::PositionChanged),
                        PositionChangedData{entity, message}
                    };
                    eventCoordinator.emitServer(client_socket, std::make_shared<Event>(positionChangedEvent));
                }
            } catch (const std::exception &e) {
                std::cerr << "Error processing entity " << entity << ": " << e.what() << std::endl;
                // Clean up tracking for this entity on error
                previous.erase(entity);
            }
        }

        // Clean up previous positions for entities that no longer exist
        for (auto it = previous.begin(); it != previous.end();) {
            if (entities.find(it->first) == entities.end()) {
                it = previous.erase(it);
            } else {
                ++it;
            }
        }
    }
};
