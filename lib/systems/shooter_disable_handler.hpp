//
// Created by Jayesh Gajbhar on 11/19/24.
//

#ifndef SHOOTER_DISABLE_HANDLER_HPP
#define SHOOTER_DISABLE_HANDLER_HPP

#include "../ECS/system.hpp"
#include "../model/components.hpp"
#include "../ECS/coordinator.hpp"
#include "../EMS/event_coordinator.hpp"
#include "../model/event.hpp"

extern Coordinator gCoordinator;
extern EventCoordinator eventCoordinator;

class ShooterDisableHandler : public System {
    std::mutex disableMutex;

    EventHandler disableHandler = [this](const std::shared_ptr<Event> &event) {
        if (event->type == eventTypeToString(EventType::DisableShooter)) {
            std::lock_guard<std::mutex> lock(disableMutex);
            DisableShooterData data = event->data;
            const auto currentTime = data.timestamp;

            for (auto entity : entities) {
                try {
                    if (!gCoordinator.hasComponent<BubbleShooter>(entity)) {
                        continue;
                    }

                    auto& shooter = gCoordinator.getComponent<BubbleShooter>(entity);
                    shooter.isDisabled = true;
                    shooter.canShoot = false;
                    shooter.isShooting = false;  // Force stop any shooting
                    shooter.lastDisableTime = currentTime;

                    std::cout << "Disabled shooter " << entity << " at " << currentTime << std::endl;
                } catch (const std::exception& e) {
                    std::cerr << "Error disabling shooter " << entity << ": " << e.what() << std::endl;
                }
            }
        }
    };

public:
    ShooterDisableHandler() {
        eventCoordinator.subscribe(disableHandler, eventTypeToString(EventType::DisableShooter));
    }

    ~ShooterDisableHandler() {
        eventCoordinator.unsubscribe(disableHandler, eventTypeToString(EventType::DisableShooter));
    }
};


#endif //SHOOTER_DISABLE_HANDLER_HPP
