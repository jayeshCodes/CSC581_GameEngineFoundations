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
        Camera cameraComponent{0, 0, 0, 0, 0, 0};
        if (camera != INVALID_ENTITY) {
            cameraComponent = gCoordinator.getComponent<Camera>(camera);
        }

        // Loop through all entities to render them
        for (const Entity entity: entities) {
            const auto &transform = gCoordinator.getComponent<Transform>(entity);

            // Calculate screen position
            float screenX = transform.x;
            float screenY = transform.y - cameraComponent.y;

            // If entity has a sprite, render it
            if (gCoordinator.hasComponent<Sprite>(entity)) {
                const auto &sprite = gCoordinator.getComponent<Sprite>(entity);
                if (!sprite.texture) continue; // Skip if texture not loaded

                // Calculate destination rectangle
                SDL_FRect dstRect = {
                    screenX,
                    screenY,
                    transform.w * sprite.scale,
                    transform.h * sprite.scale
                };

                // Calculate flip flags
                SDL_RendererFlip flip = SDL_FLIP_NONE;
                if (sprite.flipX) flip = (SDL_RendererFlip) (flip | SDL_FLIP_HORIZONTAL);
                if (sprite.flipY) flip = (SDL_RendererFlip) (flip | SDL_FLIP_VERTICAL);

                // Render the sprite
                SDL_RenderCopyExF(
                    app->renderer,
                    sprite.texture,
                    &sprite.srcRect,
                    &dstRect,
                    transform.orientation,
                    &sprite.origin,
                    flip
                );
            }
            // If entity has a color, render colored rectangle
            else if (gCoordinator.hasComponent<Color>(entity)) {
                const auto &color = gCoordinator.getComponent<Color>(entity);
                SDL_SetRenderDrawColor(app->renderer, color.color.r, color.color.g, color.color.b, color.color.a);

                SDL_FRect tRect = {
                    screenX,
                    screenY,
                    transform.w,
                    transform.h
                };

                SDL_RenderDrawRectF(app->renderer, &tRect);
                SDL_RenderFillRectF(app->renderer, &tRect);
            }
        }
    }

private:
    // Convert world coordinates to screen coordinates
    static void worldToScreen(float worldX, float worldY, float &screenX, float &screenY, const Camera &camera,
                              float cameraX) {
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
