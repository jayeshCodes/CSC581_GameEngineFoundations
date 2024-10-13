//
// Created by Jayesh Gajbhar on 10/13/24.
//

#include "../ECS/system.hpp"
#include "../components/components.hpp"
#include "../ECS/coordinator.hpp"
#include <mutex>
#include <iostream>

extern Coordinator gCoordinator;

class JumpSystem : public System {
public:
   void update(float dt) {
        std::lock_guard<std::mutex> lock(update_mutex);
        for (const auto &entity: entities) {
            auto &jump = gCoordinator.getComponent<Jump>(entity);
            auto &kinematic = gCoordinator.getComponent<CKinematic>(entity);
            auto &transform = gCoordinator.getComponent<Transform>(entity);

            const Uint8 *state = SDL_GetKeyboardState(nullptr);
            if (state[SDL_SCANCODE_SPACE] && !jump.isJumping && jump.canJump) {
                // Initialize jump
                jump.isJumping = true;
                jump.jumpTime = 0.f;
                jump.canJump = false;
                kinematic.velocity.y = -jump.initialJumpVelocity; // Add an initial upward velocity
            }

            if (jump.isJumping) {
                jump.jumpTime += dt;
                float jumpProgress = jump.jumpTime / jump.jumpDuration;

                if (jumpProgress < 1.f) {
                    // Calculate jump height using parabolic function
                    float height = jump.maxJumpHeight * (4 * jumpProgress - 4 * jumpProgress * jumpProgress);
                    float targetVelocity = -height / std::max(dt, 0.0001f); // Avoid division by zero
                    kinematic.velocity.y = std::lerp(kinematic.velocity.y, targetVelocity, 0.1f); // Smooth transition
                } else {
                    // End jump
                    jump.isJumping = false;
                }
            }

            // Apply gravity
            kinematic.velocity.y += 980.f * dt; // Increased gravity for more realistic feel

            // Limit fall speed
            kinematic.velocity.y = std::min(kinematic.velocity.y, 500.f);

            // Update position
            transform.y += kinematic.velocity.y * dt;

            // Ground check
            if (transform.y > SCREEN_HEIGHT - transform.h) {
                transform.y = SCREEN_HEIGHT - transform.h;
                kinematic.velocity.y = 0;
                jump.canJump = true;
                jump.isJumping = false;
            }
        }
    }
};
