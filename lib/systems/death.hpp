//
// Created by Jayesh Gajbhar on 10/14/24.
//

#pragma once
#include "../ECS/system.hpp"
#include "../model/components.hpp"
#include "../ECS/coordinator.hpp"
#include "../EMS/event.hpp"

extern Coordinator gCoordinator;
extern EventCoordinator eventCoordinator;

class DeathSystem : public System {
    const float DEATH_Y = SCREEN_HEIGHT + 100.f;
    const float RESPAWN_HEIGHT = 10.f;

public:
    void update() const {
        for (auto &entity: entities) {
            auto &transform = gCoordinator.getComponent<Transform>(entity);
            auto &respawnable = gCoordinator.getComponent<Respawnable>(entity);

            if ((transform.y > DEATH_Y || respawnable.isRespawn) && !respawnable.isDead) {
                Event event{
                    EntityDeath,
                    EntityDeathData{
                        entity,
                        {
                            respawnable.lastSafePosition.x, respawnable.lastSafePosition.y + RESPAWN_HEIGHT,
                            transform.h, transform.w, transform.orientation, transform.scale
                        }
                    }
                };
                auto time = eventTimeline.getElapsedTime() + 5000;
                eventCoordinator.queueEvent(std::make_shared<Event>(event), time,
                                            Priority::HIGH);
                respawnable.isDead = true;
            }
        }
    }
};
