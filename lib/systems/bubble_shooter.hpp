//
// Created by Jayesh Gajbhar on 11/14/24.
//

#ifndef BUBBLE_SHOOTER_HPP
#define BUBBLE_SHOOTER_HPP

#include "../ECS/system.hpp"
#include "../model/components.hpp"
#include "../ECS/coordinator.hpp"
#include "../EMS/event_coordinator.hpp"
#include <cmath>

#include "../helpers/colors.hpp"

extern Coordinator gCoordinator;
extern EventCoordinator eventCoordinator;

class BubbleShooterSystem : public System {
public:
    void update(float dt) {
        for (auto const &entity: entities) {
            auto &shooter = gCoordinator.getComponent<BubbleShooter>(entity);
            auto &transform = gCoordinator.getComponent<Transform>(entity);

            // Update reload timer
            if (!shooter.canShoot) {
                shooter.currentReloadTime += dt;
                if (shooter.currentReloadTime >= shooter.reloadTime) {
                    shooter.canShoot = true;
                    shooter.currentReloadTime = 0.f;
                }
            }

            // Get keyboard state
            const Uint8 *keyboardState = SDL_GetKeyboardState(nullptr);

            // Rotate shooter
            if (keyboardState[SDL_SCANCODE_LEFT]) {
                shooter.currentAngle -= shooter.rotationSpeed;
                shooter.currentAngle = std::max(shooter.currentAngle, shooter.minAngle);
            }
            if (keyboardState[SDL_SCANCODE_RIGHT]) {
                shooter.currentAngle += shooter.rotationSpeed;
                shooter.currentAngle = std::min(shooter.currentAngle, shooter.maxAngle);
            }

            // Shoot bubble
            if (keyboardState[SDL_SCANCODE_SPACE] && shooter.canShoot) {
                shootBubble(entity, shooter);
                shooter.canShoot = false;
            }
        }
    }

private:
    void shootBubble(Entity shooterEntity, BubbleShooter &shooter) {
        auto &shooterTransform = gCoordinator.getComponent<Transform>(shooterEntity);

        // Create a new bubble
        Entity bubble = gCoordinator.createEntity();

        // Calculate starting position at the tip of the shooter
        float angle = shooter.currentAngle * M_PI / 180.f;
        float start_x = shooterTransform.x + shooterTransform.w / 2.f;
        float start_y = shooterTransform.y + shooterTransform.h / 2.f;

        // Add components
        gCoordinator.addComponent<Transform>(bubble, Transform{start_x, start_y, 32.f, 32.f, shooter.currentAngle});

        gCoordinator.addComponent(bubble, Color{shade_color::generateRandomSolidColor()});
        gCoordinator.addComponent(bubble, BubbleProjectile{
                                      SDL_FPoint{
                                          shooter.shootForce * std::cos(angle),
                                          shooter.shootForce * std::sin(angle)
                                      }
                                  });
        gCoordinator.addComponent(bubble, Collision{true, false, CollisionLayer::OTHER});
        gCoordinator.addComponent(bubble, CKinematic{});
    }
};

#endif //BUBBLE_SHOOTER_HPP
