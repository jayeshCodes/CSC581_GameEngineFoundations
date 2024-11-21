// RenderSystem.hpp

#include <SDL.h>
#include "../ECS/coordinator.hpp"
#include "../ECS/system.hpp"
#include "../model/components.hpp"
#include "../core/structs.hpp"
#include "../helpers/colors.hpp"

extern Coordinator gCoordinator;
extern App *app;

class RenderSystem : public System {
private:
    // Helper function to draw a circle outline using SDL
    void drawCircleOutline(int centerX, int centerY, int radius, SDL_Color color) const {
        SDL_SetRenderDrawColor(app->renderer, color.r, color.g, color.b, color.a);

        int x = radius - 1;
        int y = 0;
        int dx = 1;
        int dy = 1;
        int err = dx - (radius << 1);

        while (x >= y) {
            SDL_RenderDrawPoint(app->renderer, centerX + x, centerY + y);
            SDL_RenderDrawPoint(app->renderer, centerX + y, centerY + x);
            SDL_RenderDrawPoint(app->renderer, centerX - y, centerY + x);
            SDL_RenderDrawPoint(app->renderer, centerX - x, centerY + y);
            SDL_RenderDrawPoint(app->renderer, centerX - x, centerY - y);
            SDL_RenderDrawPoint(app->renderer, centerX - y, centerY - x);
            SDL_RenderDrawPoint(app->renderer, centerX + y, centerY - x);
            SDL_RenderDrawPoint(app->renderer, centerX + x, centerY - y);

            if (err <= 0) {
                y++;
                err += dy;
                dy += 2;
            }
            if (err > 0) {
                x--;
                dx += 2;
                err += dx - (radius << 1);
            }
        }
    }

    // Helper function to draw a filled circle with outline using SDL
    void drawFilledCircle(int centerX, int centerY, int radius, SDL_Color fillColor) const {
        // Draw filled circle
        SDL_SetRenderDrawColor(app->renderer, fillColor.r, fillColor.g, fillColor.b, fillColor.a);

        for (int y = -radius; y <= radius; y++) {
            for (int x = -radius; x <= radius; x++) {
                if (x * x + y * y <= radius * radius) {
                    SDL_RenderDrawPoint(app->renderer, centerX + x, centerY + y);
                }
            }
        }

        // Create darker outline color
        SDL_Color outlineColor = {
            static_cast<Uint8>(fillColor.r * 0.7),
            static_cast<Uint8>(fillColor.g * 0.7),
            static_cast<Uint8>(fillColor.b * 0.7),
            fillColor.a
        };

        // Draw outline
        drawCircleOutline(centerX, centerY, radius, outlineColor);
    }

    // Alternative method using SDL2_gfx if available
    void drawFilledCircleGFX(int centerX, int centerY, int radius, SDL_Color fillColor) const {
        const int diameter = (radius * 2);
        float x = radius - 0.5f;
        float y = 0.5f;
        float tx = 1;
        float ty = 1;
        float error = tx - diameter;

        // Draw filled circle
        SDL_SetRenderDrawColor(app->renderer, fillColor.r, fillColor.g, fillColor.b, fillColor.a);

        while (x >= y) {
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

        // Create darker outline color
        SDL_Color outlineColor = {
            static_cast<Uint8>(fillColor.r * 0.7),
            static_cast<Uint8>(fillColor.g * 0.7),
            static_cast<Uint8>(fillColor.b * 0.7),
            fillColor.a
        };

        // Draw outline
        drawCircleOutline(centerX, centerY, radius, outlineColor);
    }

    void drawDirectionArrow(float x, float y, float angle, SDL_Color color, const SDL_FPoint &mousePos) const {
        // Convert angle to radians
        float radians = angle * M_PI / 180.0f;

        // Calculate distance to mouse
        float dx = mousePos.x - x;
        float dy = mousePos.y - y;
        float distance = std::sqrt(dx * dx + dy * dy);

        // Set a minimum and maximum arrow length
        float minArrowLength = 50.0f;
        float maxArrowLength = 300.0f;

        // Scale the arrow length based on the distance to the mouse
        float arrowLength = std::clamp(distance, minArrowLength, maxArrowLength);

        // Calculate end point
        float endX = x + cos(radians) * arrowLength;
        float endY = y + sin(radians) * arrowLength;

        // Calculate arrow head points
        float headAngle1 = radians + M_PI * 0.8f; // 144 degrees
        float headAngle2 = radians - M_PI * 0.8f; // -144 degrees

        float head1X = endX + cos(headAngle1) * (arrowLength * 0.1f);
        float head1Y = endY + sin(headAngle1) * (arrowLength * 0.1f);
        float head2X = endX + cos(headAngle2) * (arrowLength * 0.1f);
        float head2Y = endY + sin(headAngle2) * (arrowLength * 0.1f);

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
        if (camera != INVALID_ENTITY && gCoordinator.hasComponent<Camera>(camera)) {
            cameraComponent = gCoordinator.getComponent<Camera>(camera);
        }

        // Create a safe copy of entities to iterate over
        std::vector<Entity> currentEntities(entities.begin(), entities.end());

        // Loop through all entities to render them
        for (const Entity entity: entities) {
            try {
                if (!gCoordinator.hasComponent<Transform>(entity) ||
                    !gCoordinator.hasComponent<Color>(entity)) {
                    continue;
                }
                const auto &transform = gCoordinator.getComponent<Transform>(entity);
                const auto &color = gCoordinator.getComponent<Color>(entity);

                // Convert world coordinates to screen coordinates
                SDL_FRect tRect = {
                    transform.x - cameraComponent.x,
                    transform.y,
                    transform.w,
                    transform.h
                };

                if (gCoordinator.hasComponent<BubbleShooter>(entity)) {
                    const auto &shooter = gCoordinator.getComponent<BubbleShooter>(entity);
                    // Draw shooter as circle with outline
                    int radius = transform.w / 2;
                    int centerX = tRect.x + radius;
                    int centerY = tRect.y + radius;

                    drawFilledCircleGFX(centerX, centerY, radius, color.color);

                    // Draw direction arrow
                    SDL_Color arrowColor = {
                        static_cast<Uint8>(255 - color.color.r),
                        static_cast<Uint8>(255 - color.color.g),
                        static_cast<Uint8>(255 - color.color.b),
                        255
                    };
                    int mouseX, mouseY;
                    Uint32 mouseState = SDL_GetMouseState(&mouseX, &mouseY);
                    SDL_FPoint mousePos = {static_cast<float>(mouseX), static_cast<float>(mouseY)};

                    drawDirectionArrow(centerX, centerY, shooter.currentAngle, arrowColor, mousePos);
                } else if (gCoordinator.hasComponent<BubbleProjectile>(entity) || gCoordinator.hasComponent<
                               Bubble>(entity)) {
                    // Draw bubbles as circles with outlines
                    int radius = transform.w / 2;
                    int centerX = tRect.x + radius;
                    int centerY = tRect.y + radius;

                    drawFilledCircle(centerX, centerY, radius, color.color);
                    drawCircleOutline(centerX, centerY, radius, shade_color::Black);
                } else {
                    // Draw other entities as rectangles
                    SDL_SetRenderDrawColor(app->renderer, color.color.r, color.color.g, color.color.b, color.color.a);
                    SDL_RenderDrawRectF(app->renderer, &tRect);
                    SDL_RenderFillRectF(app->renderer, &tRect);
                }
            } catch (const std::exception &e) {
                std::cerr << "Error rendering entity: " << e.what() << std::endl;
                continue;
            }
        }
    }
};
