//
// Created by Jayesh Gajbhar on 10/6/24.
//

#include "../ECS/system.hpp"
#include "../components/components.hpp"
#include "../ECS/coordinator.hpp"

extern Coordinator gCoordinator;

class CameraSystem : public System {
public:
    void update(float dt) {
        std::lock_guard<std::mutex> lock(update_mutex);
        for (auto const &entity: entities) {
            auto &camera = gCoordinator.getComponent<Camera>(entity);
        }
    }

    // convert world coordinates to screen coordinates
    void worldToScreen(float worldX, float worldY, float &screenX, float &screenY, const Camera& camera) {
        // Translate
        float translatedX = worldX - camera.x;
        float translatedY = worldY - camera.y;

        // Rotate (co-authored by github co-pilot)
        float rotatedX = translatedX * std::cos(-camera.rotation) - translatedY * std::sin(-camera.rotation);
        float rotatedY = translatedX * std::sin(-camera.rotation) + translatedY * std::cos(-camera.rotation);

        // Scale
        float scaledX = rotatedX * camera.zoom;
        float scaledY = rotatedY * camera.zoom;

        // Convert to screen coordinates
        screenX = (scaledX + camera.viewport_width / 2);
        screenY = (scaledY + camera.viewport_height / 2);
    }

    // convert screen coordinates to world coordinates
    void screenToWorld(float screenX, float screenY, float &worldX, float &worldY, const Camera& camera) {
        // Translate
        float translatedX = screenX - camera.viewport_width / 2;
        float translatedY = screenY - camera.viewport_height / 2;

        // Scale
        float scaledX = translatedX / camera.zoom;
        float scaledY = translatedY / camera.zoom;

        // Rotate
        float rotatedX = scaledX * std::cos(camera.rotation) - scaledY * std::sin(camera.rotation);
        float rotatedY = scaledX * std::sin(camera.rotation) + scaledY * std::cos(camera.rotation);

        // Convert to world coordinates
        worldX = rotatedX + camera.x;
        worldY = rotatedY + camera.y;
    }

    Camera* getMainCamera() {
        //Assuming we are using the first camera in the system
        if(!entities.empty()) {
            return &gCoordinator.getComponent<Camera>(*entities.begin());
        }
        return nullptr;
    }
};
