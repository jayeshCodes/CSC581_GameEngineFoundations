//
// Created by Jayesh Gajbhar on 11/16/24.
//

#ifndef BUBBLE_GRID_HPP
#define BUBBLE_GRID_HPP

#include "../ECS/system.hpp"
#include "../model/components.hpp"
#include "../ECS/coordinator.hpp"
#include "../EMS/event_coordinator.hpp"
#include "../model/event.hpp"
#include <queue>
#include <set>

extern Coordinator gCoordinator;
extern EventCoordinator eventCoordinator;

class BubbleGridSystem : public System {
private:
    static constexpr int MAX_ROWS = 24;
    static constexpr int COLS = 15;
    static constexpr float GRID_SIZE = 32.0f;
    static constexpr float GRID_OFFSET_X = (SCREEN_WIDTH - (COLS * GRID_SIZE)) / 2.0f;
    static constexpr float GRID_OFFSET_Y = 32.0f;
    static constexpr float OVERLAP_THRESHOLD = GRID_SIZE * 0.8f;

    std::vector<std::vector<Entity> > grid;

    bool isValidGridPosition(int row, int col) const {
        return row >= 0 && row < MAX_ROWS && col >= 0 && col < COLS;
    }

    bool checkForOverlap(float x, float y) {
        auto bubbles = gCoordinator.getEntitiesWithComponent<Bubble>();
        for (auto entity: bubbles) {
            auto &transform = gCoordinator.getComponent<Transform>(entity);
            float dx = transform.x - x;
            float dy = transform.y - y;
            float distance = std::sqrt(dx * dx + dy * dy);

            // Only consider overlap if very close (less than half grid size)
            if (distance < GRID_SIZE / 2) {
                std::cout << "Real overlap found at distance: " << distance << std::endl;
                return true;
            }
        }
        return false;
    }


    void checkFloatingBubbles() {
        std::set<Entity> attachedBubbles;
        std::set<Entity> processed;

        // First find all bubbles attached to top row
        for (int col = 0; col < COLS; col++) {
            if (grid[0][col] != INVALID_ENTITY) {
                findAttachedBubbles(0, col, attachedBubbles, processed);
            }
        }

        // Remove any bubbles not attached to top
        auto allBubbles = gCoordinator.getEntitiesWithComponent<Bubble>();
        for (Entity bubble: allBubbles) {
            if (attachedBubbles.find(bubble) == attachedBubbles.end()) {
                auto &bubbleComp = gCoordinator.getComponent<Bubble>(bubble);
                grid[bubbleComp.row][bubbleComp.col] = INVALID_ENTITY;

                if (!gCoordinator.hasComponent<Destroy>(bubble)) {
                    gCoordinator.addComponent(bubble, Destroy{});
                }
                gCoordinator.getComponent<Destroy>(bubble).destroy = true;
            }
        }
    }

    void findAttachedBubbles(int row, int col, std::set<Entity> &attached, std::set<Entity> &processed) {
        if (!isValidGridPosition(row, col) ||
            processed.find(grid[row][col]) != processed.end() ||
            grid[row][col] == INVALID_ENTITY) {
            return;
        }

        Entity current = grid[row][col];
        attached.insert(current);
        processed.insert(current);

        std::vector<std::pair<int, int> > neighborOffsets;
        if (row % 2 == 0) {
            neighborOffsets = {{0, -1}, {0, 1}, {-1, -1}, {-1, 0}, {1, -1}, {1, 0}};
        } else {
            neighborOffsets = {{0, -1}, {0, 1}, {-1, 0}, {-1, 1}, {1, 0}, {1, 1}};
        }

        for (auto [rowOffset, colOffset]: neighborOffsets) {
            findAttachedBubbles(row + rowOffset, col + colOffset, attached, processed);
        }
    }


    void updateGridAfterMatch(const std::vector<Entity> &matches) {
        for (Entity match: matches) {
            if (!gCoordinator.hasComponent<Bubble>(match)) continue;

            auto &bubble = gCoordinator.getComponent<Bubble>(match);
            grid[bubble.row][bubble.col] = INVALID_ENTITY;
        }

        // Check for floating bubbles
        checkFloatingBubbles();
    }


    bool colorMatches(const SDL_Color &c1, const SDL_Color &c2) {
        return c1.r == c2.r && c1.g == c2.g && c1.b == c2.b;
    }

public:

    void resetGrid() {
        grid = std::vector<std::vector<Entity>>(MAX_ROWS, std::vector<Entity>(COLS, INVALID_ENTITY));
    }

    void initializeGridBubble(Entity entity, int row, int col) {
        if (!isValidGridPosition(row, col)) return;
        grid[row][col] = entity;
    }

    std::vector<Entity> findMatchingBubbles(Entity bubble) {
        std::vector<Entity> matches;
        if (!gCoordinator.hasComponent<Color>(bubble) ||
            !gCoordinator.hasComponent<Bubble>(bubble)) {
            return matches;
        }

        auto &color = gCoordinator.getComponent<Color>(bubble);
        auto &bubbleComponent = gCoordinator.getComponent<Bubble>(bubble);

        std::queue<std::pair<int, int> > toCheck;
        std::set<std::pair<int, int> > visited;

        toCheck.push({bubbleComponent.row, bubbleComponent.col});
        visited.insert({bubbleComponent.row, bubbleComponent.col});
        matches.push_back(bubble);

        while (!toCheck.empty()) {
            auto [row, col] = toCheck.front();
            toCheck.pop();

            std::vector<std::pair<int, int> > neighborOffsets;
            if (row % 2 == 0) {
                neighborOffsets = {{0, -1}, {0, 1}, {-1, -1}, {-1, 0}, {1, -1}, {1, 0}};
            } else {
                neighborOffsets = {{0, -1}, {0, 1}, {-1, 0}, {-1, 1}, {1, 0}, {1, 1}};
            }

            for (auto [rowOffset, colOffset]: neighborOffsets) {
                int newRow = row + rowOffset;
                int newCol = col + colOffset;

                if (!isValidGridPosition(newRow, newCol) ||
                    visited.find({newRow, newCol}) != visited.end()) {
                    continue;
                }

                Entity neighbor = grid[newRow][newCol];
                if (neighbor == INVALID_ENTITY ||
                    !gCoordinator.hasComponent<Color>(neighbor)) {
                    continue;
                }

                auto &neighborColor = gCoordinator.getComponent<Color>(neighbor);
                if (colorMatches(color.color, neighborColor.color)) {
                    visited.insert({newRow, newCol});
                    toCheck.push({newRow, newCol});
                    matches.push_back(neighbor);
                }
            }
        }

        return matches;
    }

    void addBubble(Entity entity) {
        if (!gCoordinator.hasComponent<Transform>(entity) ||
            !gCoordinator.hasComponent<Bubble>(entity)) {
            std::cout << "Missing required components for bubble" << std::endl;
            return;
        }

        auto &transform = gCoordinator.getComponent<Transform>(entity);
        auto &bubble = gCoordinator.getComponent<Bubble>(entity);

        // Calculate grid position first
        int row = static_cast<int>(round((transform.y - GRID_OFFSET_Y) / GRID_SIZE));
        int col = static_cast<int>(round((transform.x - GRID_OFFSET_X) / GRID_SIZE));
        if (row % 2 == 1) {
            col = static_cast<int>(round((transform.x - GRID_OFFSET_X - GRID_SIZE / 2) / GRID_SIZE));
        }

        // Validate position before checking overlap
        if (!isValidGridPosition(row, col)) {
            std::cout << "Invalid grid position" << std::endl;
            return;
        }

        // Only check overlap for the specific grid position we want to place in
        if (grid[row][col] != INVALID_ENTITY) {
            std::cout << "Grid position already occupied" << std::endl;
            if (!gCoordinator.hasComponent<Destroy>(entity)) {
                gCoordinator.addComponent<Destroy>(entity, Destroy{});
            }
            gCoordinator.getComponent<Destroy>(entity).destroy = true;
            return;
        }

        // Position is valid and free, add the bubble
        grid[row][col] = entity;
        bubble.row = row;
        bubble.col = col;

        std::cout << "Successfully added bubble at row: " << row << " col: " << col << std::endl;

        auto matches = findMatchingBubbles(entity);
        std::cout << "Found " << matches.size() << " matches" << std::endl;

        if (matches.size() >= 3) {
            std::cout << "Emitting BubbleMatch event" << std::endl;
            Event matchEvent{eventTypeToString(EventType::BubbleMatch), BubbleMatchData{matches}};
            eventCoordinator.emit(std::make_shared<Event>(matchEvent));
        }
    }

    void reset() {
        grid = std::vector<std::vector<Entity> >(MAX_ROWS, std::vector<Entity>(COLS, INVALID_ENTITY));
    }


    BubbleGridSystem() : grid(MAX_ROWS, std::vector<Entity>(COLS, INVALID_ENTITY)) {
    }

    void update() {
        // Process any pending bubbles that need to be added to the grid
        std::cout << "Updating bubble grid" << std::endl;
        auto bubbles = gCoordinator.getEntitiesWithComponent<Bubble>();
        for (auto entity: bubbles) {
            if (!gCoordinator.hasComponent<Transform>(entity)) continue;

            auto &transform = gCoordinator.getComponent<Transform>(entity);
            auto &bubble = gCoordinator.getComponent<Bubble>(entity);

            // Check if this bubble needs grid placement
            int row = static_cast<int>(round((transform.y - GRID_OFFSET_Y) / GRID_SIZE));
            int col = static_cast<int>(round((transform.x - GRID_OFFSET_X) / GRID_SIZE));
            if (row % 2 == 1) {
                col = static_cast<int>(round((transform.x - GRID_OFFSET_X - GRID_SIZE / 2) / GRID_SIZE));
            }

            if (isValidGridPosition(row, col) && grid[row][col] != entity) {
                // Check for matches before finalizing position
                grid[row][col] = entity;
                bubble.row = row;
                bubble.col = col;

                auto matches = findMatchingBubbles(entity);
                if (matches.size() >= 3) {
                    std::cout << "Found " << matches.size() << " matches" << std::endl;
                    Event matchEvent{eventTypeToString(EventType::BubbleMatch), BubbleMatchData{matches}};
                    eventCoordinator.emit(std::make_shared<Event>(matchEvent));
                }
                checkFloatingBubbles();
            }
        }
    }


    void clearGridPosition(int row, int col) {
        if (isValidGridPosition(row, col)) {
            grid[row][col] = INVALID_ENTITY;
        }
    }

    void updateGridPosition(Entity entity, int newRow, int newCol) {
        if (!isValidGridPosition(newRow, newCol)) return;

        // Find and clear old position
        auto &bubble = gCoordinator.getComponent<Bubble>(entity);
        if (isValidGridPosition(bubble.row, bubble.col)) {
            grid[bubble.row][bubble.col] = INVALID_ENTITY;
        }

        // Update to new position
        grid[newRow][newCol] = entity;
    }
};

#endif //BUBBLE_GRID_HPP
