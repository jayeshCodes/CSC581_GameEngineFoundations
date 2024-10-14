//
// Created by Jayesh Gajbhar on 10/13/24.
//

#include "../ECS/system.hpp"
#include "../model/components.hpp"
#include "../ECS/coordinator.hpp"
#include <mutex>
#include <SDL.h>
#include <iostream>

extern Coordinator gCoordinator;

class JumpSystem : public System {
public:
    void update(float dt) {
        for (const auto &entity: entities) {
            auto &[maxJumpHeight, jumpDuration, isJumping, jumpTime, canJump, initJumpVelo] = gCoordinator.
                    getComponent<Jump>(entity);
            auto &kinematic = gCoordinator.getComponent<CKinematic>(entity);
            auto &transform = gCoordinator.getComponent<Transform>(entity);

            if (isJumping) {
                kinematic.velocity.y = -initJumpVelo;
                isJumping = false;
                canJump = true;
            }
        }
    }
};
