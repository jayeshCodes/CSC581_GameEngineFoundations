//
// Created by Jayesh Gajbhar on 11/22/24.
//

#ifndef SHOOT_HPP
#define SHOOT_HPP

#include "../ECS/coordinator.hpp"
#include "../EMS/event_coordinator.hpp"
#include "../ECS/system.hpp"
#include "../model/components.hpp"
#include "../core/structs.hpp"
#include "../core/defs.hpp"
#include "../model/event.hpp"
#include <mutex>

extern Coordinator gCoordinator;
extern EventCoordinator eventCoordinator;

class ShooterSystem : public System {
private:
    std::mutex shootMutex;
    const float MAX_PROJECTILE_DISTANCE = 1000.0f; // Maximum travel distance

    float calculateAngle(float startX, float startY, float targetX, float targetY, float cameraY) {
        // Adjust target Y position based on camera position
        float adjustedTargetY = targetY + cameraY;
        float dx = targetX - startX;
        float dy = adjustedTargetY - startY;
        return std::atan2(dy, dx);
    }



public:
    void update(float dt) {
        int mouseX, mouseY;
        Uint32 mouseState = SDL_GetMouseState(&mouseX, &mouseY);

        float cameraY = 0;
        for (auto& [id, entity] : gCoordinator.getEntityIds()) {
            if (gCoordinator.hasComponent<Camera>(entity)) {
                cameraY = gCoordinator.getComponent<Camera>(entity).y;
                break;
            }
        }

        for (auto entity : entities) {
            auto& shooter = gCoordinator.getComponent<Shooter>(entity);
            if (!shooter.isActive) continue;

            auto& transform = gCoordinator.getComponent<Transform>(entity);
            float angle = calculateAngle(transform.x + transform.w/2, transform.y + transform.h/2,
                                      mouseX, mouseY, cameraY);

            if ((mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) &&
                (gameTimeline.getElapsedTime() - shooter.lastFireTime) / 1000.0f >= shooter.fireRate) {

                Event projectileEvent{
                    "ProjectileCreate",
                    ProjectileCreateData{transform, angle}
                };
                eventCoordinator.emit(std::make_shared<Event>(projectileEvent));
                shooter.lastFireTime = gameTimeline.getElapsedTime();
                }
        }
    }
};

#endif //SHOOT_HPP
