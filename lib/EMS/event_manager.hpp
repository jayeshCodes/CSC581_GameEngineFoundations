//
// Created by Jayesh Gajbhar on 10/23/24.
//

#ifndef EVENT_MANAGER_HPP
#define EVENT_MANAGER_HPP

#include <functional>
#include <queue>
#include <unordered_map>
#include <vector>
#include <memory>
#include <typeindex>
#include <mutex>
#include "event.hpp"
#include "../core/timeline.hpp"

using EventHandler = std::function<void(const Event &)>;
using EventTypeId = std::type_index;

class EventManager {
private:
    std::unordered_map<EventTypeId, std::vector<EventHandler> > handlers;
    std::queue<std::unique_ptr<Event> > eventQueue;
    std::mutex queueMutex;
    std::mutex handlersMutex;
    Timeline *timeline;

    EventManager() {
        timeline = new Timeline(nullptr, 1);
    }

public:
    template<typename T>
    void subscribe(EventHandler handler) {
        std::lock_guard<std::mutex> lock(handlersMutex);
        handlers[std::type_index(typeid(T))].push_back(handler);
    }

    template<typename T>
    void unsubscribe(EventHandler handler) {
        std::lock_guard<std::mutex> lock(handlersMutex);
        auto &eventHandlers = handlers[std::type_index(typeid(T))];
        // Basic remove implementation for now
        eventHandlers.erase(
            std::remove_if(eventHandlers.begin(), eventHandlers.end(),
                           [&handler](const EventHandler &h) {
                               return h.target_type() == handler.target_type();
                           }),
            eventHandlers.end()
        );
    }

    // use for immediate event processing
    template<typename T>
    void emit(const T &event) {
        std::lock_guard<std::mutex> lock(handlersMutex);
        auto typeId = std::type_index(typeid(T));
        if (handlers.find(typeId) != handlers.end()) {
            for (auto &handler: handlers[typeId]) {
                handler(event);
            }
        }
    }

    template<typename T>
    void queueEvent(std::unique_ptr<T> event) {
        std::lock_guard<std::mutex> lock(queueMutex);
        event->timeStamp = timeline->getCurrentTime();
        eventQueue.push(std::move(event));
    }
};

#endif //EVENT_MANAGER_HPP
