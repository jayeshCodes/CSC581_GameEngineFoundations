//
// Created by Jayesh Gajbhar on 11/17/24.
//

#ifndef GRID_GENERATOR_HPP
#define GRID_GENERATOR_HPP

#include "../ECS/system.hpp"
#include "../model/components.hpp"
#include "../ECS/coordinator.hpp"
#include "../helpers/colors.hpp"
#include "../EMS/event_coordinator.hpp"
#include "../model/event.hpp"
#include "../helpers/colors.hpp"
#include <random>
#include <queue>

extern Coordinator gCoordinator;
extern EventCoordinator eventCoordinator;


class BubbleGridGeneratorSystem : public System {
private:
    std::random_device rd; // Seed with random device
    std::mt19937 gen{rd()}; // Use Mersenne Twister engine
    static constexpr int ROWS = 24;
    static constexpr int COLS = 16;
    static constexpr float GRID_SIZE = 32.0f;

    void calculateGridParameters(GridGenerator &generator) {
        generator.horizontalSpacing = 32.0f;
        generator.verticalSpacing = 32.0f;

        float gridWidth = generator.horizontalSpacing * COLS;
        generator.startX = (SCREEN_WIDTH - gridWidth) / 2.0f;

        // Debug output
        std::cout << "Grid init - SCREEN_WIDTH: " << SCREEN_WIDTH
                << ", gridWidth: " << gridWidth
                << ", startX: " << generator.startX << std::endl;
    }

    SDL_Color getRandomColor(const GridGenerator &generator) {
        if (generator.availableColors.empty()) {
            return shade_color::getRandomBubbleColor();
        }
        std::uniform_int_distribution<> dis(0, generator.availableColors.size() - 1);
        return generator.availableColors[dis(gen)];
    }

    Entity createBubble(const GridGenerator& generator, int row, int col) {
        try {
            Entity bubble = gCoordinator.createEntity();

            float x = generator.startX + (col * generator.horizontalSpacing);
            if (row % 2 == 1) {
                x += generator.horizontalSpacing / 2.0f;
            }
            float y = generator.startY + (row * generator.verticalSpacing);

            gCoordinator.addComponent(bubble, Transform{x, y, 32.0f, 32.0f, 0.0f, 1.0f});
            gCoordinator.addComponent(bubble, Color{getRandomColor(generator)});
            gCoordinator.addComponent(bubble, CKinematic{{0, 0}, 0, {0, 0}, 0});
            gCoordinator.addComponent(bubble, ClientEntity{});
            gCoordinator.addComponent(bubble, Destroy{});
            gCoordinator.addComponent(bubble, Bubble{true, row, col, 16.0f});  // Add Bubble component

            // Register with grid system
            if (auto gridSystem = gCoordinator.getSystem<BubbleGridSystem>()) {
                gridSystem->initializeGridBubble(bubble, row, col);
            }

            return bubble;
        } catch (const std::exception& e) {
            std::cerr << "Error creating bubble: " << e.what() << std::endl;
            return INVALID_ENTITY;
        }
    }

    void generateRow(const GridGenerator &generator, int rowIndex) {
        for (int col = 0; col < COLS; col++) {
            Entity bubble = createBubble(generator, rowIndex, col);

            // Verify bubble was created successfully
            if (!gCoordinator.hasComponent<Transform>(bubble) ||
                !gCoordinator.hasComponent<Bubble>(bubble)) {
                std::cerr << "Failed to create bubble at row " << rowIndex
                        << ", col " << col << std::endl;
                continue;
            }
        }
    }

    void generateNewTopRow(const GridGenerator &generator) {
        generateRow(generator, 0);
    }

    void shiftRowsDown() {
        auto bubbles = gCoordinator.getEntitiesWithComponent<Bubble>();
        for (auto entity: bubbles) {
            if (!gCoordinator.hasComponent<Transform>(entity) ||
                !gCoordinator.hasComponent<Bubble>(entity)) {
                continue;
            }

            auto &bubble = gCoordinator.getComponent<Bubble>(entity);
            auto &transform = gCoordinator.getComponent<Transform>(entity);

            // Verify new position is within bounds
            float newY = transform.y + GRID_SIZE;
            if (newY < SCREEN_HEIGHT - GRID_SIZE) {
                transform.y = newY;
                bubble.row++;
            }
        }
    }

public:
    void initializeGrid(Entity generatorEntity) {
        auto &generator = gCoordinator.getComponent<GridGenerator>(generatorEntity);
        calculateGridParameters(generator);

        if (generator.availableColors.empty()) {
            generator.availableColors = {
                shade_color::Red, shade_color::Blue, shade_color::Green,
                shade_color::Yellow, shade_color::Purple
            };
        }

        for (int row = 0; row < generator.initialRows; row++) {
            generateRow(generator, row);
        }
    }

    void update() {
        for (auto entity: entities) {
            auto &generator = gCoordinator.getComponent<GridGenerator>(entity);
            if (generator.needsNewRow) {
                shiftRowsDown();
                generateNewTopRow(generator);
                generator.needsNewRow = false;
            }
        }
    }
};

#endif //GRID_GENERATOR_HPP
