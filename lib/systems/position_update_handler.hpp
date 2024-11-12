//
// Created by Utsav Lal on 11/4/24.
//

#pragma once
#include "../ECS/coordinator.hpp"
#include "../ECS/system.hpp"
#include "../EMS/event_coordinator.hpp"
#include "../model/data_model.hpp"


extern EventCoordinator eventCoordinator;
extern Coordinator gCoordinator;

class PositionUpdateHandler : public System {
    EventHandler positionUpdateHandler = [this](const std::shared_ptr<Event> &event) {
        if (event->type == eventTypeToString(EventType::PositionChanged)) {
            const PositionChangedData &data = event->data;
            const nlohmann::json received_msg = nlohmann::json::parse(data.message);
            const SimpleMessage receivedMessage = received_msg;
            const auto id = gCoordinator.createEntity(receivedMessage.entity_key);
            const auto received_transform = std::get<Transform>(receivedMessage.components[0]);

            if (!gCoordinator.hasComponent<Transform>(id)) { return; }

            auto &transform = gCoordinator.getComponent<Transform>(id);

            transform = received_transform;
        }
    };

public:
    PositionUpdateHandler() {
        eventCoordinator.subscribe(positionUpdateHandler, eventTypeToString(EventType::PositionChanged));
    }

    ~PositionUpdateHandler() {
        eventCoordinator.unsubscribe(positionUpdateHandler, eventTypeToString(EventType::PositionChanged));
    }
};
