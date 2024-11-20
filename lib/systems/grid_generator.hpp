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
#include "../helpers/constants.hpp"

extern Coordinator gCoordinator;
extern EventCoordinator eventCoordinator;


class BubbleGridGeneratorSystem : public System {
private:
    std::random_device rd; // Seed with random device
    std::mt19937 gen{rd()}; // Use Mersenne Twister engine
    static constexpr int ROWS = 24;
    static constexpr int COLS = 15; // Reduced by 1 to prevent overflow
    static constexpr int MAX_COLOR_REPEATS = 2;

    struct ColorCount {
        SDL_Color color;
        int count;

        bool operator==(const SDL_Color &other) const {
            return color.r == other.r &&
                   color.g == other.g &&
                   color.b == other.b;
        }
    };

    struct GridColorState {
        std::vector<std::vector<SDL_Color> > colorGrid;

        void ensureSize(int row, int col) {
            if (row >= colorGrid.size()) {
                colorGrid.resize(row + 1);
            }
            if (col >= colorGrid[row].size()) {
                colorGrid[row].resize(col + 1);
            }
        }

        void addColor(int row, int col, SDL_Color color) {
            ensureSize(row, col);
            colorGrid[row][col] = color;
        }

        std::vector<SDL_Color> getVerticalColors(int row, int col, int checkRange = 2) {
            std::vector<SDL_Color> colors;
            for (int i = std::max(0, row - checkRange); i <= row; i++) {
                if (i < colorGrid.size() && col < colorGrid[i].size()) {
                    colors.push_back(colorGrid[i][col]);
                }
            }
            return colors;
        }
    };

    GridColorState colorState;

    SDL_Color getColorForPosition(const GridGenerator &generator, int row, int col,
                                  std::vector<ColorCount> &rowColors) {
        // Initialize color counts for this row if empty
        if (rowColors.empty() && !generator.availableColors.empty()) {
            for (const auto &color: generator.availableColors) {
                rowColors.push_back({color, 0});
            }
        }

        // Get vertical colors for this position
        auto verticalColors = colorState.getVerticalColors(row, col);

        // Filter available colors based on both horizontal and vertical constraints
        std::vector<SDL_Color> validColors;
        for (const auto &colorCount: rowColors) {
            if (colorCount.count >= MAX_COLOR_REPEATS) continue;

            // Count occurrences in vertical direction
            int verticalCount = 0;
            for (const auto &vColor: verticalColors) {
                if (colorCount.color.r == vColor.r &&
                    colorCount.color.g == vColor.g &&
                    colorCount.color.b == vColor.b) {
                    verticalCount++;
                }
            }

            // Add color only if it doesn't exceed limits in either direction
            if (verticalCount < MAX_COLOR_REPEATS) {
                validColors.push_back(colorCount.color);
            }
        }

        // If no valid colors, reset horizontal counts and try again
        if (validColors.empty()) {
            for (auto &colorCount: rowColors) {
                colorCount.count = 0;
            }
            validColors = generator.availableColors;

            // Filter again based on vertical constraints
            std::vector<SDL_Color> tempValidColors;
            for (const auto &color: validColors) {
                int verticalCount = 0;
                for (const auto &vColor: verticalColors) {
                    if (color.r == vColor.r && color.g == vColor.g && color.b == vColor.b) {
                        verticalCount++;
                    }
                }
                if (verticalCount < MAX_COLOR_REPEATS) {
                    tempValidColors.push_back(color);
                }
            }

            if (!tempValidColors.empty()) {
                validColors = tempValidColors;
            }
        }

        // Select random color from valid colors
        std::uniform_int_distribution<> dis(0, validColors.size() - 1);
        SDL_Color selectedColor = validColors[dis(gen)];

        // Update counts and state
        for (auto &colorCount: rowColors) {
            if (colorCount == selectedColor) {
                colorCount.count++;
                break;
            }
        }

        colorState.addColor(row, col, selectedColor);
        return selectedColor;
    }

    void calculateGridParameters(GridGenerator &generator) {
        generator.horizontalSpacing = bubble_constants::GRID_SIZE;
        generator.verticalSpacing = bubble_constants::GRID_SIZE;
        generator.startX = bubble_constants::GRID_OFFSET_X;

        // Validate grid boundaries
        float totalWidth = (COLS - 1) * generator.horizontalSpacing;
        if (generator.startX + totalWidth > SCREEN_WIDTH) {
            std::cerr << "Warning: Grid might exceed screen boundaries!" << std::endl;
        }
    }

    SDL_Color getRandomColor(const GridGenerator &generator) {
        if (generator.availableColors.empty()) {
            return shade_color::getRandomBubbleColor();
        }
        std::uniform_int_distribution<> dis(0, generator.availableColors.size() - 1);
        return generator.availableColors[dis(gen)];
    }

    Entity createBubble(const GridGenerator &generator, int row, int col) {
        try {
            if (col >= COLS) {
                std::cerr << "Attempted to create bubble outside grid bounds" << std::endl;
                return INVALID_ENTITY;
            }

            Entity bubble = gCoordinator.createEntity();

            // Calculate position with proper offset and boundary checking
            float x = generator.startX + (col * generator.horizontalSpacing);
            if (row % 2 == 1) {
                // For odd rows, check if shifted bubble would exceed boundary
                if (col == COLS - 1) {
                    // Skip last bubble in odd rows to prevent overflow
                    return INVALID_ENTITY;
                }
                x += generator.horizontalSpacing / 2.0f;
            }

            // Validate x position
            if (x + bubble_constants::GRID_SIZE > SCREEN_WIDTH) {
                std::cerr << "Bubble position would exceed screen width" << std::endl;
                return INVALID_ENTITY;
            }

            float y = generator.startY + (row * generator.verticalSpacing);

            gCoordinator.addComponent(bubble, Transform{
                                          x, y, bubble_constants::GRID_SIZE, bubble_constants::GRID_SIZE, 0.0f, 1.0f
                                      });
            gCoordinator.addComponent(bubble, Color{getRandomColor(generator)});
            gCoordinator.addComponent(bubble, CKinematic{{0, 0}, 0, {0, 0}, 0});
            gCoordinator.addComponent(bubble, ClientEntity{});
            gCoordinator.addComponent(bubble, Destroy{});
            gCoordinator.addComponent(bubble, Bubble{true, row, col, bubble_constants::GRID_SIZE / 2.0f});

            if (auto gridSystem = gCoordinator.getSystem<BubbleGridSystem>()) {
                gridSystem->initializeGridBubble(bubble, row, col);
            }

            return bubble;
        } catch (const std::exception &e) {
            std::cerr << "Error creating bubble: " << e.what() << std::endl;
            return INVALID_ENTITY;
        }
    }

    void generateRow(const GridGenerator &generator, int rowIndex) {
        std::vector<ColorCount> rowColors;
        int maxCols = (rowIndex % 2 == 1) ? COLS - 1 : COLS;

        for (int col = 0; col < maxCols; col++) {
            if (col >= COLS) continue;

            Entity bubble = gCoordinator.createEntity();
            float x = generator.startX + (col * generator.horizontalSpacing);
            if (rowIndex % 2 == 1) {
                x += generator.horizontalSpacing / 2.0f;
            }

            if (x + bubble_constants::GRID_SIZE > SCREEN_WIDTH) continue;

            float y = generator.startY + (rowIndex * generator.verticalSpacing);

            try {
                // Get color with both horizontal and vertical distribution control
                SDL_Color bubbleColor = getColorForPosition(generator, rowIndex, col, rowColors);

                gCoordinator.addComponent(bubble, Transform{
                                              x, y, bubble_constants::GRID_SIZE, bubble_constants::GRID_SIZE,
                                              0.0f, 1.0f
                                          });
                gCoordinator.addComponent(bubble, Color{bubbleColor});
                gCoordinator.addComponent(bubble, CKinematic{{0, 0}, 0, {0, 0}, 0});
                gCoordinator.addComponent(bubble, ClientEntity{});
                gCoordinator.addComponent(bubble, Destroy{});
                gCoordinator.addComponent(bubble, Bubble{
                                              true, rowIndex, col, bubble_constants::GRID_SIZE / 2.0f
                                          });

                if (auto gridSystem = gCoordinator.getSystem<BubbleGridSystem>()) {
                    gridSystem->initializeGridBubble(bubble, rowIndex, col);
                }
            } catch (const std::exception &e) {
                std::cerr << "Error creating bubble: " << e.what() << std::endl;
                continue;
            }
        }
    }

    void generateNewTopRow(const GridGenerator &generator) {
        // Shift color state down
        for (auto &row: colorState.colorGrid) {
            row.clear();
        }
        colorState.colorGrid.insert(colorState.colorGrid.begin(), std::vector<SDL_Color>());
        if (colorState.colorGrid.size() > ROWS) {
            colorState.colorGrid.pop_back();
        }

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
            float newY = transform.y + bubble_constants::GRID_SIZE;
            if (newY < SCREEN_HEIGHT - bubble_constants::GRID_SIZE) {
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
                shade_color::Red,
                shade_color::Blue,
                shade_color::Green,
                shade_color::Yellow,
                shade_color::Purple
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
