//
// Created by Utsav Lal on 10/2/24.
//

#include <SDL_render.h>

#include "../components/components.hpp"
#include "../ECS/coordinator.hpp"
#include "../ECS/system.hpp"
#include "../objects/shapes/rectangle.hpp"

extern Coordinator gCoordinator;

class RenderSystem : public System {
public:
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
};
