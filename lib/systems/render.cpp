// RenderSystem.hpp

#include <SDL.h>
#include "../ECS/coordinator.hpp"
#include "../ECS/system.hpp"
#include "../model/components.hpp"
#include "../core/structs.hpp"
#include "../helpers/texture_manager.hpp"
#include <iostream>

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

        SDL_Rect viewport;
        SDL_RenderGetViewport(app->renderer, &viewport);

        for (const Entity entity: entities) {
            const auto &transform = gCoordinator.getComponent<Transform>(entity);
            float screenX = transform.x;
            float screenY = transform.y - cameraComponent.y;

            if (gCoordinator.hasComponent<Sprite>(entity)) {
                const auto &sprite = gCoordinator.getComponent<Sprite>(entity);
                SDL_Texture *texture = TextureManager::getInstance()->loadTexture(sprite.texturePath);
                if (!texture) {
                    continue;
                }

                // Get texture dimensions
                int texWidth, texHeight;
                SDL_QueryTexture(texture, nullptr, nullptr, &texWidth, &texHeight);

                // Calculate destination rectangle based on transform dimensions
                SDL_FRect dstRect = {
                    screenX,
                    screenY,
                    transform.w, // Use transform width directly
                    transform.h // Use transform height directly
                };

                // Set up source rectangle to use full texture
                SDL_Rect srcRect = {
                    0, 0, texWidth, texHeight
                };

                // Ensure alpha blending is enabled
                SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

                // Render the sprite
                SDL_RenderCopyExF(
                    app->renderer,
                    texture,
                    &srcRect,
                    &dstRect,
                    transform.orientation,
                    nullptr, // Render from center
                    sprite.flipX ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE
                );

                // Debug visualization of bounds (optional)
#ifdef DEBUG_RENDER
                SDL_SetRenderDrawColor(app->renderer, 255, 0, 0, 255);
                SDL_RenderDrawRectF(app->renderer, &dstRect);
#endif
            } else if (gCoordinator.hasComponent<Color>(entity)) {
                const auto &color = gCoordinator.getComponent<Color>(entity);
                SDL_SetRenderDrawColor(app->renderer, color.color.r, color.color.g, color.color.b, color.color.a);

                SDL_FRect tRect = {
                    screenX,
                    screenY,
                    transform.w,
                    transform.h
                };

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
