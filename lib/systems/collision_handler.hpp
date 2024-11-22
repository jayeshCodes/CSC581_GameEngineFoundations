//
// Created by Jayesh Gajbhar on 10/30/24.
//

#ifndef COLLISION_HANDLER_HPP
#define COLLISION_HANDLER_HPP

#include "../ECS/coordinator.hpp"
#include "../EMS/event_coordinator.hpp"
#include "../model/components.hpp"
#include "../ECS/system.hpp"

extern Coordinator gCoordinator;
extern EventCoordinator eventCoordinator;

class CollisionHandlerSystem : public System {
private:
    EventHandler collisionHandler = [this](const std::shared_ptr<Event>& event) {
        if (event->type == eventTypeToString(EventType::EntityCollided)) {
            EntityCollidedData data = event->data;
            auto& entityA = data.entityA;
            auto& entityB = data.entityB;

            auto& transformA = gCoordinator.getComponent<Transform>(entityA);
            auto& transformB = gCoordinator.getComponent<Transform>(entityB);
            auto& kinematicA = gCoordinator.getComponent<CKinematic>(entityA);
            auto& kinematicB = gCoordinator.getComponent<CKinematic>(entityB);
            auto& collisionA = gCoordinator.getComponent<Collision>(entityA);
            auto& collisionB = gCoordinator.getComponent<Collision>(entityB);
            auto& rigidBodyA = gCoordinator.getComponent<RigidBody>(entityA);
            auto& rigidBodyB = gCoordinator.getComponent<RigidBody>(entityB);

            // Determine which entity is the player
            bool aIsPlayer = gCoordinator.getEntityKey(entityA) == mainCharID;
            bool bIsPlayer = gCoordinator.getEntityKey(entityB) == mainCharID;

            if (!aIsPlayer && !bIsPlayer) return;

            // Get player and platform entities
            Entity playerEntity = aIsPlayer ? entityA : entityB;
            Entity platformEntity = aIsPlayer ? entityB : entityA;

            auto& playerTransform = aIsPlayer ? transformA : transformB;
            auto& playerKinematic = aIsPlayer ? kinematicA : kinematicB;
            auto& platformTransform = aIsPlayer ? transformB : transformA;
            auto& platformCollision = aIsPlayer ? collisionB : collisionA;

            // Calculate overlap
            float overlapX = std::min(playerTransform.x + playerTransform.w, platformTransform.x + platformTransform.w) -
                           std::max(playerTransform.x, platformTransform.x);
            float overlapY = std::min(playerTransform.y + playerTransform.h, platformTransform.y + platformTransform.h) -
                           std::max(playerTransform.y, platformTransform.y);

            // Check if player is above platform
            bool fromTop = (playerTransform.y + playerTransform.h - overlapY <= platformTransform.y);

            if (fromTop && platformCollision.isTrigger) {
                // Handle trigger collision from top
                if (playerKinematic.velocity.y > 0) {
                    // Emit trigger event but don't resolve collision physically
                    Event triggerEvent{eventTypeToString(EventType::EntityTriggered),
                        EntityTriggeredData{platformEntity, playerEntity}};
                    eventCoordinator.emit(std::make_shared<Event>(triggerEvent));
                }
            }
            else if (fromTop) {
                // Normal platform collision from top
                const float epsilon = 0.01f;
                playerTransform.y = platformTransform.y - playerTransform.h - epsilon;
                if (playerKinematic.velocity.y > 0) {
                    playerKinematic.velocity.y = 0;
                }

                // Update jump component if it exists
                if (gCoordinator.hasComponent<Jump>(playerEntity)) {
                    auto& jump = gCoordinator.getComponent<Jump>(playerEntity);
                    jump.canJump = true;
                }
            }
            else {
                // Side or bottom collisions
                if (overlapX < overlapY) {
                    // Side collision
                    float pushDirection = (playerTransform.x < platformTransform.x) ? -1.0f : 1.0f;
                    playerTransform.x += overlapX * pushDirection;
                    playerKinematic.velocity.x = 0;
                }
                else {
                    // Bottom collision
                    playerTransform.y = platformTransform.y + platformTransform.h;
                    playerKinematic.velocity.y = 0;
                }
            }
        }
    };

public:
    CollisionHandlerSystem() {
        // eventCoordinator.subscribe(collisionHandler, eventTypeToString(EventType::EntityCollided));
    }

    ~CollisionHandlerSystem() {
        // eventCoordinator.unsubscribe(collisionHandler, eventTypeToString(EventType::EntityCollided));
    }
};

#endif //COLLISION_HANDLER_HPP
