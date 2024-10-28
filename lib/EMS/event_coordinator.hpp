//
// Created by Utsav Lal on 10/26/24.
//

#ifndef EVENT_COORDINATOR_HPP
#define EVENT_COORDINATOR_HPP
#include <memory>

#include "event_manager.hpp"

class EventCoordinator {
    std::unique_ptr<EventManager> eventManager;
public:
    EventCoordinator() {
        eventManager = std::make_unique<EventManager>();
    }

    void subscribe(const EventHandler &handler, const EventType type) const {
        eventManager->subscribe(handler, type);
    }

    void unsubscribe(const EventHandler &handler, EventType type) const {
        eventManager->unsubscribe(handler, type);
    }

    void emit(const std::shared_ptr<Event>& event) const {
        eventManager->emit(event);
    }

    void queueEvent(std::shared_ptr<Event> event, const int64_t time, const Priority priority) const {
        EventData eventData(std::move(event), time, priority);
        eventManager->queueEvent(std::make_shared<EventData>(eventData));
    }
};

#endif //EVENT_COORDINATOR_HPP
