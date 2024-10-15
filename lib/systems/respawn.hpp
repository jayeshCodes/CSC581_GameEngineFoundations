//
// Created by Jayesh Gajbhar on 10/14/24.
//

#ifndef RESPAWN_HPP
#define RESPAWN_HPP

#include "../ECS/system.hpp"
#include "../model/components.hpp"
#include "../ECS/coordinator.hpp"

extern Coordinator gCoordinator;

class RespawnSystem : public System {
public:
    void update() {
        for (auto &entity: entities) {
            auto &respawnable = gCoordinator.getComponent<Respawnable>(entity);
            if (respawnable.isRespawn) {
                auto &transform = gCoordinator.getComponent<Transform>(entity);
                // respawn the character to the last safe position
                transform = respawnable.lastSafePosition;
                respawnable.isRespawn = false;
            }
        }
    }
};

#endif //RESPAWN_HPP
