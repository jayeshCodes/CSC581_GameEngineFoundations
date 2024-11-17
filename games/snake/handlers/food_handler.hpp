//
// Created by Utsav Lal on 11/16/24.
//

#pragma once
#include "../../../lib/helpers/colors.hpp"
#include "../.././../lib/ECS/coordinator.hpp"
#include "../.././../lib/ECS/system.hpp"
#include "../.././../lib/EMS/event_coordinator.hpp"
#include "../helpers/quantizer.hpp"
#include "../model/components.hpp"
#include "../model/event.hpp"

extern Coordinator gCoordinator;
extern EventCoordinator eventCoordinator;
extern int screen_width;
extern int screen_height;


class FoodHandler : public System {
    int length = -1;

    std::vector<float> generateRandomSpawnPoint(int length) {
        int max_rows = screen_height / length;
        int max_cols = screen_width / length;
        int rRow = Random::generateRandomInt(1, max_rows - 2);
        int rCol = Random::generateRandomInt(1, max_cols - 2);
        return SnakeQuantizer::dequantize(rRow, rCol, length);
    }

    void generateFood() {
        auto spawnPos = generateRandomSpawnPoint(this->length);
        auto food = gCoordinator.createEntity();
        gCoordinator.addComponent(food, Transform{
                                      spawnPos[0], spawnPos[1], static_cast<float>(this->length),
                                      static_cast<float>(this->length), 0, 1
                                  });
        gCoordinator.addComponent(food, Color{shade_color::Black});
        gCoordinator.addComponent(food, Collision{true, false, CollisionLayer::FOOD});
        gCoordinator.addComponent(food, Destroy{});
        gCoordinator.addComponent(food, Food{});
    }

    EventHandler gameStartHandler = [this](const std::shared_ptr<Event> (&event)) {
        if (event->type == GameEvents::eventTypeToString(GameEvents::GameStart)) {
            // Create food entity
            GameEvents::GameStartData data = event->data;
            this->length = data.length;
            generateFood();
        }
    };

    EventHandler foodEatenHandler = [this](const std::shared_ptr<Event> (&event)) {
        if (event->type == GameEvents::eventTypeToString(GameEvents::FoodEaten)) {
            generateFood();
        }
    };

public:
    FoodHandler() {
        eventCoordinator.subscribe(gameStartHandler, GameEvents::eventTypeToString(GameEvents::GameStart));
        eventCoordinator.subscribe(foodEatenHandler, GameEvents::eventTypeToString(GameEvents::FoodEaten));
    }

    ~FoodHandler() {
        eventCoordinator.unsubscribe(gameStartHandler, GameEvents::eventTypeToString(GameEvents::GameStart));
        eventCoordinator.unsubscribe(foodEatenHandler, GameEvents::eventTypeToString(GameEvents::FoodEaten));
    }
};
