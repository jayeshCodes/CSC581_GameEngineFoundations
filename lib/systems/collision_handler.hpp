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
    EventHandler collisionHandler = [this](const std::shared_ptr<Event> &event) {
        if (event->type == eventTypeToString(EventType::EntityCollided)) {
            try {
                EntityCollidedData data = event->data;
                Entity bubbleEntity = INVALID_ENTITY;

                // Determine which entity is the bubble
                if (gCoordinator.hasComponent<BubbleProjectile>(data.entityA)) {
                    bubbleEntity = data.entityA;
                } else if (gCoordinator.hasComponent<BubbleProjectile>(data.entityB)) {
                    bubbleEntity = data.entityB;
                }

                if (bubbleEntity == INVALID_ENTITY) {
                    return;
                }

                // Handle bubble collision
                auto &projectile = gCoordinator.getComponent<BubbleProjectile>(bubbleEntity);
                std::cout << "Handling bubble collision - reversing velocity" << std::endl;
                projectile.velocity.x = -projectile.velocity.x;
            } catch (const std::exception &e) {
                std::cerr << "Error handling collision: " << e.what() << std::endl;
            }
        }
    };

public:
    CollisionHandlerSystem() {
        eventCoordinator.subscribe(collisionHandler, eventTypeToString(EventType::EntityCollided));
    }

    ~CollisionHandlerSystem() {
        eventCoordinator.unsubscribe(collisionHandler, eventTypeToString(EventType::EntityCollided));
    }
};

#endif //COLLISION_HANDLER_HPP
