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

    void createProjectile(const Transform &shooterTransform, float angle) {
        std::cout << "Creating projectile" << std::endl;
        Entity projectile = gCoordinator.createEntity();

        // Create projectile sprite
        Sprite projectileSprite;
        projectileSprite.texturePath = "assets/images/knife.png";
        projectileSprite.srcRect = {0, 0, 32, 32};
        projectileSprite.scale = 0.5f;
        projectileSprite.origin = {16, 16}; // Center origin for rotation
        projectileSprite.flipX = false;
        projectileSprite.flipY = false;

        // Calculate velocity based on angle
        float projectileSpeed = 1000.0f; // Adjust as needed
        SDL_FPoint velocity = {
            std::cos(angle) * projectileSpeed,
            std::sin(angle) * projectileSpeed
        };

        // Size of projectile
        float projectileSize = 10.0f;

        // Position projectile at shooter's position
        gCoordinator.addComponent(projectile, Transform{
                                      shooterTransform.x + shooterTransform.w / 2 - projectileSize / 2,
                                      shooterTransform.y + shooterTransform.h / 2 - projectileSize / 2,
                                      projectileSize,
                                      projectileSize,
                                      static_cast<float>(angle * (180.0f / M_PI)) // Convert angle to degrees
                                  });

        gCoordinator.addComponent(projectile, projectileSprite);
        gCoordinator.addComponent(projectile, CKinematic{{velocity.x, velocity.y}, 0, {0, 0}, 0});
        gCoordinator.addComponent(projectile, Projectile{projectileSpeed, 10.0f, 0.0f, false});
        gCoordinator.addComponent(projectile, Collision{true, false, CollisionLayer::OTHER});
        gCoordinator.addComponent(projectile, RigidBody{1.0f});
        gCoordinator.addComponent(projectile, ClientEntity{0, false});
        gCoordinator.addComponent(projectile, Destroy{});

        // Add registration with systems
        Signature projectileSignature;
        projectileSignature.set(gCoordinator.getComponentType<Projectile>());
        projectileSignature.set(gCoordinator.getComponentType<Transform>());
        projectileSignature.set(gCoordinator.getComponentType<CKinematic>());
        gCoordinator.setSystemSignature<ShooterSystem>(projectileSignature);
    }

    EventHandler projectileHandler = [this](const std::shared_ptr<Event> &event) {
        if (event->type == "ProjectileCreate") {
            const auto &data = event->data.get<ProjectileCreateData>();
            createProjectile(data.shooterTransform, data.angle);
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
