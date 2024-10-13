//
// Created by Jayesh Gajbhar on 10/13/24.
//

#include "../ECS/system.hpp"
#include "../components/components.hpp"
#include "../ECS/coordinator.hpp"
#include <mutex>
#include <optional>
#include <iostream>

extern Coordinator gCoordinator;

class CollisionSystem : public System {
public:
    void update(float dt) {
        std::lock_guard<std::mutex> lock(update_mutex);
        std::vector<std::pair<Entity, Entity> > collisions;

        for (auto it = entities.begin(); it != entities.end(); ++it) {
            Entity entity = *it;
            if (!hasRequiredComponents(entity)) continue;

            const auto &transform = gCoordinator.getComponent<Transform>(entity);
            const auto &collision = gCoordinator.getComponent<Collision>(entity);

            for (auto other_it = std::next(it); other_it != entities.end(); ++other_it) {
                Entity other_entity = *other_it;
                if (!hasRequiredComponents(other_entity)) continue;

                const auto &other_transform = gCoordinator.getComponent<Transform>(other_entity);
                const auto &other_collision = gCoordinator.getComponent<Collision>(other_entity);

                if (checkCollision(transform, collision, other_transform, other_collision)) {
                    collisions.emplace_back(entity, other_entity);
                }
            }
        }

        for (const auto &[e1, e2]: collisions) {
            handleCollision(e1, e2, dt);
        }
    }

private:
    static bool hasRequiredComponents(Entity entity) {
        return gCoordinator.hasComponent<Transform>(entity) &&
               gCoordinator.hasComponent<Collision>(entity) &&
               gCoordinator.hasComponent<CKinematic>(entity);
    }

    static bool checkCollision(const Transform &t1, const Collision &c1, const Transform &t2, const Collision &c2) {
        // Assuming all shapes are rectangles for now
        return (t1.x < t2.x + c2.width &&
                t1.x + c1.width > t2.x &&
                t1.y < t2.y + c2.height &&
                t1.y + c1.height > t2.y);
    }

    static void handleCollision(Entity e1, Entity e2, float dt) {
        auto &c1 = gCoordinator.getComponent<Collision>(e1);
        auto &c2 = gCoordinator.getComponent<Collision>(e2);

        if (c1.isTrigger || c2.isTrigger) {
            // Handle trigger collision (e.g., event system, callbacks)
        } else {
            auto &t1 = gCoordinator.getComponent<Transform>(e1);
            auto &t2 = gCoordinator.getComponent<Transform>(e2);
            auto &k1 = gCoordinator.getComponent<CKinematic>(e1);
            auto &k2 = gCoordinator.getComponent<CKinematic>(e2);

            // Avoid division by zero
            float width1 = std::max(c1.width, 0.001f);
            float height1 = std::max(c1.height, 0.001f);
            float width2 = std::max(c2.width, 0.001f);
            float height2 = std::max(c2.height, 0.001f);

            float overlap_x = (t1.x + width1 / 2) - (t2.x + width2 / 2);
            float overlap_y = (t1.y + height1 / 2) - (t2.y + height2 / 2);

            if (std::abs(overlap_x) < std::abs(overlap_y)) {
                t1.x += overlap_x / 2;
                t2.x -= overlap_x / 2;
                std::swap(k1.velocity.x, k2.velocity.x);
                k1.velocity.x *= 0.9f; // Slight energy loss
                k2.velocity.x *= 0.9f;
            } else {
                t1.y += overlap_y / 2;
                t2.y -= overlap_y / 2;
                std::swap(k1.velocity.y, k2.velocity.y);
                k1.velocity.y *= 0.9f; // Slight energy loss
                k2.velocity.y *= 0.9f;
            }

            // Apply changes based on dt, with bounds checking
            t1.x = std::clamp(t1.x + k1.velocity.x * dt, 0.0f, static_cast<float>(SCREEN_WIDTH));
            t1.y = std::clamp(t1.y + k1.velocity.y * dt, 0.0f, static_cast<float>(SCREEN_HEIGHT));
            t2.x = std::clamp(t2.x + k2.velocity.x * dt, 0.0f, static_cast<float>(SCREEN_WIDTH));
            t2.y = std::clamp(t2.y + k2.velocity.y * dt, 0.0f, static_cast<float>(SCREEN_HEIGHT));
        }
    }
};
