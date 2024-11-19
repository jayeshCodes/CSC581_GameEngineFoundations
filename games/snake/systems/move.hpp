//
// Created by Utsav Lal on 11/16/24.
//

#pragma once

#include "../../../lib/ECS/coordinator.hpp"
#include "../../../lib/ECS/system.hpp"
#include "../../../lib/EMS/event_coordinator.hpp"
#include "../../../lib/helpers/colors.hpp"
#include "../helpers/quantizer.hpp"
#include "../model/event.hpp"
#include "../model/components.hpp"

extern Coordinator gCoordinator;
extern EventCoordinator eventCoordinator;
extern int screen_width;
extern int screen_height;
extern int length;

enum Direction {
    UP,
    DOWN,
    LEFT,
    RIGHT
};

class MoveSystem : public System {
    std::deque<Entity> snakeParts;
    Direction currentDirection = Direction::RIGHT;
    float updateTime = 0.75f;
    float currentTime = 0.0f;
    bool gameOver = false;

    EventHandler gameStartHandler = [this](const std::shared_ptr<Event> (&event)) {
        if (event->type == GameEvents::eventTypeToString(GameEvents::GameStart)) {
            for(auto entity: entities) {
                gCoordinator.addComponent<Destroy>(entity, Destroy{0, true, true});
            }
            const int rows = screen_height / length;
            const int cols = screen_width / length;
            const auto startPos = SnakeQuantizer::dequantize(rows / 2, cols / 2, length);
            Entity player = gCoordinator.createEntity();
            gCoordinator.addComponent(player, Transform{startPos[0], startPos[1], static_cast<float>(length), static_cast<float>(length), 0, 1});
            gCoordinator.addComponent(player, Color{shade_color::Black});
            gCoordinator.addComponent(player, CKinematic{SDL_FPoint{150, 0}});
            gCoordinator.addComponent(player, Collision{true, false, CollisionLayer::PLAYER});
            gCoordinator.addComponent(player, Destroy{});
            gCoordinator.addComponent(player, Snake{length});
            snakeParts.clear();
            snakeParts.emplace_back(*entities.begin());
        }
    };

    EventHandler foodEatenHandler = [this](const std::shared_ptr<Event> (&event)) {
        if (event->type == GameEvents::eventTypeToString(GameEvents::FoodEaten)) {
            snakeParts.emplace_front(this->createBody());
        }
    };

    EventHandler movementHandler = [this](const std::shared_ptr<Event> &event) {
        if (event->type == GameEvents::eventTypeToString(GameEvents::Left)) {
            if (currentDirection != Direction::RIGHT) {
                currentDirection = Direction::LEFT;
            }
        } else if (event->type == GameEvents::eventTypeToString(GameEvents::Right)) {
            if (currentDirection != Direction::LEFT) {
                currentDirection = Direction::RIGHT;
            }
        } else if (event->type == GameEvents::eventTypeToString(GameEvents::Top)) {
            if (currentDirection != Direction::DOWN) {
                currentDirection = Direction::UP;
            }
        } else if (event->type == GameEvents::eventTypeToString(GameEvents::Down)) {
            if (currentDirection != Direction::UP) {
                currentDirection = Direction::DOWN;
            }
        }
    };

    Entity createBody() {
        auto &headTransform = gCoordinator.getComponent<Transform>(snakeParts.front());
        const auto length = static_cast<float>(gCoordinator.getComponent<Snake>(snakeParts.front()).length);
        Entity player;
        auto currentPos = SnakeQuantizer::quantize(headTransform.x, headTransform.y, length);
        std::vector<float> newHeadPos(2);
        switch (currentDirection) {
            case Direction::UP:
                newHeadPos = SnakeQuantizer::dequantize(currentPos[1] - 1, currentPos[0], length);
            break;
            case Direction::DOWN:
                newHeadPos = SnakeQuantizer::dequantize(currentPos[1] + 1, currentPos[0], length);
            break;
            case Direction::LEFT:
                newHeadPos = SnakeQuantizer::dequantize(currentPos[1], currentPos[0] - 1, length);
            break;
            case Direction::RIGHT:
                newHeadPos = SnakeQuantizer::dequantize(currentPos[1], currentPos[0] + 1, length);
            break;
        }
        player = gCoordinator.createEntity();
        gCoordinator.addComponent(player, Transform{newHeadPos[0], newHeadPos[1], length, length, 0, 1});
        gCoordinator.addComponent(player, Color{shade_color::Black});
        gCoordinator.addComponent(player, CKinematic{SDL_FPoint{150, 0}});
        gCoordinator.addComponent(player, Collision{true, false, CollisionLayer::PLAYER});
        gCoordinator.addComponent(player, Destroy{});
        gCoordinator.addComponent(player, Snake{static_cast<int>(length)});
        return player;
    }

public:
    MoveSystem() {
        eventCoordinator.subscribe(gameStartHandler, GameEvents::eventTypeToString(GameEvents::GameStart));
        eventCoordinator.subscribe(foodEatenHandler, GameEvents::eventTypeToString(GameEvents::FoodEaten));
        eventCoordinator.subscribe(movementHandler, GameEvents::eventTypeToString(GameEvents::Left));
        eventCoordinator.subscribe(movementHandler, GameEvents::eventTypeToString(GameEvents::Right));
        eventCoordinator.subscribe(movementHandler, GameEvents::eventTypeToString(GameEvents::Top));
        eventCoordinator.subscribe(movementHandler, GameEvents::eventTypeToString(GameEvents::Down));
    }

    ~MoveSystem() {
        eventCoordinator.unsubscribe(gameStartHandler, GameEvents::eventTypeToString(GameEvents::GameStart));
        eventCoordinator.unsubscribe(foodEatenHandler, GameEvents::eventTypeToString(GameEvents::FoodEaten));
        eventCoordinator.unsubscribe(movementHandler, GameEvents::eventTypeToString(GameEvents::Left));
        eventCoordinator.unsubscribe(movementHandler, GameEvents::eventTypeToString(GameEvents::Right));
        eventCoordinator.unsubscribe(movementHandler, GameEvents::eventTypeToString(GameEvents::Top));
        eventCoordinator.unsubscribe(movementHandler, GameEvents::eventTypeToString(GameEvents::Down));
    }

    void update(float dt) {
        if(gameOver) return;
        currentTime += dt;

        while (currentTime >= updateTime) {
            currentTime -= updateTime;

            // Update the snake's movement at fixed intervals
            snakeParts.emplace_front(createBody());
            const Entity entity = snakeParts.back();
            snakeParts.pop_back();
            gCoordinator.getComponent<Destroy>(entity).destroy = true;
        }
    }
};
