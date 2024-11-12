//
// Created by Utsav Lal on 11/6/24.
//

#pragma once
#include "../ECS/coordinator.hpp"
#include "../ECS/system.hpp"
#include "../EMS/event_coordinator.hpp"

extern Coordinator gCoordinator;
extern EventCoordinator eventCoordinator;

class ComboEventHandler : public System {
    EventHandler handler = [this](const std::shared_ptr<Event> &event) {
        if (event->type == eventTypeToString(EventType::DashRight)) {
            const DashData &data = event->data;
            auto &dash = gCoordinator.getComponent<Dash>(data.entity);
            if (dash.isDashing) return;
            dash.isDashing = true;
            dash.dashTimeRemaining = dash.dashDuration;
            dash.dashSpeed = 500.f;
        }
        if (event->type == eventTypeToString(EventType::DashLeft)) {
            const DashData &data = event->data;
            auto &dash = gCoordinator.getComponent<Dash>(data.entity);
            if (dash.isDashing) return;
            dash.isDashing = true;
            dash.dashTimeRemaining = dash.dashDuration;
            dash.dashSpeed = -500.f;
        }
    };

public:
    ComboEventHandler() {
        eventCoordinator.subscribe(handler, eventTypeToString(EventType::DashRight));
        eventCoordinator.subscribe(handler, eventTypeToString(EventType::DashLeft));
    }

    ~ComboEventHandler() {
        eventCoordinator.subscribe(handler, eventTypeToString(EventType::DashRight));
        eventCoordinator.subscribe(handler, eventTypeToString(EventType::DashLeft));
    }
};
