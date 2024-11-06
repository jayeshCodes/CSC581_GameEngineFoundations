//
// Created by Utsav Lal on 11/6/24.
//

#pragma once
#include "../ECS/coordinator.hpp"
#include "../ECS/system.hpp"
#include "../model/components.hpp"

extern Coordinator gCoordinator;

class DashSystem : public System {
public:
    void update(const float dt) const {
        for (const auto &entity: entities) {
            auto &[dashSpeed, dashDuration, dashCooldown, isDashing, dashTimeRemaining, cooldownTimeRemaining] =
                    gCoordinator.getComponent<Dash>(entity);
            auto &[velocity, rotation, acceleration, angular_acceleration] = gCoordinator.getComponent<
                CKinematic>(entity);
            if (!isDashing) {
                return;
            }
            // if (cooldownTimeRemaining > 0) {
            //     cooldownTimeRemaining -= dt;
            //     isDashing = false;
            //     return;
            // }
            dashTimeRemaining -= dt;
            if (dashTimeRemaining <= 0) {
                isDashing = false;
                velocity.x = 0;
                cooldownTimeRemaining = dashCooldown;
            } else {
                velocity.x = dashSpeed;
            }
        }
    }
};
