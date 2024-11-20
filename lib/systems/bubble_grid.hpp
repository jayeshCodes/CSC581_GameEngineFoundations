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
    static constexpr int MAX_ROWS = 24;
    static constexpr int COLS = 15;
    static constexpr float GRID_SIZE = 32.0f;
    static constexpr float GRID_OFFSET_X = (SCREEN_WIDTH - (COLS * GRID_SIZE)) / 2.0f;
    static constexpr float GRID_OFFSET_Y = 32.0f;


    int currentMaxRow = 0; // Track highest occupied row
    std::vector<std::vector<Entity> > grid;

    void updateGridSize(int newRow) {
        currentMaxRow = std::max(currentMaxRow, newRow);
        if (currentMaxRow >= grid.size()) {
            grid.resize(currentMaxRow + 1, std::vector<Entity>(COLS, INVALID_ENTITY));
        }
    }

    bool isValidGridPosition(int row, int col) const {
        return row >= 0 && row < MAX_ROWS && col >= 0 && col < COLS &&
               row < grid.size();
    }

    void ensureGridSize(int row) {
        if (row >= grid.size()) {
            grid.resize(row + 1, std::vector<Entity>(COLS, INVALID_ENTITY));
        }
    }

    void updateGridCoordinates() {
        auto bubbles = gCoordinator.getEntitiesWithComponent<Bubble>();
        for (auto entity: bubbles) {
            if (!gCoordinator.hasComponent<Transform>(entity)) continue;

            auto &transform = gCoordinator.getComponent<Transform>(entity);
            auto &bubble = gCoordinator.getComponent<Bubble>(entity);

            // Update grid position based on world coordinates
            GridPosition newPos = worldToGrid(transform.x, transform.y);
            if (newPos.row >= 0 && newPos.row < MAX_ROWS &&
                newPos.col >= 0 && newPos.col < COLS) {
                // Update grid array
                if (newPos.row >= grid.size()) {
                    grid.resize(newPos.row + 1, std::vector<Entity>(COLS, INVALID_ENTITY));
                }
                // Remove from old position if exists
                if (bubble.row < grid.size() && bubble.col < COLS) {
                    grid[bubble.row][bubble.col] = INVALID_ENTITY;
                }
                // Update to new position
                grid[newPos.row][newPos.col] = entity;
                bubble.row = newPos.row;
                bubble.col = newPos.col;
            }
        }
    }

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
                    newY < GRID_OFFSET_Y + (MAX_ROWS * GRID_SIZE)) {
                    if (!isPositionOccupied(newX, newY)) {
                        return {newX, newY};
                    }
                }
            }
        }
        return {x, y}; // Return original if no empty position found
    }

    bool checkCollision(float x, float y, float &snapX, float &snapY) {
        std::cout << "\nChecking collisions for position x: " << x << ", y: " << y << std::endl;

        auto bubbles = gCoordinator.getEntitiesWithComponent<Bubble>();
        float closestDistance = std::numeric_limits<float>::max();
        bool collision = false;

        for (auto entity: bubbles) {
            auto &transform = gCoordinator.getComponent<Transform>(entity);
            float distance = std::sqrt(
                std::pow(transform.x - x, 2) +
                std::pow(transform.y - y, 2)
            );

            std::cout << "Distance to bubble " << entity << ": " << distance << std::endl;

            if (distance <= GRID_SIZE * 1.1f) {
                std::cout << "Collision detected with bubble " << entity << std::endl;
                collision = true;
                if (distance < closestDistance) {
                    closestDistance = distance;

                    float angle = std::atan2(y - transform.y, x - transform.x);
                    snapX = transform.x + GRID_SIZE * std::cos(angle);
                    snapY = transform.y + GRID_SIZE * std::sin(angle);

                    std::cout << "New snap position calculated - x: " << snapX << ", y: " << snapY << std::endl;
                }
            }
        }

        return collision;
    }

    GridPosition worldToGrid(float x, float y) {
        float relativeX = x - GRID_OFFSET_X;
        float relativeY = y - GRID_OFFSET_Y;

        int col = static_cast<int>(std::round(relativeX / GRID_SIZE));
        int row = static_cast<int>(std::round(relativeY / GRID_SIZE));

        // Update grid size if needed
        updateGridSize(row);

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

                        if (newRow >= 0 && newRow < MAX_ROWS &&
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

        // Horizontal neighbors
        if (col > 0) neighbors.push_back({row, col - 1});
        if (col < COLS - 1) neighbors.push_back({row, col + 1});

        // Vertical neighbors based on row parity
        bool isEvenRow = (row % 2 == 0);

        // Upper neighbors
        if (row > 0) {
            if (isEvenRow) {
                neighbors.push_back({row - 1, col});
                if (col > 0) neighbors.push_back({row - 1, col - 1});
            } else {
                neighbors.push_back({row - 1, col});
                if (col < COLS - 1) neighbors.push_back({row - 1, col + 1});
            }
        }

        // Lower neighbors
        if (row < MAX_ROWS - 1) {
            if (isEvenRow) {
                neighbors.push_back({row + 1, col});
                if (col > 0) neighbors.push_back({row + 1, col - 1});
            } else {
                neighbors.push_back({row + 1, col});
                if (col < COLS - 1) neighbors.push_back({row + 1, col + 1});
            }
        }

        return neighbors;
    }

    bool colorMatches(const SDL_Color &c1, const SDL_Color &c2) {
        return c1.r == c2.r && c1.g == c2.g && c1.b == c2.b;
    }

    std::vector<Entity> findMatchingGroup(Entity bubble) {
        if (!gCoordinator.hasComponent<Color>(bubble) ||
            !gCoordinator.hasComponent<Bubble>(bubble)) {
            return {};
        }

        auto &color = gCoordinator.getComponent<Color>(bubble);
        auto &bubbleComponent = gCoordinator.getComponent<Bubble>(bubble);

        std::vector<Entity> matches;
        std::set<GridPosition> visited;
        std::queue<GridPosition> toVisit;

        GridPosition startPos{bubbleComponent.row, bubbleComponent.col};
        toVisit.push(startPos);
        visited.insert(startPos);
        matches.push_back(bubble);

        while (!toVisit.empty()) {
            GridPosition current = toVisit.front();
            toVisit.pop();

            auto neighbors = getNeighbors(current.row, current.col);
            for (const auto &neighbor: neighbors) {
                if (neighbor.row >= grid.size() || neighbor.col >= COLS ||
                    visited.find(neighbor) != visited.end()) {
                    continue;
                }

                Entity neighborEntity = grid[neighbor.row][neighbor.col];
                if (neighborEntity != INVALID_ENTITY &&
                    gCoordinator.hasComponent<Color>(neighborEntity)) {
                    auto &neighborColor = gCoordinator.getComponent<Color>(neighborEntity);

                    // Check if colors match
                    if (colorMatches(color.color, neighborColor.color)) {
                        matches.push_back(neighborEntity);
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
            // Update score immediately
            updateScore(bubbles);

            for (Entity bubble: bubbles) {
                if (!gCoordinator.hasComponent<Transform>(bubble)) continue;

                auto &transform = gCoordinator.getComponent<Transform>(bubble);
                GridPosition pos = worldToGrid(transform.x, transform.y);

                if (isValidGridPosition(pos.row, pos.col)) {
                    grid[pos.row][pos.col] = INVALID_ENTITY;
                    if (!gCoordinator.hasComponent<Destroy>(bubble)) {
                        gCoordinator.addComponent<Destroy>(bubble, Destroy{});
                    }
                    gCoordinator.getComponent<Destroy>(bubble).destroy = true;
                }
            }
        } catch (const std::exception &e) {
            std::cerr << "Error removing bubbles: " << e.what() << std::endl;
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
        for (int radius = 1; radius < std::max(MAX_ROWS, COLS); radius++) {
            // Check positions in a diamond pattern
            for (int dy = -radius; dy <= radius; dy++) {
                for (int dx = -radius; dx <= radius; dx++) {
                    // Only check positions that are exactly 'radius' steps away
                    if (std::abs(dx) + std::abs(dy) == radius) {
                        int newRow = startPos.row + dy;
                        int newCol = startPos.col + dx;

                        // Check if position is valid and empty
                        if (newRow >= 0 && newRow < MAX_ROWS &&
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

    void findConnectedBubbles(int row, int col, std::set<Entity> &anchored) {
        if (!isValidGridPosition(row, col)) return;

        Entity bubble = grid[row][col];
        if (bubble == INVALID_ENTITY || anchored.find(bubble) != anchored.end()) {
            return;
        }

        anchored.insert(bubble);

        // Get valid neighbors
        auto neighbors = getNeighbors(row, col);
        for (const auto &neighbor: neighbors) {
            if (isValidGridPosition(neighbor.row, neighbor.col)) {
                findConnectedBubbles(neighbor.row, neighbor.col, anchored);
            }
        }
    }

    void processMatchesAndFloating(Entity entity) {
        try {
            auto matches = findMatchingGroup(entity);
            if (matches.size() >= 3) {
                // Immediately remove matched bubbles and update score
                removeBubbles(matches);

                // Queue floating bubble check with very small delay
                Event checkFloatingEvent{
                    eventTypeToString(EventType::CheckFloatingBubbles),
                    {}
                };
                // eventCoordinator.queueEvent(
                //     std::make_shared<Event>(checkFloatingEvent),
                //     eventTimeline.getElapsedTime() + 50, // Reduced delay for more immediate feedback
                //     Priority::HIGH // Higher priority for immediate processing
                // );
                eventCoordinator.emit(std::make_shared<Event>(checkFloatingEvent));
            }
        } catch (const std::exception &e) {
            std::cerr << "Error in processMatchesAndFloating: " << e.what() << std::endl;
        }
    }

public:
    BubbleGridSystem() : grid(1, std::vector<Entity>(COLS, INVALID_ENTITY)) {
        std::cout << "Grid initialized with dimensions: " << grid.size() << "x" << COLS << std::endl;
    }

    void initializeGridBubble(Entity entity, int row, int col) {
        try {
            updateGridSize(row);

            // Don't allow bubbles outside the grid boundaries
            if (col >= 0 && col < COLS && row >= 0 && row < MAX_ROWS) {
                grid[row][col] = entity;
                if (!gCoordinator.hasComponent<Bubble>(entity)) {
                    gCoordinator.addComponent(entity, Bubble{true, row, col, GRID_SIZE / 2});
                }
                // std::cout << "Initialized grid bubble at (" << row << "," << col
                // << ") with entity " << entity << std::endl;
            } else {
                // Destroy bubbles outside grid boundaries
                if (!gCoordinator.hasComponent<Destroy>(entity)) {
                    gCoordinator.addComponent<Destroy>(entity, Destroy{});
                }
                gCoordinator.getComponent<Destroy>(entity).destroy = true;
            }
        } catch (const std::exception &e) {
            std::cerr << "Error initializing grid bubble: " << e.what() << std::endl;
        }
    }

    void addBubble(Entity entity) {
        try {
            auto &transform = gCoordinator.getComponent<Transform>(entity);

            // Calculate exact grid position
            float relativeX = transform.x - GRID_OFFSET_X;
            float relativeY = transform.y - GRID_OFFSET_Y;

            int row = static_cast<int>(round(relativeY / GRID_SIZE));
            int col = static_cast<int>(round(relativeX / GRID_SIZE));

            // Adjust for odd rows
            if (row % 2 == 1) {
                relativeX -= GRID_SIZE / 2.0f;
                col = static_cast<int>(round(relativeX / GRID_SIZE));
            }

            if (row >= 0 && row < MAX_ROWS && col >= 0 && col < COLS) {
                ensureGridSize(row);
                grid[row][col] = entity;

                if (!gCoordinator.hasComponent<Bubble>(entity)) {
                    gCoordinator.addComponent(entity, Bubble{true, row, col});
                } else {
                    auto &bubble = gCoordinator.getComponent<Bubble>(entity);
                    bubble.row = row;
                    bubble.col = col;
                }

                // Immediately process matches and floating bubbles
                processMatchesAndFloating(entity);
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

    void handleFloatingBubbles() {
        try {
            std::set<Entity> anchored;

            // First, find bubbles connected to the top row
            for (int col = 0; col < COLS; ++col) {
                if (isValidGridPosition(0, col) && grid[0][col] != INVALID_ENTITY) {
                    findConnectedBubbles(0, col, anchored);
                }
            }

            // Remove unanchored bubbles
            auto bubbles = gCoordinator.getEntitiesWithComponent<Bubble>();
            for (auto entity: bubbles) {
                if (!gCoordinator.hasComponent<Bubble>(entity)) continue;

                auto &bubble = gCoordinator.getComponent<Bubble>(entity);
                if (!isValidGridPosition(bubble.row, bubble.col)) continue;

                if (anchored.find(entity) == anchored.end()) {
                    // Remove from grid
                    grid[bubble.row][bubble.col] = INVALID_ENTITY;

                    // Mark for destruction
                    if (!gCoordinator.hasComponent<Destroy>(entity)) {
                        gCoordinator.addComponent<Destroy>(entity, Destroy{});
                    }
                    gCoordinator.getComponent<Destroy>(entity).destroy = true;
                }
            }
        } catch (const std::exception &e) {
            std::cerr << "Error in handleFloatingBubbles: " << e.what() << std::endl;
        }
    }


    void handleGridDrop() {
        try {
            // Update grid coordinates
            updateGridCoordinates();

            // Process all bubbles for matches after grid movement
            auto bubbles = gCoordinator.getEntitiesWithComponent<Bubble>();
            std::set<Entity> processedBubbles;

            for (auto entity: bubbles) {
                if (!gCoordinator.hasComponent<Bubble>(entity) ||
                    processedBubbles.find(entity) != processedBubbles.end()) {
                    continue;
                }

                auto matches = findMatchingGroup(entity);
                if (matches.size() >= 3) {
                    for (auto match: matches) {
                        processedBubbles.insert(match);
                    }
                    removeBubbles(matches);
                }
            }

            // Check for floating bubbles after all matches are processed
            Event checkFloatingEvent{
                eventTypeToString(EventType::CheckFloatingBubbles),
                {}
            };
            eventCoordinator.queueEvent(
                std::make_shared<Event>(checkFloatingEvent),
                eventTimeline.getElapsedTime() + 50,
                Priority::HIGH
            );
        } catch (const std::exception &e) {
            std::cerr << "Error in handleGridDrop: " << e.what() << std::endl;
        }
    }


    Entity getGridEntity(int row, int col) const {
        if (!isValidGridPosition(row, col)) {
            return INVALID_ENTITY;
        }
        return grid[row][col];
    }
};

#endif //BUBBLE_GRID_HPP
