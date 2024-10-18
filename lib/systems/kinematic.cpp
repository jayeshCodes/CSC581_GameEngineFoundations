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
        for(const auto entity: entities) {
            auto& transform = gCoordinator.getComponent<Transform>(entity);
            auto& kinematic = gCoordinator.getComponent<CKinematic>(entity);

            kinematic.rotation += kinematic.angular_acceleration * dt;

            kinematic.velocity.x += kinematic.acceleration.x * dt;
            kinematic.velocity.y += kinematic.acceleration.y * dt;

            transform.x += kinematic.velocity.x * dt;
            transform.y += kinematic.velocity.y * dt;

            transform.orientation += kinematic.rotation * dt;

            // if(transform.y > SCREEN_HEIGHT) {
            //     transform.y = 0;
            // } else if(transform.y < 0) {
            //     transform.y = SCREEN_HEIGHT;
            // }

            // if (transform.y < 0) {
            //     transform.y = 0;
            //     kinematic.velocity.y = 0; // Stop vertical movement if at the top
            // } else if (transform.y + transform.h > SCREEN_HEIGHT) {
            //     // `rect.h` is the height of the object
            //     transform.y = SCREEN_HEIGHT - transform.h;
            //     kinematic.velocity.y = 0; // Stop vertical movement if at the bottom
            // }
            // if (transform.x < 0) {
            //     transform.x = 0;
            //     kinematic.velocity.x = 0;
            // } else if (transform.x + transform.w > SCREEN_WIDTH) {
            //     transform.x = SCREEN_WIDTH - transform.w;
            //     kinematic.velocity.x = 0;
            // }
        }
    }
};
