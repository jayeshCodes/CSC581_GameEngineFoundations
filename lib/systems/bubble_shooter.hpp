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
private:
    SDL_Color nextBubbleColor;
public:
    BubbleShooterSystem() : nextBubbleColor(shade_color::getRandomBubbleColor()) {}
    void update(float dt) {
        for (auto const &entity: entities) {
            auto &shooter = gCoordinator.getComponent<BubbleShooter>(entity);
            auto &transform = gCoordinator.getComponent<Transform>(entity);
            auto &color = gCoordinator.getComponent<Color>(entity);

            color.color = nextBubbleColor;

            // Update reload timer
            if (!shooter.canShoot) {
                shooter.currentReloadTime += dt;
                if (shooter.currentReloadTime >= shooter.reloadTime) {
                    shooter.canShoot = true;
                    shooter.currentReloadTime = 0.f;
                    // Generate next color when reloaded
                    nextBubbleColor = shade_color::getRandomBubbleColor();
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

        Entity bubble = gCoordinator.createEntity();

        // Calculate starting position at the tip of the shooter
        float angle = shooter.currentAngle * M_PI / 180.f;
        float start_x = shooterTransform.x;
        float start_y = shooterTransform.y;

        // Add components
        gCoordinator.addComponent(bubble, Transform{start_x, start_y, 32.f, 32.f, shooter.currentAngle});
        gCoordinator.addComponent(bubble, Color{nextBubbleColor});  // Use stored color
        gCoordinator.addComponent(bubble, CKinematic{{0, 0}, 0, {0, 0}, 0});
        gCoordinator.addComponent(bubble, BubbleProjectile{
                                      SDL_FPoint{
                                          shooter.shootForce * std::cos(angle), // Standard trig for direction
                                          shooter.shootForce * std::sin(angle)
                                      },
                                      true
                                  });
        gCoordinator.addComponent(bubble, Destroy{});
        gCoordinator.addComponent(bubble, ClientEntity{});

        nextBubbleColor = shade_color::getRandomBubbleColor();
    }
};

#endif //BUBBLE_SHOOTER_HPP
