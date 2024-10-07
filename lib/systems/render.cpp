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
    // Funtion overload to accept camera system
    void update(const Camera &camera, float x, float y) {
        for (const Entity entity: entities) {
            const auto &transform = gCoordinator.getComponent<Transform>(entity);
            const auto &color = gCoordinator.getComponent<Color>(entity);

            SDL_SetRenderDrawColor(app->renderer, color.color.r, color.color.g, color.color.b, color.color.a);

            // Convert world coordinates to screen coordinates
            float screenX, screenY;
            worldToScreen(transform.x, transform.y, screenX, screenY, camera);

            SDL_FRect tRect;
            tRect = {
                screenX - x + camera.viewport_width / 2 - 300, screenY - y + camera.viewport_height * 3 / 4,
                transform.w * camera.zoom, transform.h * camera.zoom
            };


            SDL_RenderDrawRectF(app->renderer, &tRect);
            SDL_RenderFillRectF(app->renderer, &tRect);
        }
    }

    void update() const {
        for (const Entity entity: entities) {
            const auto &transform = gCoordinator.getComponent<Transform>(entity);
            const auto &color = gCoordinator.getComponent<Color>(entity);

            SDL_SetRenderDrawColor(app->renderer, color.color.r, color.color.g, color.color.b, color.color.a);

            SDL_FRect tRect = {transform.x, transform.y, transform.w, transform.h};

            //TODO Calculate scale

            SDL_RenderDrawRectF(app->renderer, &tRect);
            SDL_RenderFillRectF(app->renderer, &tRect);
        }
    }

private:
    // Helper function to convert world coordinates to screen coordinates (moved from camera.cpp)
    void worldToScreen(float worldX, float worldY, float &screenX, float &screenY, const Camera &camera) const {
        // Translate
        float translatedX = worldX - camera.x;
        float translatedY = worldY - camera.y;

        // Rotate
        float rotatedX = translatedX * std::cos(-camera.rotation) - translatedY * std::sin(-camera.rotation);
        float rotatedY = translatedX * std::sin(-camera.rotation) + translatedY * std::cos(-camera.rotation);

        // Scale
        float scaledX = rotatedX * camera.zoom;
        float scaledY = rotatedY * camera.zoom;

        // Convert to screen coordinates
        screenX = (scaledX + camera.viewport_width / 2);
        screenY = (scaledY + camera.viewport_height / 2);
    }
};
