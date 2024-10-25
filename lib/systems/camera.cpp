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
            if (playerTransform.x >= x + viewport_width) {
                x += viewport_width;
            } else if (playerTransform.x < x) {
                x -= viewport_width;
                if (x < 0) {
                    x = 0;
                }
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
