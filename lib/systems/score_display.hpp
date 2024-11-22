//
// Created by Jayesh Gajbhar on 11/22/24.
//

#ifndef SCORE_DISPLAY_HPP
#define SCORE_DISPLAY_HPP

#include "../ECS/coordinator.hpp"
#include "../EMS/event_coordinator.hpp"
#include "../ECS/system.hpp"
#include "../helpers/font_helper.hpp"
#include "../model/components.hpp"
#include "../core/structs.hpp"
#include "../core/defs.hpp"
#include "../model/event.hpp"
#include <mutex>

extern Coordinator gCoordinator;
extern EventCoordinator eventCoordinator;

class ScoreDisplaySystem : public System {
private:
    FontHelper *fontHelper;

public:
    ScoreDisplaySystem() {
        fontHelper = FontHelper::getInstance();
        fontHelper->loadFont("score", "assets/fonts/Dokdo-Regular.ttf", 24);
    }

    void update() {
        for (auto &entity: entities) {
            auto &score = gCoordinator.getComponent<Score>(entity);

            std::string scoreText = "Score: " + std::to_string(static_cast<int>(score.score));

            int textX = (SCREEN_WIDTH / 2) - 50;
            int textY = 20;

            fontHelper->renderText(scoreText, "score" , SDL_Color{0, 0, 0, 255}, textX, textY);
        }
    }
};

#endif //SCORE_DISPLAY_HPP
