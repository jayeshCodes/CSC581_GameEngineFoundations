//
// Created by Jayesh Gajbhar on 11/14/24.
//

#ifndef BUBBLE_SHOOTER_HPP
#define BUBBLE_SHOOTER_HPP

#include "../ECS/system.hpp"
#include "../model/components.hpp"
#include "../ECS/coordinator.hpp"
#include "../EMS/event_coordinator.hpp"
#include <SDL2/SDL.h>
#include "../helpers/colors.hpp"
#include <cmath>

extern Coordinator gCoordinator;
extern EventCoordinator eventCoordinator;

class BubbleShooterSystem : public System {
private:
    SDL_Color nextBubbleColor;

    // Utility to calculate angle in degrees between two points
    float calculateAngle(float x1, float y1, float x2, float y2) {
        float angle = std::atan2(y2 - y1, x2 - x1) * 180.f / M_PI; // Convert radians to degrees
        if (angle < 0) {
            angle += 360.f; // Normalize negative angles to positive
        }
        return angle;
    }

public:
    BubbleShooterSystem() : nextBubbleColor(shade_color::getRandomBubbleColor()) {
    }

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

            // Handle mouse input
            int mouseX, mouseY;
            Uint32 mouseState = SDL_GetMouseState(&mouseX, &mouseY);

            // Calculate angle based on mouse position (now correctly converted to degrees)
            float angleToMouse = calculateAngle(transform.x, transform.y, mouseX, mouseY);
            shooter.currentAngle = std::clamp(angleToMouse, shooter.minAngle, shooter.maxAngle);


            // Shoot bubble with mouse left click or space bar
            if ((mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) && shooter.canShoot) {
                shootBubble(entity, shooter);
                shooter.canShoot = false;
            }
            else if (keyboardState[SDL_SCANCODE_SPACE] && shooter.canShoot) {
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
        gCoordinator.addComponent(bubble, Color{nextBubbleColor}); // Use stored color
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
