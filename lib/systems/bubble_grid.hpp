//
// Created by Jayesh Gajbhar on 11/16/24.
//

#ifndef BUBBLE_GRID_HPP
#define BUBBLE_GRID_HPP

#include "../ECS/system.hpp"
#include "../model/components.hpp"
#include "../ECS/coordinator.hpp"
#include <queue>
#include <set>

extern Coordinator gCoordinator;

struct GridPosition {
    int row;
    int col;

    bool operator==(const GridPosition &other) const {
        return row == other.row && col == other.col;
    }

    // Add hash function for std::set
    bool operator<(const GridPosition &other) const {
        if (row != other.row) return row < other.row;
        return col < other.col;
    }
};

class BubbleGridSystem : public System {
private:
    static constexpr int ROWS = 24;
    static constexpr int COLS = 16;
    static constexpr float GRID_SIZE = 32.f;
    static constexpr float GRID_OFFSET_X = (SCREEN_WIDTH - (COLS * GRID_SIZE)) / 2.0f; // Center the grid
    static constexpr float GRID_OFFSET_Y = 32.0f;


    std::vector<std::vector<Entity> > grid;

    bool isPositionOccupied(float x, float y) {
        auto bubbles = gCoordinator.getEntitiesWithComponent<Bubble>();
        for (auto entity: bubbles) {
            auto &transform = gCoordinator.getComponent<Transform>(entity);
            float distance = std::sqrt(
                std::pow(transform.x - x, 2) +
                std::pow(transform.y - y, 2)
            );

            // If distance is less than bubble diameter, position is occupied
            if (distance < GRID_SIZE) {
                return true;
            }
        }
        return false;
    }

    SDL_FPoint findNearestEmptyPosition(float x, float y) {
        // Check positions in expanding circles
        for (float radius = GRID_SIZE; radius <= GRID_SIZE * 3; radius += GRID_SIZE / 2) {
            for (float angle = 0; angle < 2 * M_PI; angle += M_PI / 4) {
                float newX = x + radius * std::cos(angle);
                float newY = y + radius * std::sin(angle);

                // Ensure position is within grid bounds
                if (newX >= GRID_OFFSET_X &&
                    newX < GRID_OFFSET_X + (COLS * GRID_SIZE) &&
                    newY >= GRID_OFFSET_Y &&
                    newY < GRID_OFFSET_Y + (ROWS * GRID_SIZE)) {
                    if (!isPositionOccupied(newX, newY)) {
                        return {newX, newY};
                    }
                }
            }
        }
        return {x, y}; // Return original if no empty position found
    }

    bool checkCollision(float x, float y, float &snapX, float &snapY) {
        auto bubbles = gCoordinator.getEntitiesWithComponent<Bubble>();
        float closestDistance = std::numeric_limits<float>::max();
        bool collision = false;

        for (auto entity: bubbles) {
            auto &transform = gCoordinator.getComponent<Transform>(entity);
            float distance = std::sqrt(
                std::pow(transform.x - x, 2) +
                std::pow(transform.y - y, 2)
            );

            if (distance <= GRID_SIZE * 1.1f) {
                // Slightly larger than bubble size for better snapping
                collision = true;
                if (distance < closestDistance) {
                    closestDistance = distance;

                    // Calculate snap position adjacent to colliding bubble
                    float angle = std::atan2(y - transform.y, x - transform.x);
                    snapX = transform.x + GRID_SIZE * std::cos(angle);
                    snapY = transform.y + GRID_SIZE * std::sin(angle);
                }
            }
        }

        return collision;
    }

    GridPosition worldToGrid(float x, float y) {
        // Adjust for grid offset
        float relativeX = x - GRID_OFFSET_X;
        float relativeY = y - GRID_OFFSET_Y;

        int col = static_cast<int>(std::round(relativeX / GRID_SIZE));
        int row = static_cast<int>(std::round(relativeY / GRID_SIZE));

        // Clamp values
        col = std::clamp(col, 0, COLS - 1);
        row = std::clamp(row, 0, ROWS - 1);

        // std::cout << "World to Grid: (" << x << ", " << y << ") -> (" << row << ", " << col << ")" << std::endl;
        return {row, col};
    }

    SDL_FPoint gridToWorld(const GridPosition &pos) {
        return {
            GRID_OFFSET_X + (pos.col * GRID_SIZE),
            GRID_OFFSET_Y + (pos.row * GRID_SIZE)
        };
    }

    // Find empty spot in expanding radius
    GridPosition findEmptyPosition(const GridPosition &startPos) {
        // Check in expanding rings
        for (int radius = 1; radius <= 3; radius++) {
            // Check up to 3 cells away
            for (int dr = -radius; dr <= radius; dr++) {
                for (int dc = -radius; dc <= radius; dc++) {
                    // Only check positions exactly 'radius' cells away
                    if (std::abs(dr) + std::abs(dc) == radius) {
                        int newRow = startPos.row + dr;
                        int newCol = startPos.col + dc;

                        if (newRow >= 0 && newRow < ROWS &&
                            newCol >= 0 && newCol < COLS &&
                            grid[newRow][newCol] == INVALID_ENTITY) {
                            return {newRow, newCol};
                        }
                    }
                }
            }
        }
        return {-1, -1}; // No position found
    }

    std::vector<GridPosition> getNeighbors(int row, int col) {
        std::vector<GridPosition> neighbors;

        // Even rows
        if (row % 2 == 0) {
            neighbors.push_back({row - 1, col - 1}); // Top left
            neighbors.push_back({row - 1, col}); // Top right
            neighbors.push_back({row, col - 1}); // Left
            neighbors.push_back({row, col + 1}); // Right
            neighbors.push_back({row + 1, col - 1}); // Bottom left
            neighbors.push_back({row + 1, col}); // Bottom right
        }
        // Odd rows
        else {
            neighbors.push_back({row - 1, col}); // Top left
            neighbors.push_back({row - 1, col + 1}); // Top right
            neighbors.push_back({row, col - 1}); // Left
            neighbors.push_back({row, col + 1}); // Right
            neighbors.push_back({row + 1, col}); // Bottom left
            neighbors.push_back({row + 1, col + 1}); // Bottom right
        }

        // Remove invalid neighbors
        std::vector<GridPosition> validNeighbors;
        for (auto &neighbor: neighbors) {
            if (neighbor.row >= 0 && neighbor.row < ROWS && neighbor.col >= 0 && neighbor.col < COLS) {
                validNeighbors.push_back(neighbor);
            }
        }

        return validNeighbors;
    }

    std::vector<Entity> findMatchingGroup(Entity bubble) {
        auto &color = gCoordinator.getComponent<Color>(bubble);
        auto &transform = gCoordinator.getComponent<Transform>(bubble);

        GridPosition pos = worldToGrid(transform.x, transform.y);
        std::vector<Entity> matches;
        std::set<GridPosition, std::less<GridPosition> > visited; // Specify GridPosition type
        std::queue<GridPosition> toVisit;

        toVisit.push(pos);
        visited.insert(pos);

        while (!toVisit.empty()) {
            GridPosition current = toVisit.front();
            toVisit.pop();

            Entity currentBubble = grid[current.row][current.col];
            if (currentBubble == INVALID_ENTITY) {
                continue;
            }

            auto &currentColor = gCoordinator.getComponent<Color>(currentBubble);
            if (currentColor.color.r == color.color.r &&
                currentColor.color.g == color.color.g &&
                currentColor.color.b == color.color.b) {
                matches.push_back(currentBubble);
                auto neighbors = getNeighbors(current.row, current.col);
                for (auto &neighbor: neighbors) {
                    if (visited.find(neighbor) == visited.end()) {
                        toVisit.push(neighbor);
                        visited.insert(neighbor);
                    }
                }
            }
        }

        return matches;
    }

    void removeBubbles(const std::vector<Entity> &bubbles) {
        try {
            // Update score before removing bubbles
            updateScore(bubbles);

            // Then remove the bubbles
            for (Entity bubble: bubbles) {
                auto &transform = gCoordinator.getComponent<Transform>(bubble);
                GridPosition pos = worldToGrid(transform.x, transform.y);
                grid[pos.row][pos.col] = INVALID_ENTITY;
                gCoordinator.addComponent<Destroy>(bubble, Destroy{});
                gCoordinator.getComponent<Destroy>(bubble).destroy = true;
            }

            // Handle floating bubbles after removal
            handleFloatingBubbles();
        } catch (const std::exception &e) {
            std::cerr << "Error removing bubbles: " << e.what() << std::endl;
        }
    }

    void handleFloatingBubbles() {
        std::set<Entity> anchored;
        std::set<GridPosition, std::less<GridPosition> > checked; // Specify GridPosition type

        // Find all the bubbles connected to the top row
        for (int col = 0; col < COLS; ++col) {
            if (grid[0][col] != INVALID_ENTITY) {
                findAnchoredBubbles(0, col, anchored, checked);
            }
        }

        // Remove all the unanchored bubbles
        for (int row = 0; row < ROWS; ++row) {
            for (int col = 0; col < COLS; ++col) {
                Entity bubble = grid[row][col];
                if (bubble != INVALID_ENTITY && anchored.find(bubble) == anchored.end()) {
                    gCoordinator.addComponent<Destroy>(bubble, Destroy{});
                    gCoordinator.getComponent<Destroy>(bubble).destroy = true;
                    grid[row][col] = INVALID_ENTITY;
                }
            }
        }
    }

    void findAnchoredBubbles(int row, int col, std::set<Entity> &anchored,
                             std::set<GridPosition, std::less<GridPosition> > &checked) {
        // Update parameter type
        GridPosition pos{row, col};
        if (checked.find(pos) != checked.end()) {
            return;
        }

        checked.insert(pos);

        Entity bubble = grid[row][col];
        if (bubble == INVALID_ENTITY) {
            return;
        }

        anchored.insert(bubble);

        for (const auto &neighbor: getNeighbors(row, col)) {
            findAnchoredBubbles(neighbor.row, neighbor.col, anchored, checked);
        }
    }

    GridPosition findNearestEmptyPosition(const GridPosition &startPos) {
        // Check in expanding circles around the target position
        for (int radius = 1; radius < std::max(ROWS, COLS); radius++) {
            // Check positions in a diamond pattern
            for (int dy = -radius; dy <= radius; dy++) {
                for (int dx = -radius; dx <= radius; dx++) {
                    // Only check positions that are exactly 'radius' steps away
                    if (std::abs(dx) + std::abs(dy) == radius) {
                        int newRow = startPos.row + dy;
                        int newCol = startPos.col + dx;

                        // Check if position is valid and empty
                        if (newRow >= 0 && newRow < ROWS &&
                            newCol >= 0 && newCol < COLS &&
                            grid[newRow][newCol] == INVALID_ENTITY) {
                            return {newRow, newCol};
                        }
                    }
                }
            }
        }
        return {-1, -1}; // No empty position found
    }

public:
    BubbleGridSystem() {
        grid.resize(ROWS, std::vector<Entity>(COLS, INVALID_ENTITY));
        // std::cout << "Grid dimensions: " << ROWS << "x" << COLS << std::endl;
        // std::cout << "Grid offset: (" << GRID_OFFSET_X << ", " << GRID_OFFSET_Y << ")" << std::endl;
    }

    void initializeGridBubble(Entity entity, int row, int col) {
        try {
            auto& transform = gCoordinator.getComponent<Transform>(entity);
            grid[row][col] = entity;

            // Ensure bubble has Bubble component with correct grid position
            if (!gCoordinator.hasComponent<Bubble>(entity)) {
                gCoordinator.addComponent(entity, Bubble{true, row, col, GRID_SIZE/2});
            }

            std::cout << "Initialized grid bubble at (" << row << "," << col
                      << ") with entity " << entity << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error initializing grid bubble: " << e.what() << std::endl;
        }
    }

    void addBubble(Entity entity) {
        try {
            auto &transform = gCoordinator.getComponent<Transform>(entity);
            float snapX = transform.x;
            float snapY = transform.y;

            if (checkCollision(transform.x, transform.y, snapX, snapY)) {
                // Snap to grid position nearest to collision point
                transform.x = snapX;
                transform.y = snapY;
            } else if (transform.y <= GRID_OFFSET_Y) {
                // Only snap to top if no collision occurred
                transform.y = GRID_OFFSET_Y;
                float relativeX = transform.x - GRID_OFFSET_X;
                transform.x = std::round(relativeX / GRID_SIZE) * GRID_SIZE + GRID_OFFSET_X;
            }

            GridPosition pos = worldToGrid(transform.x, transform.y);
            if (pos.row >= 0 && pos.row < ROWS && pos.col >= 0 && pos.col < COLS) {
                grid[pos.row][pos.col] = entity;

                if (!gCoordinator.hasComponent<Bubble>(entity)) {
                    gCoordinator.addComponent(entity, Bubble{true, pos.row, pos.col});
                }

                auto matches = findMatchingGroup(entity);
                if (matches.size() >= 3) {
                    removeBubbles(matches);
                    handleFloatingBubbles();
                }
            }
        } catch (const std::exception &e) {
            std::cerr << "Error in addBubble: " << e.what() << std::endl;
        }
    }


    // Modify BubbleGridSystem to include scoring
    void updateScore(const std::vector<Entity> &matches) {
        try {
            auto scoreEntities = gCoordinator.getEntitiesWithComponent<Score>();
            if (scoreEntities.empty()) {
                std::cerr << "No score entities found" << std::endl;
                return;
            }

            Entity scoreEntity = scoreEntities[0];
            auto &score = gCoordinator.getComponent<Score>(scoreEntity);

            // Calculate score for this match
            int matchSize = static_cast<int>(matches.size());
            if (matchSize >= 3) {
                // Base score calculation
                int baseScore = score.bubbleScore * matchSize;

                // Bonus points for matches larger than 3
                int comboBonus = 0;
                if (matchSize > 3) {
                    comboBonus = (matchSize - 3) * score.comboBonus;
                }

                // Multiplier logic - increases with larger matches
                // For example: 3 bubbles = 1x, 4 bubbles = 1.5x, 5 bubbles = 2x, etc.
                float matchMultiplier = 1.0f + (matchSize - 3) * 0.5f;

                int newScore = static_cast<int>((baseScore + comboBonus) * matchMultiplier);

                score.value += newScore;
            }
        } catch (const std::exception &e) {
            std::cerr << "Error updating score: " << e.what() << std::endl;
        }
    }
};

#endif //BUBBLE_GRID_HPP
