//
// Created by Utsav Lal on 10/16/24.
//

#pragma once

#include <iostream>
#include <zmq.hpp>

#include "../ECS/coordinator.hpp"
#include "../ECS/system.hpp"
#include "../model/components.hpp"

extern Coordinator gCoordinator;

class TestingClientSystem : public System {
    std::vector<Entity> generatedEntities;

public:
    void update(Timeline &timeline) {
        for (auto entity: entities) {
            auto &testbench = gCoordinator.getComponent<TestClient>(entity);
            if (testbench.testCompleted) {
                return;
            }

            if (testbench.testStarted) {
                if (++testbench.iterations > 10000) {
                    testbench.endTime = timeline.getElapsedTime();
                    std::cout << "Time taken for " << testbench.entities << " entities: " << testbench.endTime -
                            testbench.startTime << " milliseconds" << std::endl;
                    testbench.testStarted = false;
                    testbench.testCompleted = true;
                    std::cout << "Total number of entities generated: " << gCoordinator.getEntityIds().size() << std::endl;
                    // for (const auto entity1: generatedEntities) {
                    //     gCoordinator.addComponent(entity1, Destroy{0, true, true});
                    // }
                }
                return;
            }

            std::cout << "Generating entities" << std::endl;
            generatedEntities.reserve(testbench.entities);
            for (int i = 0; i < testbench.entities; i++) {
                const Entity id = gCoordinator.createEntity();
                auto x = static_cast<float>(Random::generateRandomInt(0, SCREEN_WIDTH));
                Transform transform{static_cast<float>(Random::generateRandomInt(0, SCREEN_WIDTH)), 50, 32, 32};
                Color color{
                    SDL_Color{
                        static_cast<Uint8>(Random::generateRandomInt(0, 255)),
                        static_cast<Uint8>(Random::generateRandomInt(0, 255)),
                        static_cast<Uint8>(Random::generateRandomInt(0, 255)), 255
                    }
                };
                gCoordinator.addComponent(id, transform);
                gCoordinator.addComponent(id, color);
                gCoordinator.addComponent(id, Gravity{0, static_cast<float>(Random::generateRandomFloat(0, 1))});
                gCoordinator.addComponent(id, CKinematic{0, 0});
                gCoordinator.addComponent(id, ClientEntity{});
                gCoordinator.addComponent(id, Destroy{});

                generatedEntities.emplace_back(id);
            }
            testbench.testStarted = true;
            testbench.startTime = timeline.getElapsedTime();
            std::cout << "Entities generated and time started" << std::endl;
        }
    }
};
