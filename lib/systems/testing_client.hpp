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
                GameManager::getInstance()->gameRunning = false;
                return;
            }

            if (testbench.testStarted) {
                ++testbench.iterations;
                if (testbench.iterations % 100 == 0) {
                    std::cout << timeline.getElapsedTime() - testbench.startTime << std::endl;
                }
                if (testbench.iterations > 10000) {
                    testbench.testCompleted = true;
                }
                return;
            }
            testbench.testStarted = true;
            testbench.iterations = 0;
            testbench.startTime = timeline.getElapsedTime();
        }
    }
};
