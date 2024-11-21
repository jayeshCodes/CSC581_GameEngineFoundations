//
// Created by Utsav Lal on 10/2/24.
//

#include "../model/components.hpp"
#include "../core/defs.hpp"
#include "../ECS/coordinator.hpp"
#include "../ECS/system.hpp"
extern Coordinator gCoordinator;

class KinematicSystem : public System {
public:
    void update(float dt) {
        std::lock_guard<std::mutex> lock(update_mutex);

        // Create a safe copy of entities to iterate over
        std::vector<Entity> currentEntities(entities.begin(), entities.end());

        for(const auto entity: currentEntities) {
            try {
                // Skip if entity no longer exists or doesn't have required components
                if (!gCoordinator.hasComponent<Transform>(entity) ||
                    !gCoordinator.hasComponent<CKinematic>(entity)) {
                    continue;
                    }

                // Get references to components
                auto& transform = gCoordinator.getComponent<Transform>(entity);
                auto& kinematic = gCoordinator.getComponent<CKinematic>(entity);

                // Update rotation
                kinematic.rotation += kinematic.angular_acceleration * dt;

                // Update velocities
                kinematic.velocity.x += kinematic.acceleration.x * dt;
                kinematic.velocity.y += kinematic.acceleration.y * dt;

                // Update positions
                float new_x = transform.x + kinematic.velocity.x * dt;
                float new_y = transform.y + kinematic.velocity.y * dt;

                // Apply the position updates
                transform.x = new_x;
                transform.y = new_y;

                // Update orientation
                transform.orientation += kinematic.rotation * dt;

            } catch (const std::exception& e) {
                std::cerr << "Error updating entity " << entity << " in KinematicSystem: " << e.what() << std::endl;
                continue;
            }
        }
    }
};
