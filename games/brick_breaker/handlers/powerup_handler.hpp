//
// Created by Utsav Lal on 11/14/24.
//

#pragma once

#include "../../../lib/ECS/coordinator.hpp"
#include "../../../lib/ECS/system.hpp"
#include "../../../lib/EMS/event_coordinator.hpp"
#include "../../../lib/model/event.hpp"
#include "../model/component.hpp"
#include "../model/events.hpp"

extern Coordinator gCoordinator;
extern EventCoordinator eventCoordinator;
extern Timeline eventTimeline;

class PowerupHandler : public System {
    Entity ball = INVALID_ENTITY;
    Entity launcher = INVALID_ENTITY;


    EventHandler handler = [this](const std::shared_ptr<Event> &event) {
        if (ball == INVALID_ENTITY && launcher == INVALID_ENTITY) {
            for (const auto &entity: gCoordinator.getEntityIds()) {
                if (gCoordinator.hasComponent<Ball>(entity.second)) {
                    ball = entity.second;
                } else if (gCoordinator.hasComponent<Launcher>(entity.second)) {
                    launcher = entity.second;
                }
            }
        }
        if (event->type == eventTypeToString(GameEvents::PowerUpCollected)) {
            const GameEvents::PowerUpCollectedData data = event->data;
            auto &powerupType = data.type;
            if (powerupType == PowerUpType::Elongate) {
                auto &launcherTransform = gCoordinator.getComponent<Transform>(launcher);
                launcherTransform.w += 20;
                launcherTransform.x -= 10;
                if (data.isFinal) return;
                Event qEvent{
                    GameEvents::eventTypeToString(GameEvents::PowerUpCollected),
                    GameEvents::PowerUpCollectedData{PowerUpType::Shorten, true}
                };
                eventCoordinator.queueEvent(std::make_shared<Event>(qEvent), eventTimeline.getElapsedTime() + 10000,
                                            Priority::HIGH);
            } else if (powerupType == PowerUpType::Shorten) {
                auto &launcherTransform = gCoordinator.getComponent<Transform>(launcher);
                launcherTransform.w -= 20;
                launcherTransform.x += 10;
                if (data.isFinal) return;
                Event qEvent{
                    GameEvents::eventTypeToString(GameEvents::PowerUpCollected),
                    GameEvents::PowerUpCollectedData{PowerUpType::Elongate, true}
                };
                eventCoordinator.queueEvent(std::make_shared<Event>(qEvent), eventTimeline.getElapsedTime() + 10000,
                                            Priority::HIGH);
            } else if (powerupType == PowerUpType::SpeedUp) {
                auto &ballKinematic = gCoordinator.getComponent<CKinematic>(ball);
                ballKinematic.velocity.y *= 2.f;
                ballKinematic.velocity.x *= 2.f;
                if (data.isFinal) return;
                Event qEvent{
                    GameEvents::eventTypeToString(GameEvents::PowerUpCollected),
                    GameEvents::PowerUpCollectedData{PowerUpType::SpeedDown, true}
                };
                eventCoordinator.queueEvent(std::make_shared<Event>(qEvent), eventTimeline.getElapsedTime() + 10000,
                                            Priority::HIGH);
            } else if (powerupType == PowerUpType::SpeedDown) {
                auto &ballKinematic = gCoordinator.getComponent<CKinematic>(ball);
                ballKinematic.velocity.y /= 2.f;
                ballKinematic.velocity.x /= 2.f;
                if (data.isFinal) return;
                Event qEvent{
                    GameEvents::eventTypeToString(GameEvents::PowerUpCollected),
                    GameEvents::PowerUpCollectedData{PowerUpType::SpeedUp, true}
                };
                eventCoordinator.queueEvent(std::make_shared<Event>(qEvent), eventTimeline.getElapsedTime() + 10000,
                                            Priority::HIGH);
            }
        }
    };

public:
    PowerupHandler() {
        eventCoordinator.subscribe(handler, eventTypeToString(GameEvents::PowerUpCollected));
    }

    ~PowerupHandler() {
        eventCoordinator.unsubscribe(handler, eventTypeToString(GameEvents::PowerUpCollected));
    }
};
