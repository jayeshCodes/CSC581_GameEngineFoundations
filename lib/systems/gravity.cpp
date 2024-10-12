//
// Created by Utsav Lal on 10/2/24.
//

#include "../model/components.hpp"
#include "../ECS/coordinator.hpp"
#include "../ECS/system.hpp"

extern Coordinator gCoordinator;

class GravitySystem : public System {
public:
    void update(float dt) {
        std::lock_guard<std::mutex> lock(update_mutex);
        for (auto entity: entities) {
            auto &kinematic = gCoordinator.getComponent<CKinematic>(entity);
            auto &gravity = gCoordinator.getComponent<Gravity>(entity);

            kinematic.acceleration.y += 9.8f * dt;
        }
    }
};
