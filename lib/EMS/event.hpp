//
// Created by Jayesh Gajbhar on 10/23/24.
//

#ifndef EVENT_HPP
#define EVENT_HPP

#include <string>
#include <any>
#include "../ECS/types.hpp"

// all the events in the game engine will inherit from this class
class Event {
public:
    virtual ~Event() = default;
    double timestamp;

    // helper method to get event type name
    virtual const char* getType() const = 0;
};

// Example event (generated by claude)
class EntityCreatedEvent : public Event {
public:
    Entity entity;
    explicit EntityCreatedEvent(Entity e) : entity(e) {}
    const char* getType() const override { return "EntityCreatedEvent"; }
};


#endif //EVENT_HPP
