//
// Created by Utsav Lal on 10/13/24.
//

#pragma once

#include "../ECS/coordinator.hpp"
#include "../ECS/system.hpp"
#include "../model/components.hpp"

extern Coordinator gCoordinator;

class DestroySystem : public System {
public:
    void update() const {
        std::vector<Entity> entitiesToDestroy;

        for (const auto entity: entities) {
            if (auto &[slot, destroyed, isSent] = gCoordinator.getComponent<Destroy>(entity); destroyed && isSent) {
                entitiesToDestroy.push_back(entity);
            }
        }

        for (const auto entity: entitiesToDestroy) {
            gCoordinator.destroyEntity(entity);
        }
    }
};
