//
// Created by Jayesh Gajbhar on 11/20/24.
//

#ifndef SHOOTER_VERIFICATION_HPP
#define SHOOTER_VERIFICATION_HPP

#include "event_system.hpp"
#include "../ECS/system.hpp"
#include "../model/components.hpp"
#include "../ECS/coordinator.hpp"
#include "../EMS/event_coordinator.hpp"

extern Coordinator gCoordinator;
extern EventCoordinator eventCoordinator;

class ShooterVerificationSystem : public System {
public:
    void update() {
        static int64_t lastCheck = 0;
        auto currentTime = eventTimeline.getElapsedTime();

        // Only check every 1000ms to avoid spam
        if (currentTime - lastCheck < 1000) return;

        lastCheck = currentTime;

        std::cout << "\nShooter Verification at time " << currentTime << ":" << std::endl;

        for (auto entity : entities) {
            try {
                if (!gCoordinator.hasComponent<BubbleShooter>(entity)) {
                    std::cout << "Entity " << entity << " registered but missing BubbleShooter" << std::endl;
                    continue;
                }

                auto& shooter = gCoordinator.getComponent<BubbleShooter>(entity);
                std::cout << "Entity " << entity << " - "
                         << "disabled: " << shooter.isDisabled
                         << ", canShoot: " << shooter.canShoot
                         << ", lastDisableTime: " << shooter.lastDisableTime << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "Error verifying entity " << entity << ": " << e.what() << std::endl;
            }
        }
        std::cout << std::endl;
    }
};

#endif //SHOOTER_VERIFICATION_HPP
