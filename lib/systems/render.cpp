//
// Created by Utsav Lal on 10/2/24.
//

#include <SDL_render.h>
#include <iostream>

#include "../components/components.hpp"
#include "../ECS/coordinator.hpp"
#include "../ECS/system.hpp"

#include "../objects/shapes/rectangle.hpp"

extern Coordinator gCoordinator;

class RenderSystem : public System {
public:
    void update(const Camera &camera, float playerX, float playerY) {
        float cameraX = calculateCameraX(camera, playerX);

        for (const Entity entity: entities) {
            const auto &transform = gCoordinator.getComponent<Transform>(entity);
            const auto &color = gCoordinator.getComponent<Color>(entity);

            SDL_SetRenderDrawColor(app->renderer, color.color.r, color.color.g, color.color.b, color.color.a);

            // Convert world coordinates to screen coordinates
            float screenX, screenY;
            worldToScreen(transform.x, transform.y, screenX, screenY, camera, cameraX);

            SDL_FRect tRect = {
                screenX,
                screenY,
                transform.w * camera.zoom,
                transform.h * camera.zoom
            };

            SDL_RenderDrawRectF(app->renderer, &tRect);
            SDL_RenderFillRectF(app->renderer, &tRect);
        }
    }

private:
    float calculateCameraX(const Camera &camera, float playerX) {
        float halfViewport = camera.viewport_width / 2;
        if (playerX > halfViewport) {
            return playerX + 0.75f * halfViewport;
        }
        return 0;
    }

    void worldToScreen(float worldX, float worldY, float &screenX, float &screenY, const Camera &camera,
                       float cameraX) const {
        // Translate X based on camera position, keep Y constant
        float translatedX = worldX - cameraX;
        float translatedY = worldY - camera.y - 100.f;

        // Rotate (if needed)
        float rotatedX = translatedX * std::cos(-camera.rotation) - translatedY * std::sin(-camera.rotation);
        float rotatedY = translatedX * std::sin(-camera.rotation) + translatedY * std::cos(-camera.rotation);

        // Scale
        float scaledX = rotatedX * camera.zoom;
        float scaledY = rotatedY * camera.zoom;

        // Convert to screen coordinates
        screenX = scaledX + camera.viewport_width / 2;
        screenY = scaledY + camera.viewport_height / 2;
    }
};
