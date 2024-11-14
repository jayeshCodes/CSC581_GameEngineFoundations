//
// Created by Utsav Lal on 11/13/24.
//

#pragma once

#include "../../../lib/ECS/coordinator.hpp"
#include "../../../lib/ECS/system.hpp"
#include "../../../lib/EMS/event_coordinator.hpp"
#include "../../../lib/model/event.hpp"
#include "../model/component.hpp"
#include "../model/events.hpp"

extern Coordinator gCoordinator;
extern EventCoordinator eventCoordinator;

class CollisionHandler : public System {
    void brickAndBallCollision(Entity brick, Entity ball) {
        auto &ballKinematic = gCoordinator.getComponent<CKinematic>(ball);
        auto &ballTransform = gCoordinator.getComponent<Transform>(ball);
        auto &brickTransform = gCoordinator.getComponent<Transform>(brick);

        // Calculate overlaps
        const float overlapX = std::min(ballTransform.x + ballTransform.w, brickTransform.x + brickTransform.w) -
                               std::max(ballTransform.x, brickTransform.x);
        const float overlapY = std::min(ballTransform.y + ballTransform.h, brickTransform.y + brickTransform.h) -
                               std::max(ballTransform.y, brickTransform.y);

        // Ensure the ball doesn't stay inside the brick
        if (overlapX < overlapY) {
            // Collision is on the left or right
            ballKinematic.velocity.x *= -1;

            // Adjust the ball's position to the edge of the brick to avoid sticking
            if (ballTransform.x < brickTransform.x) {
                ballTransform.x = brickTransform.x - ballTransform.w; // Move ball to the left
            } else {
                ballTransform.x = brickTransform.x + brickTransform.w; // Move ball to the right
            }
        } else {
            // Collision is on the top or bottom
            ballKinematic.velocity.y *= -1;

            // Adjust the ball's position to the top or bottom of the brick
            if (ballTransform.y < brickTransform.y) {
                ballTransform.y = brickTransform.y - ballTransform.h; // Move ball above
            } else {
                ballTransform.y = brickTransform.y + brickTransform.h; // Move ball below
            }
        }

        // Queue the brick for destruction after collision processing
        gCoordinator.addComponent(brick, Destroy{0, true, true});
    }

    void ballAndLauncherCollision(Entity launcher, Entity ball) {
        auto &b = gCoordinator.getComponent<Ball>(ball);
        if(!b.isLaunched) return;

        auto &ballKinematic = gCoordinator.getComponent<CKinematic>(ball);
        auto &ballTransform = gCoordinator.getComponent<Transform>(ball);
        auto &launcherTransform = gCoordinator.getComponent<Transform>(launcher);

        float launcherMid = launcherTransform.x + launcherTransform.w / 2;
        float hitPosition = ballTransform.x - launcherMid;
        float normalizedHitPosition = hitPosition / (launcherTransform.w / 2);

        float angleFactor = 0.5f;
        ballKinematic.velocity.x = normalizedHitPosition * angleFactor * 100;
        ballKinematic.velocity.y = -std::abs(ballKinematic.velocity.y);
    }

    EventHandler collisionHandler = [this](const std::shared_ptr<Event> &event) {
        if (event->type == eventTypeToString(EventType::EntityCollided)) {
            const EntityCollidedData data = event->data;
            auto &entityA = data.entityA;
            auto &entityB = data.entityB;
            if (gCoordinator.hasComponent<Brick>(entityA)) {
                brickAndBallCollision(entityA, entityB);
            } else if (gCoordinator.hasComponent<Brick>(entityB)) {
                brickAndBallCollision(entityB, entityA);
            } else if (gCoordinator.hasComponent<Launcher>(entityA)) {
                ballAndLauncherCollision(entityA, entityB);
            } else if (gCoordinator.hasComponent<Launcher>(entityB)) {
                ballAndLauncherCollision(entityB, entityA);
            }
        }
    };

public:
    CollisionHandler() {
        eventCoordinator.subscribe(collisionHandler, eventTypeToString(EventType::EntityCollided));
    }

    ~CollisionHandler() {
        eventCoordinator.unsubscribe(collisionHandler, eventTypeToString(EventType::EntityCollided));
    }
};

