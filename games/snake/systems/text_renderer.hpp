//
// Created by Utsav Lal on 11/17/24.
//

#pragma once

#include "../../../main.hpp"
#include "../../../lib/ECS/coordinator.hpp"
#include "../../../lib/ECS/system.hpp"
#include "../model/components.hpp"

extern Coordinator gCoordinator;

class TextRenderer : public System {
public:
    TextRenderer() {
    }

    ~TextRenderer() {

    }

    void update() const {
        for (auto &entity: entities) {
            if (!gCoordinator.hasComponent<Transform>(entity)) return;
            if (!gCoordinator.hasComponent<Text>(entity)) return;
            const auto &text = gCoordinator.getComponent<Text>(entity);

            SDL_RenderCopy(app->renderer, text.font, nullptr, &text.rect);
        }
    }
};
