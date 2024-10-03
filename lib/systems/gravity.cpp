//
// Created by Utsav Lal on 10/2/24.
//

#include "../components/components.hpp"
#include "../ECS/coordinator.hpp"
#include "../ECS/system.hpp"

extern Coordinator gCoordinator;

class GravitySystem : public System {
public:
    void update(float dt) {
        for (auto entity: entities) {
            auto &kinematic = gCoordinator.getComponent<CKinematic>(entity);

            kinematic.acceleration.y += 9.8f * dt;
        }
    }
};
