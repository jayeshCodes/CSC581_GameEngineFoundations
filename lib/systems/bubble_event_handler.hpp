//
// Created by Jayesh Gajbhar on 11/19/24.
//

#ifndef BUBBLE_EVENT_HANDLER_HPP
#define BUBBLE_EVENT_HANDLER_HPP

#include "../ECS/system.hpp"
#include "../model/components.hpp"
#include "../ECS/coordinator.hpp"
#include "../EMS/event_coordinator.hpp"
#include "../model/event.hpp"
#include "../core/defs.hpp"
#include "../game/GameManager.hpp"

extern Coordinator gCoordinator;
extern EventCoordinator eventCoordinator;

class BubbleEventHandlerSystem : public System {
private:
    EventHandler floatingBubbleshandler = [this](const std::shared_ptr<Event> &event) {
        if (event->type == eventTypeToString(EventType::CheckFloatingBubbles)) {
            if (auto gridSystem = gCoordinator.getSystem<BubbleGridSystem>()) {
                gridSystem->handleFloatingBubbles();
            }
        }
    };

    EventHandler gameOverHandler = [this](const std::shared_ptr<Event>& event) {
      if (event->type == eventTypeToString(EventType::GameOver)) {
          std::cout << "Game Over !!!" << std::endl;
          // TODO : Add Game Over text to screen
          GameManager::getInstance()->gameRunning = false;
      }
    };

public:
    BubbleEventHandlerSystem() {
        eventCoordinator.subscribe(floatingBubbleshandler, eventTypeToString(EventType::CheckFloatingBubbles));
        eventCoordinator.subscribe(gameOverHandler, eventTypeToString(EventType::GameOver));
    }
};

#endif //BUBBLE_EVENT_HANDLER_HPP
