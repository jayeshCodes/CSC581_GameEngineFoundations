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
    static constexpr int ROWS = 12;
    static constexpr int COLS = 12;
    static constexpr float GRID_SIZE = 32.f;
    static constexpr float GRID_OFFSET_X = (SCREEN_WIDTH - (COLS * GRID_SIZE)) / 2.0f; // Center the grid
    static constexpr float GRID_OFFSET_Y = 32.0f;


    std::vector<std::vector<Entity> > grid;

    GridPosition worldToGrid(float x, float y) {
        // Adjust for grid offset
        float relativeX = x - GRID_OFFSET_X;
        float relativeY = y - GRID_OFFSET_Y;

        int col = static_cast<int>(std::round(relativeX / GRID_SIZE));
        int row = static_cast<int>(std::round(relativeY / GRID_SIZE));

        // Clamp values
        col = std::clamp(col, 0, COLS - 1);
        row = std::clamp(row, 0, ROWS - 1);

        std::cout << "World to Grid: (" << x << ", " << y << ") -> (" << row << ", " << col << ")" << std::endl;
        return {row, col};
    }

    SDL_FPoint gridToWorld(const GridPosition& pos) {
        return {
            GRID_OFFSET_X + (pos.col * GRID_SIZE),
            GRID_OFFSET_Y + (pos.row * GRID_SIZE)
        };
    }

    // Find empty spot in expanding radius
    GridPosition findEmptyPosition(const GridPosition& startPos) {
        // Check in expanding rings
        for (int radius = 1; radius <= 3; radius++) {  // Check up to 3 cells away
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
        return {-1, -1};  // No position found
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
        for (Entity bubble: bubbles) {
            auto &transform = gCoordinator.getComponent<Transform>(bubble);
            GridPosition pos = worldToGrid(transform.x, transform.y);
            grid[pos.row][pos.col] = INVALID_ENTITY;
            gCoordinator.addComponent<Destroy>(bubble, Destroy{});
            gCoordinator.getComponent<Destroy>(bubble).destroy = true;
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
        std::cout << "Grid dimensions: " << ROWS << "x" << COLS << std::endl;
        std::cout << "Grid offset: (" << GRID_OFFSET_X << ", " << GRID_OFFSET_Y << ")" << std::endl;
    }

    void addBubble(Entity entity) {
        try {
            auto& transform = gCoordinator.getComponent<Transform>(entity);

            // Get initial grid position
            GridPosition pos = worldToGrid(transform.x, transform.y);

            // Check if position is occupied
            if (grid[pos.row][pos.col] != INVALID_ENTITY) {
                GridPosition newPos = findEmptyPosition(pos);
                if (newPos.row >= 0) {  // Valid position found
                    pos = newPos;
                    // Update transform to match grid position
                    SDL_FPoint worldPos = gridToWorld(pos);
                    transform.x = worldPos.x;
                    transform.y = worldPos.y;
                } else {
                    std::cout << "No valid position found for bubble" << std::endl;
                    return;
                }
            }

            // Add to grid
            grid[pos.row][pos.col] = entity;

            std::cout << "Placed bubble at grid position: (" << pos.row << ", " << pos.col << ")" << std::endl;

            // Check for matches
            auto matches = findMatchingGroup(entity);
            if (matches.size() >= 3) {
                removeBubbles(matches);
                handleFloatingBubbles();
            }
        } catch (const std::exception& e) {
            std::cout << "Error in addBubble: " << e.what() << std::endl;
        }
    }
};

#endif //BUBBLE_GRID_HPP
