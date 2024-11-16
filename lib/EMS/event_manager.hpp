//
// Created by Jayesh Gajbhar on 10/23/24.
//

#ifndef EVENT_MANAGER_HPP
#define EVENT_MANAGER_HPP

#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <typeindex>
#include <unordered_map>
#include <vector>
#include "types.hpp"

#include "../core/timeline.hpp"
#include "../data_structures/ThreadSafePriorityQueue.hpp"
#include "../helpers/network_helper.hpp"

enum Priority { LOW, MEDIUM, HIGH };

struct EventData {
    std::shared_ptr<Event> event;
    int64_t timestamp;
    Priority priority;

    EventData(std::shared_ptr<Event> evt, const int64_t time, const Priority prio)
        : event(std::move(evt)), timestamp(time), priority(prio) {}
};

using EventHandler = std::function<void(std::shared_ptr<Event>)>;
using EventTypeId = std::type_index;
using QueuedEvent = std::shared_ptr<EventData>; // Event, timestamp, priority
constexpr int MAX_EVENTS = 100000;

struct CompareQueuedEvent {
    bool operator()(const QueuedEvent &a, const QueuedEvent &b) const {
        if (a->timestamp != b->timestamp) {
            return a < b; // Compare timestamps
        }
        return a > b; // Compare priorities
    }
};

class EventManager {
private:
    std::unordered_map<std::string, std::vector<EventHandler> > handlers;
    ThreadSafePriorityQueue<QueuedEvent, CompareQueuedEvent> eventQueue;
    std::mutex queueMutex;
    std::mutex handlersMutex;

public:
    void subscribe(const EventHandler &handler, const std::string &eventType) {
        std::lock_guard lock(handlersMutex);
        handlers[eventType].push_back(handler);
    }

    void unsubscribe(const EventHandler handler, const std::string &eventType) {
        std::lock_guard lock(handlersMutex);
        if (!handlers.contains(eventType)) {
            std::cerr << "Error: Invalid eventType" << std::endl;
            return;
        }
        auto &eventHandlers = handlers[eventType];
        // Basic remove implementation for now
        std::erase_if(eventHandlers,
                      [&handler](const EventHandler &h) {
                          return h.target_type() == handler.target_type();
                      });
    }

    // use for immediate event processing
    void emit(const std::shared_ptr<Event> &event) {
        auto typeId = std::type_index(typeid(event->type));
        if (handlers.contains(event->type)) {
            for (auto &handler: handlers[event->type]) {
                handler(std::make_shared<Event>(*event));
            }
        }
    }

    void emitToServer(zmq::socket_t& socket, const std::shared_ptr<Event> &event) {
        NetworkHelper::sendEventClient(socket, event);
    }

    void receiveFromServer(zmq::socket_t socket) {
        std::shared_ptr<Event> event;
        NetworkHelper::receiveEventClient(socket, event);
        emit(event);
    }


    void queueEvent(const std::shared_ptr<EventData>& event) {
        if(eventQueue.size() > MAX_EVENTS) {
            std::cout << "We cannot raise more events since event queue is full!!!" << std::endl;
            return;
        }
        eventQueue.push(event);
    }

    void processEventQueue(int64_t time) {
        QueuedEvent queuedEvent;
        while(eventQueue.pop(queuedEvent)) {
            if(queuedEvent->timestamp <= time) {
                emit(queuedEvent->event);
            } else {
                eventQueue.push(queuedEvent); // Reinsert if not ready to process
                break;
            }
        }
    }

    void clearQueue() {
        eventQueue.clear();
    }


};

#endif //EVENT_MANAGER_HPP
