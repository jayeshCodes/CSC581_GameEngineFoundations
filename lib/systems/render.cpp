// RenderSystem.hpp

#include <SDL.h>
#include "../ECS/coordinator.hpp"
#include "../ECS/system.hpp"
#include "../model/components.hpp"
#include "../core/structs.hpp"

extern Coordinator gCoordinator;
extern App *app;

class RenderSystem : public System {
private:
    // Helper function to draw a filled circle using SDL
    void drawFilledCircle(int centerX, int centerY, int radius, SDL_Color color) const {
        SDL_SetRenderDrawColor(app->renderer, color.r, color.g, color.b, color.a);

        for (int y = -radius; y <= radius; y++) {
            for (int x = -radius; x <= radius; x++) {
                if (x * x + y * y <= radius * radius) {
                    SDL_RenderDrawPoint(app->renderer, centerX + x, centerY + y);
                }
            }
        }
    }

    // Alternative method using SDL2_gfx if available
    void drawFilledCircleGFX(int centerX, int centerY, int radius, SDL_Color color) const {
        const int diameter = (radius * 2);
        float x = radius - 0.5f;
        float y = 0.5f;
        float tx = 1;
        float ty = 1;
        float error = tx - diameter;

        SDL_SetRenderDrawColor(app->renderer, color.r, color.g, color.b, color.a);

        while (x >= y) {
            // Each of the following renders a line from left to right
            SDL_RenderDrawLine(app->renderer, centerX - x, centerY - y, centerX + x, centerY - y);
            SDL_RenderDrawLine(app->renderer, centerX - x, centerY + y, centerX + x, centerY + y);
            SDL_RenderDrawLine(app->renderer, centerX - y, centerY - x, centerX + y, centerY - x);
            SDL_RenderDrawLine(app->renderer, centerX - y, centerY + x, centerX + y, centerY + x);

            if (error <= 0) {
                ++y;
                error += ty;
                ty += 2;
            }
            if (error > 0) {
                --x;
                tx += 2;
                error += (tx - diameter);
            }
        }
    }

    void drawDirectionArrow(float x, float y, float angle, SDL_Color color) const {
        // Convert angle to radians
        float radians = angle * M_PI / 180.0f;

        // Arrow properties
        const float arrowLength = 200.0f;
        const float arrowHeadLength = 20.0f;
        const float arrowHeadWidth = 15.0f;

        // Calculate end point
        float endX = x + cos(radians) * arrowLength;
        float endY = y + sin(radians) * arrowLength;

        // Calculate arrow head points
        float headAngle1 = radians + M_PI * 0.8f; // 144 degrees
        float headAngle2 = radians - M_PI * 0.8f; // -144 degrees

        float head1X = endX + cos(headAngle1) * arrowHeadLength;
        float head1Y = endY + sin(headAngle1) * arrowHeadLength;
        float head2X = endX + cos(headAngle2) * arrowHeadLength;
        float head2Y = endY + sin(headAngle2) * arrowHeadLength;

        // Draw arrow line
        SDL_SetRenderDrawColor(app->renderer, color.r, color.g, color.b, color.a);
        SDL_RenderDrawLineF(app->renderer, x, y, endX, endY);

        // Draw arrow head
        SDL_RenderDrawLineF(app->renderer, endX, endY, head1X, head1Y);
        SDL_RenderDrawLineF(app->renderer, endX, endY, head2X, head2Y);
    }

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

public:
    // Update method to handle rendering and camera control
    void update(const Entity camera) const {
        // Update camera based on the player's position
        Camera cameraComponent{0, 0, 0, 0, 0, 0};
        if (camera != INVALID_ENTITY) {
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

            if (gCoordinator.hasComponent<BubbleShooter>(entity)) {
                // Draw shooter as circle
                int radius = transform.w / 2;
                int centerX = tRect.x + radius;
                int centerY = tRect.y + radius;

                drawFilledCircle(centerX, centerY, radius, color.color);

                // Draw direction arrow
                auto &shooter = gCoordinator.getComponent<BubbleShooter>(entity);

                // Create contrasting color for arrow
                SDL_Color arrowColor = {
                    static_cast<Uint8>(255 - color.color.r),
                    static_cast<Uint8>(255 - color.color.g),
                    static_cast<Uint8>(255 - color.color.b),
                    255
                };

                drawDirectionArrow(centerX, centerY, shooter.currentAngle, arrowColor);

            } else if (gCoordinator.hasComponent<BubbleProjectile>(entity)) {
                // Draw bubbles as circles
                int radius = transform.w / 2;
                int centerX = tRect.x + radius;
                int centerY = tRect.y + radius;

                drawFilledCircle(centerX, centerY, radius, color.color);
            } else {
                // Draw other entities as rectangles
                SDL_SetRenderDrawColor(app->renderer, color.color.r, color.color.g, color.color.b, color.color.a);
                SDL_RenderDrawRectF(app->renderer, &tRect);
                SDL_RenderFillRectF(app->renderer, &tRect);
            }
        }
    }
};
