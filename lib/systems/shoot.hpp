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
    const float MAX_TARGET_RANGE = 800.0f;
    const float MIN_SHOOT_INTERVAL = 0.5f;  // Seconds between shots

    struct ProjectileTrajectory {
        float angle;
        float velocity;
    };

    ProjectileTrajectory calculateTrajectory(const Transform& shooter, const Transform& target) {
        float dx = (target.x + target.w/2) - (shooter.x + shooter.w/2);
        float dy = (target.y + target.h/2) - (shooter.y + shooter.h/2);
        float distance = std::sqrt(dx * dx + dy * dy);

        // Calculate direct angle to target
        float rawAngle = std::atan2(dy, dx);

        // Adjust velocity based on distance
        float baseVelocity = 500.0f;
        float velocity = baseVelocity;

        // Modify angle based on distance
        float adjustedAngle = rawAngle;
        if (distance > 200.0f) {
            // For longer distances, create more of an arc
            adjustedAngle -= 0.1f;  // Small upward adjustment
            velocity = baseVelocity * 1.2f;  // Increase velocity
        }

        std::cout << "Trajectory calc - Distance: " << distance
                  << " Raw angle: " << rawAngle
                  << " Adjusted angle: " << adjustedAngle
                  << " Velocity: " << velocity << std::endl;

        return {adjustedAngle, velocity};
    }

    Entity findNearestMonster(const Transform& shooterTransform) {
        Entity nearestMonster = INVALID_ENTITY;
        float minDistance = MAX_TARGET_RANGE;

        for (auto& [id, entity] : gCoordinator.getEntityIds()) {
            if (gCoordinator.hasComponent<Monster>(entity)) {
                auto& monsterTransform = gCoordinator.getComponent<Transform>(entity);

                float dx = (monsterTransform.x + monsterTransform.w/2) - (shooterTransform.x + shooterTransform.w/2);
                float dy = (monsterTransform.y + monsterTransform.h/2) - (shooterTransform.y + shooterTransform.h/2);
                float distance = std::sqrt(dx * dx + dy * dy);

                // Only consider monsters within range and above the player
                if (distance < minDistance && monsterTransform.y < shooterTransform.y) {
                    minDistance = distance;
                    nearestMonster = entity;
                }
            }
        }

        return nearestMonster;
    }

public:
    void update(float dt) {
        const Uint8* keystate = SDL_GetKeyboardState(nullptr);

        for (auto entity : entities) {
            auto& shooter = gCoordinator.getComponent<Shooter>(entity);
            if (!shooter.isActive) continue;

            auto& transform = gCoordinator.getComponent<Transform>(entity);

            if (keystate[SDL_SCANCODE_SPACE] &&
                (gameTimeline.getElapsedTime() - shooter.lastFireTime) / 1000.0f >= MIN_SHOOT_INTERVAL) {

                Entity targetMonster = findNearestMonster(transform);

                if (targetMonster != INVALID_ENTITY) {
                    auto& monsterTransform = gCoordinator.getComponent<Transform>(targetMonster);
                    auto trajectory = calculateTrajectory(transform, monsterTransform);

                    std::cout << "Firing at monster at position: ("
                              << monsterTransform.x << ", " << monsterTransform.y << ")" << std::endl;

                    // Create projectile with calculated trajectory
                    Event projectileEvent{
                        "ProjectileCreate",
                        ProjectileCreateData{
                            transform,
                            trajectory.angle,
                            trajectory.velocity
                        }
                    };
                    eventCoordinator.emit(std::make_shared<Event>(projectileEvent));
                    shooter.lastFireTime = gameTimeline.getElapsedTime();
                }
            }
        }
    }
};

#endif //SHOOT_HPP