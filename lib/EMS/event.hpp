//
// Created by Jayesh Gajbhar on 10/23/24.
//

#ifndef EVENT_HPP
#define EVENT_HPP

#include "../ECS/types.hpp"
#include "../model/components.hpp"

// all the events in the game engine will inherit from this class
enum EventType {
    EntityCreated,
};

struct Event {
    EventType type;
    std::variant<std::string> data;
};


#endif //EVENT_HPP
