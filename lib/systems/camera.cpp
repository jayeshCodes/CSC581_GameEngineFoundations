//
// Created by Jayesh Gajbhar on 10/6/24.
//

#include "../ECS/system.hpp"
#include "../model/components.hpp"
#include "../ECS/coordinator.hpp"

extern Coordinator gCoordinator;

class CameraSystem : public System {
public:
    void update(Entity mainChar) {
        auto &playerTransform = gCoordinator.getComponent<Transform>(mainChar);
        for (auto &entity: entities) {
            auto &[x, y, zoom, rotation, viewport_width, viewport_height] = gCoordinator.getComponent<Camera>(entity);

            // Add some deadzone at the bottom of the screen
            // Only scroll up when player goes above middle of the screen
            float screenMiddleY = viewport_height / 2;

            if (playerTransform.y < y + screenMiddleY) {
                // Player is above middle of screen, move camera up
                y = playerTransform.y - screenMiddleY;
            }

            // Optional: Prevent camera from going below 0
            if (y > 0) {
                y = 0;
            }
        }
    }

    Camera *getMainCamera() {
        //Assuming we are using the first camera in the system
        if (!entities.empty()) {
            return &gCoordinator.getComponent<Camera>(*entities.begin());
        }
        return nullptr;
    }
};
