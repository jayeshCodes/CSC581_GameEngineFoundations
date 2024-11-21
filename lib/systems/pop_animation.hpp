//
// Created by Jayesh Gajbhar on 11/21/24.
//

#ifndef POP_ANIMATION_HPP
#define POP_ANIMATION_HPP

#include "../ECS/system.hpp"
#include "../ECS/coordinator.hpp"
#include "../model/components.hpp"
#include "../EMS/event_coordinator.hpp"
#include "../model/event.hpp"

extern Coordinator gCoordinator;
extern EventCoordinator eventCoordinator;

class PopAnimationSystem : public System {
public:
    void update(float dt) {
        std::vector<Entity> completedAnimations;

        for (auto entity: entities) {
            if (!gCoordinator.hasComponent<PopAnimation>(entity) ||
                !gCoordinator.hasComponent<Transform>(entity) ||
                !gCoordinator.hasComponent<Color>(entity)) {
                continue;
            }

            auto &animation = gCoordinator.getComponent<PopAnimation>(entity);
            if (!animation.isAnimating) continue;

            auto &transform = gCoordinator.getComponent<Transform>(entity);
            auto &color = gCoordinator.getComponent<Color>(entity);

            // Update animation time
            animation.currentTime += dt;
            float progress = animation.currentTime / animation.duration;

            if (progress >= 1.0f) {
                // Animation complete, mark for destruction
                completedAnimations.push_back(entity);
                continue;
            }

            // Use a smooth easing function for better visual effect
            float easedProgress = 0.5f - 0.5f * std::cos(progress * M_PI);

            // Scale up and then down with a bounce effect
            float scaleProgress = 1.0f + std::sin(progress * M_PI) * (animation.finalScale - animation.initialScale);
            transform.scale = scaleProgress;

            // Rotate slightly during animation for extra effect
            transform.orientation = std::sin(progress * M_PI * 2) * 15.0f; // 15 degree rotation

            // Fade out with easing
            color.color.a = static_cast<Uint8>((1.0f - easedProgress) * 255);

            // Optional: Add slight movement during animation
            transform.y -= dt * 50.0f; // Drift upward slightly
        }

        // Clean up completed animations
        for (auto entity: completedAnimations) {
            if (!gCoordinator.hasComponent<Destroy>(entity)) {
                gCoordinator.addComponent<Destroy>(entity, Destroy{});
            }
            gCoordinator.getComponent<Destroy>(entity).destroy = true;
        }
    }
};

#endif //POP_ANIMATION_HPP
