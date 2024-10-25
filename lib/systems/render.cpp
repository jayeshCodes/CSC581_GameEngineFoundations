// RenderSystem.hpp

#include <SDL.h>
#include "../ECS/coordinator.hpp"
#include "../ECS/system.hpp"
#include "../model/components.hpp"
#include "../core/structs.hpp"

extern Coordinator gCoordinator;
extern App *app;

class RenderSystem : public System {
public:
    // Update method to handle rendering and camera control
    void update(const Entity camera) const {
        // Update camera based on the player's position
        Camera cameraComponent{0,0,0,0, 0, 0};
        if(camera != INVALID_ENTITY) {
            cameraComponent = gCoordinator.getComponent<Camera>(camera);
        }

        // Loop through all entities to render them
        for (const Entity entity: entities) {
            const auto &transform = gCoordinator.getComponent<Transform>(entity);
            const auto &color = gCoordinator.getComponent<Color>(entity);

            // Set the color for rendering the entity
            SDL_SetRenderDrawColor(app->renderer, color.color.r, color.color.g, color.color.b, color.color.a);

            // Convert world coordinates to screen coordinates
            SDL_FRect tRect = {
                transform.x - cameraComponent.x,
                transform.y,
                transform.w,
                transform.h
            };

            // Draw and fill the rectangle
            SDL_RenderDrawRectF(app->renderer, &tRect);
            SDL_RenderFillRectF(app->renderer, &tRect);
        }
    }

private:
    // Convert world coordinates to screen coordinates
    static void worldToScreen(float worldX, float worldY, float &screenX, float &screenY, const Camera &camera, float cameraX) {
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
