//
// Created by Jayesh Gajbhar on 11/19/24.
//

#ifndef SHOOTER_HANDLER_HPP
#define SHOOTER_HANDLER_HPP

#include "../ECS/system.hpp"
#include "../model/components.hpp"
#include "../ECS/coordinator.hpp"
#include "../EMS/event_coordinator.hpp"
#include "../model/event.hpp"
#include "../core/defs.hpp"
#include "../game/GameManager.hpp"
#include "bubble_grid.hpp"

extern Coordinator gCoordinator;
extern EventCoordinator eventCoordinator;

class ShooterHandlerSystem : public System {
private:
    std::mutex resetMutex;

    EventHandler resetHandler = [this](const std::shared_ptr<Event> &event) {
        if (event->type == eventTypeToString(EventType::ResetShooter)) {
            std::lock_guard<std::mutex> lock(resetMutex);
            ResetShooterData data = event->data;
            const auto resetTime = data.timestamp;

            for (auto entity: entities) {
                try {
                    if (!gCoordinator.hasComponent<BubbleShooter>(entity)) {
                        continue;
                    }

                    auto &shooter = gCoordinator.getComponent<BubbleShooter>(entity);
                    if (resetTime > shooter.lastDisableTime) {
                        shooter.isDisabled = false;
                        shooter.canShoot = true;
                        shooter.isShooting = false;
                        shooter.currentReloadTime = shooter.reloadTime;
                        shooter.lastShootTime = resetTime; // Prevent immediate shooting after reset

                        std::cout << "Reset shooter " << entity << " at " << resetTime << std::endl;
                    }
                } catch (const std::exception &e) {
                    std::cerr << "Error resetting shooter " << entity << ": " << e.what() << std::endl;
                }
            }
        }
    };

public:
    ShooterHandlerSystem() {
        eventCoordinator.subscribe(resetHandler, eventTypeToString(EventType::ResetShooter));
    }

    ~ShooterHandlerSystem() {
        eventCoordinator.unsubscribe(resetHandler, eventTypeToString(EventType::ResetShooter));
    }
};

#endif //SHOOTER_HANDLER_HPP
