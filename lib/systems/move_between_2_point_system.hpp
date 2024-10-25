//
// Created by Utsav Lal on 10/8/24.
//

#ifndef MOVE_BETWEEN_2_POINT_SYSTEM_HPP
#define MOVE_BETWEEN_2_POINT_SYSTEM_HPP
#include "../ECS/system.hpp"
#include "../model/components.hpp"
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
            float currentX = movingPlatform.movementType == HORIZONTAL ? transform.x : transform.y;
            switch (movingPlatform.state) {
                case TO:
                    if (currentX <= movingPlatform.p1) {
                        movingPlatform.state = STOP;
                        currTime = timeline.getElapsedTime();
                        kinematic.velocity.x = 0;
                        kinematic.velocity.y = 0;
                        return;
                    }
                    movingPlatform.movementType == HORIZONTAL
                        ? kinematic.velocity.x = -200
                        : kinematic.velocity.y = -200;
                    break;
                case FRO:
                    if (currentX >= movingPlatform.p2) {
                        movingPlatform.state = STOP;
                        currTime = timeline.getElapsedTime();
                        kinematic.velocity.x = 0;
                        kinematic.velocity.y = 0;
                        return;
                    }
                    movingPlatform.movementType == HORIZONTAL ? kinematic.velocity.x = 200 : kinematic.velocity.y = 200;
                    break;
                case STOP:
                    Uint32 passedTime = timeline.getElapsedTime();
                    if ((passedTime - currTime) / 1000.f > movingPlatform.wait_time) {
                        if (currentX <= movingPlatform.p1) {
                            movingPlatform.state = FRO;
                        } else {
                            movingPlatform.state = TO;
                        }
                    }
                    break;
            }
        }
    }
};

#endif //MOVE_BETWEEN_2_POINT_SYSTEM_HPP
