//
// Created by Jayesh Gajbhar on 11/22/24.
//

#ifndef SCORE_HPP
#define SCORE_HPP

#include "../ECS/coordinator.hpp"
#include "../EMS/event_coordinator.hpp"
#include "../ECS/system.hpp"
#include "../model/components.hpp"
#include "../core/structs.hpp"
#include "../core/defs.hpp"
#include "../model/event.hpp"
#include <mutex>

extern Coordinator gCoordinator;
extern EventCoordinator eventCoordinator;

class ScoreSystem : public System {
public:
    void update() {
        for (auto &entity: entities) {
            auto &transform = gCoordinator.getComponent<Transform>(entity);
            auto &score = gCoordinator.getComponent<Score>(entity);

            if (transform.y < score.highestY) {
                // Calculate distance traveled upward
                score.score = (SCREEN_HEIGHT - transform.y) * 0.1f;  // Scale factor to make score more readable
                score.highestY = transform.y;
            }
        }
    }
};

#endif //SCORE_HPP
