//
// Created by Jayesh Gajbhar on 11/21/24.
//

#ifndef INSTRUCTION_RENDER_HPP
#define INSTRUCTION_RENDER_HPP

#include "../ECS/system.hpp"
#include "../model/components.hpp"
#include "../helpers/font_helper.hpp"
#include <string>
#include <vector>

extern Coordinator gCoordinator;

class InstructionDisplaySystem : public System {
private:
    const float BASE_X = SCREEN_WIDTH * 3 / 4.f - 70.f;
    const float BASE_Y = SCREEN_HEIGHT - 200.f;
    const float LINE_SPACING = 30.f; // Adjust this value to control spacing between lines

    void renderMultilineText(FontHelper *fontHelper, const std::vector<std::string> &lines, float startY) {
        for (size_t i = 0; i < lines.size(); i++) {
            float y = startY + (i * LINE_SPACING);
            fontHelper->renderText(
                lines[i],
                "game_font",
                SDL_Color{200, 200, 200, 200},
                BASE_X,
                y
            );
        }
    }

public:
    void update() {
        try {
            // Split instructions into separate lines
            std::vector<std::string> shootingInstructions = {
                "Controls:",
                "Mouse to aim",
                "Click to shoot"
            };

            // Control instructions
            std::vector<std::string> controlInstructions = {
                "Press P to pause the game",
                "Press R to reset the game"
            };

            auto *fontHelper = FontHelper::getInstance();
            if (fontHelper) {
                // Render shooting instructions
                renderMultilineText(fontHelper, shootingInstructions, BASE_Y);

                // Render control instructions
                renderMultilineText(fontHelper, controlInstructions,
                                    BASE_Y + (shootingInstructions.size() * LINE_SPACING) + 20.f);
            }
        } catch (const std::exception &e) {
            std::cerr << "Error rendering instructions: " << e.what() << std::endl;
        }
    }
};

#endif //INSTRUCTION_RENDER_HPP
