//
// Created by Jayesh Gajbhar on 11/17/24.
//

#ifndef SCORE_DISPLAY_HPP
#define SCORE_DISPLAY_HPP

#include "../ECS/system.hpp"
#include "../model/components.hpp"
#include "../helpers/font_helper.hpp"
#include <string>

extern Coordinator gCoordinator;

class ScoreDisplaySystem : public System {
private:
    static constexpr float SCORE_X = 50.0f;
    static constexpr float SCORE_Y = 50.0f;
    SDL_Color textColor = {255, 255, 255, 255};

public:
    void update() {
        try {
            for (auto entity: entities) {

                // Verify entity still exists
                auto ids = gCoordinator.getEntityIds();
                bool entityExists = false;
                for (const auto &[id, e]: ids) {
                    if (e == entity) {
                        entityExists = true;
                        break;
                    }
                }

                if (!entityExists) {
                    std::cerr << "Entity no longer exists: " << entity << std::endl;
                    continue;
                }

                if (!gCoordinator.hasComponent<Score>(entity)) {
                    std::cerr << "Score component not found on entity: " << entity << std::endl;
                    continue;
                }

                const auto &score = gCoordinator.getComponent<Score>(entity);

                // Draw score
                try {
                    // Create score text
                    std::string scoreText = "Score: " + std::to_string(score.value);
                    std::string multiplierText = "x" + std::to_string(score.multiplier);

                    auto *fontHelper = FontHelper::getInstance();
                    if (fontHelper) {
                        fontHelper->renderText(scoreText, "game_font", textColor, SCORE_X, SCORE_Y);
                        fontHelper->renderText(multiplierText, "game_font", textColor, SCORE_X, SCORE_Y + 30);
                    }
                } catch (const std::exception &e) {
                    std::cerr << "Error rendering score: " << e.what() << std::endl;
                }
            }
        } catch (const std::exception &e) {
            std::cerr << "Error in score display update: " << e.what() << std::endl;
        }
    }
};

#endif //SCORE_DISPLAY_HPP
