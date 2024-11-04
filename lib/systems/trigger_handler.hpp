//
// Created by Jayesh Gajbhar on 10/30/24.
//

#ifndef TRIGGER_HANDLER_HPP
#define TRIGGER_HANDLER_HPP

#include "../ECS/coordinator.hpp"
#include "../EMS/event_coordinator.hpp"
#include "../model/components.hpp"
#include "../ECS/system.hpp"

extern Coordinator gCoordinator;
extern EventCoordinator eventCoordinator;

class TriggerHandlerSystem : public System {
    EventHandler triggerHandler = [this](const std::shared_ptr<Event> &event) {
      if(event->type == EventType::EntityTriggered) {
          const auto &data = std::get<EntityTriggeredData>(event->data);
          auto& triggerEntity = data.triggerEntity;
          auto& otherEntity = data.otherEntity;

          auto &kinematic = gCoordinator.getComponent<CKinematic>(otherEntity);
          kinematic.velocity.y = -100.f;
      }
    };

public:
    TriggerHandlerSystem() {
        eventCoordinator.subscribe(triggerHandler, EventType::EntityTriggered);
    }

    ~TriggerHandlerSystem() {
        eventCoordinator.unsubscribe(triggerHandler, EventType::EntityTriggered);
    }
};

#endif //TRIGGER_HANDLER_HPP
