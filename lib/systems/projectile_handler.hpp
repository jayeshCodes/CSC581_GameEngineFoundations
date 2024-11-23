//
// Created by Jayesh Gajbhar on 11/22/24.
//

#ifndef PROJECTILE_HANDLER_HPP
#define PROJECTILE_HANDLER_HPP

#include "../ECS/coordinator.hpp"
#include "../EMS/event_coordinator.hpp"
#include "../ECS/system.hpp"
#include "../model/components.hpp"
#include "../core/structs.hpp"
#include "../model/event.hpp"
#include <mutex>

#include "shoot.hpp"

extern Coordinator gCoordinator;
extern EventCoordinator eventCoordinator;

class ProjectileHandlerSystem : public System {
private:
    std::mutex projectileMutex;
    const float MAX_PROJECTILE_DISTANCE = 1000.0f;
    const float PROJECTILE_SPEED = 800.0f;  // Slightly reduced speed
    const float UPWARD_ANGLE_OFFSET = -0.4f; // Add upward bias to projectile trajectory

     void createProjectile(const Transform &shooterTransform, float angle, float nVelocity) {
        std::cout << "Creating projectile" << std::endl;
        Entity projectile = gCoordinator.createEntity();

        // Create projectile sprite
        Sprite projectileSprite;
        projectileSprite.texturePath = "assets/images/knife.png";
        projectileSprite.srcRect = {0, 0, 32, 32};
        projectileSprite.scale = 0.5f;
        projectileSprite.origin = {16, 16};
        projectileSprite.flipX = false;
        projectileSprite.flipY = false;

        // Add upward bias to the angle
        float adjustedAngle = angle;

        // Calculate velocity with adjusted angle
         SDL_FPoint velocity = {
             std::cos(angle) * nVelocity,
             std::sin(angle) * nVelocity
         };

        // Size of projectile
        float projectileSize = 20.0f;  // Increased size for better collision detection

        // Get the nearest monster for trajectory calculation
        Entity nearestMonster = INVALID_ENTITY;
        float minDistance = std::numeric_limits<float>::max();

        for (auto& [id, entity] : gCoordinator.getEntityIds()) {
            if (gCoordinator.hasComponent<Monster>(entity)) {
                auto& monsterTransform = gCoordinator.getComponent<Transform>(entity);
                float dx = monsterTransform.x - shooterTransform.x;
                float dy = monsterTransform.y - shooterTransform.y;
                float distance = std::sqrt(dx * dx + dy * dy);

                if (distance < minDistance) {
                    minDistance = distance;
                    nearestMonster = entity;
                }
            }
        }

        // If we found a monster, adjust the trajectory
        if (nearestMonster != INVALID_ENTITY) {
            auto& monsterTransform = gCoordinator.getComponent<Transform>(nearestMonster);

            // Calculate direction to monster
            float dx = monsterTransform.x - shooterTransform.x;
            float dy = monsterTransform.y - shooterTransform.y;
            float distance = std::sqrt(dx * dx + dy * dy);

            // Add some vertical velocity to account for gravity
            velocity.x = (dx / distance) * PROJECTILE_SPEED;
            velocity.y = ((dy / distance) * PROJECTILE_SPEED) - 300.0f; // Add upward boost

            std::cout << "Targeting monster at (" << monsterTransform.x << ", " << monsterTransform.y
                      << ") with velocity (" << velocity.x << ", " << velocity.y << ")" << std::endl;
        }

        // Position projectile at shooter's position
        gCoordinator.addComponent(projectile, Transform{
            shooterTransform.x + shooterTransform.w / 2 - projectileSize / 2,
            shooterTransform.y + shooterTransform.h / 2 - projectileSize / 2,
            projectileSize,
            projectileSize,
            static_cast<float>(adjustedAngle * (180.0f / M_PI))
        });

        gCoordinator.addComponent(projectile, projectileSprite);
        gCoordinator.addComponent(projectile, CKinematic{{velocity.x, velocity.y}, 0, {0, -200}, 0}); // Added slight gravity
        gCoordinator.addComponent(projectile, Projectile{800.f,100.0f}); // Added damage amount
        gCoordinator.addComponent(projectile, Collision{true, false, CollisionLayer::OTHER});
        gCoordinator.addComponent(projectile, RigidBody{1.0f});
        gCoordinator.addComponent(projectile, ClientEntity{0, false});
        gCoordinator.addComponent(projectile, Destroy{});
    }

    EventHandler projectileHandler = [this](const std::shared_ptr<Event> &event) {
        if (event->type == "ProjectileCreate") {
            const auto &data = event->data.get<ProjectileCreateData>();
            createProjectile(data.shooterTransform, data.angle, data.velocity);
        }
    };

public:
    ProjectileHandlerSystem() {
        eventCoordinator.subscribe(projectileHandler, "ProjectileCreate");
    }

    ~ProjectileHandlerSystem() {
        eventCoordinator.unsubscribe(projectileHandler, "ProjectileCreate");
    }

    void update(float dt) {
        std::lock_guard<std::mutex> lock(projectileMutex);
        updateProjectiles(dt);
    }

private:
    void updateProjectiles(float dt) {
        std::vector<Entity> projectilesToDestroy;
        float cameraY = 0;

        // Get camera position
        for (auto &[id, entity]: gCoordinator.getEntityIds()) {
            if (gCoordinator.hasComponent<Camera>(entity)) {
                cameraY = gCoordinator.getComponent<Camera>(entity).y;
                break;
            }
        }

        for (auto &[id, entity]: gCoordinator.getEntityIds()) {
            if (!gCoordinator.hasComponent<Projectile>(entity)) continue;

            auto &projectile = gCoordinator.getComponent<Projectile>(entity);
            auto &transform = gCoordinator.getComponent<Transform>(entity);
            auto &kinematic = gCoordinator.getComponent<CKinematic>(entity);

            float deltaDistance = std::sqrt(
                                      kinematic.velocity.x * kinematic.velocity.x +
                                      kinematic.velocity.y * kinematic.velocity.y
                                  ) * dt;

            projectile.distanceTravelled += deltaDistance;

            // Check relative to camera position
            float screenY = transform.y - cameraY;

            // Destroy only if projectile is truly off-screen or has traveled too far
            if (projectile.distanceTravelled > MAX_PROJECTILE_DISTANCE ||
                projectile.isDestroyed ||
                transform.x < -50 || transform.x > SCREEN_WIDTH + 50 ||
                screenY < -50 || screenY > SCREEN_HEIGHT + 50) {
                projectilesToDestroy.push_back(entity);
            }
        }

        for (auto entity: projectilesToDestroy) {
            gCoordinator.destroyEntity(entity);
        }
    }
};

#endif //PROJECTILE_HANDLER_HPP
