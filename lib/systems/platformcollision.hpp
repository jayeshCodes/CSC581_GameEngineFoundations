//
// Created by Jayesh Gajbhar on 10/14/24.
//

#ifndef PLATFORMCOLLISION_HPP
#define PLATFORMCOLLISION_HPP

#include "../ECS/system.hpp"
#include "../model/components.hpp"
#include "../ECS/coordinator.hpp"
#include "respawn.hpp"
#include <iostream>

extern Coordinator gCoordinator;

// Assuming that the platform doesnt use the collision system
class PlatformCollisionSystem : public System {
public:
    float airTime = 0.0f;
    float maxAirTime = 5.0f;

    void update(float dt) {
        // get all the characters which have the respawnable component
        auto characters = gCoordinator.getEntitiesWithComponent<Respawnable>();

        // get all the platform entities
        auto platforms = gCoordinator.getEntitiesWithComponent<Platform>();

        for (auto &character: characters) {
            auto &transform = gCoordinator.getComponent<Transform>(character);
            auto &respawnable = gCoordinator.getComponent<Respawnable>(character);
            auto &gravity = gCoordinator.getComponent<Gravity>(character);

            bool onPlatform = false;

            for (auto &platform: platforms) {
                auto &platformComponent = gCoordinator.getComponent<Platform>(platform);
                auto &platformTransform = gCoordinator.getComponent<Transform>(platform);
                if (isOnPlatform(transform, platformTransform)) {
                    onPlatform = true;
                    gravity.gravY = 0;

                    // snap the character to the platform
                    // transform.y = platformComponent.minY;

                    // reset vertical velocity to 0 since the character is on the platform
                    auto &kinematic = gCoordinator.getComponent<CKinematic>(character);
                    kinematic.velocity.y = std::min(0.f, kinematic.velocity.y);

                    // update the last safe position for the character to respawn
                    respawnable.lastSafePosition = {10.f,SCREEN_HEIGHT - 50.f, 32, 32, 0, 1};
                    break;
                }
            }

            if (!onPlatform) {
                gravity.gravY = 100.f;
                airTime += dt;
                if (airTime > maxAirTime) {
                    // respawn the character
                    respawnable.isRespawn = true;
                    onPlatform = true;
                    std::cout << "Respawning character" << std::endl;
                }
            }
        }
    }

private:
    // helper functions here

    // function to check if the entity is on a platform
    static bool isOnPlatform(const Transform &transform, const Transform &platform) {
        // Check if the character's x position is within the platform's width
        bool withinXBounds = transform.x >= platform.x && transform.x <= platform.x + platform.w;
        std::cout << "Within X bounds: " << withinXBounds << std::endl;

        // Check if the character's y position is at or slightly above the platform's maxY
        bool onPlatformY = transform.y <= platform.y + platform.h && transform.y >= platform.y - 200.f;
        std::cout << "On platform Y: " << onPlatformY << std::endl;

        return withinXBounds && onPlatformY;
    }
};

#endif //PLATFORMCOLLISION_HPP
