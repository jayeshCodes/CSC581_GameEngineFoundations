//
// Created by Utsav Lal on 10/8/24.
//

#ifndef MOVE_BETWEEN_2_POINT_SYSTEM_HPP
#define MOVE_BETWEEN_2_POINT_SYSTEM_HPP
#include "../ECS/system.hpp"
#include "../components/components.hpp"
#include "../ECS/coordinator.hpp"

extern Coordinator gCoordinator;

class MoveBetween2PointsSystem : public System {
private:
    Uint32 currTime = 0;

public:
    void update(float dt, Timeline &timeline) {
        for (const auto entity: entities) {
            auto &movingPlatform = gCoordinator.getComponent<MovingPlatform>(entity);
            auto &transform = gCoordinator.getComponent<Transform>(entity);
            auto &kinematic = gCoordinator.getComponent<CKinematic>(entity);
            float currentX = transform.x;
            switch (movingPlatform.state) {
                case LEFT:
                    if (currentX <= movingPlatform.p1) {
                        movingPlatform.state = STOP;
                        currTime = timeline.getElapsedTime();
                        kinematic.velocity.x = 0;
                        return;
                    }
                    kinematic.velocity.x = -200;
                    break;
                case RIGHT:
                    if (currentX >= movingPlatform.p2) {
                        movingPlatform.state = STOP;
                        currTime = timeline.getElapsedTime();
                        kinematic.velocity.x = 0;
                        return;
                    }
                    kinematic.velocity.x = 200;
                    break;
                case STOP:
                    Uint32 passedTime = timeline.getElapsedTime();
                    if ((passedTime - currTime) / 1000.f > movingPlatform.wait_time) {
                        if (currentX <= movingPlatform.p1) {
                            movingPlatform.state = RIGHT;
                        } else {
                            movingPlatform.state = LEFT;
                        }
                    }
                    break;
            }
        }
    }
};

#endif //MOVE_BETWEEN_2_POINT_SYSTEM_HPP
