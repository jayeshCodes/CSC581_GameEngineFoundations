//
// Created by Jayesh Gajbhar on 11/23/24.
//

#ifndef PROJECTILE_MONSTER_COLLISION_HPP
#define PROJECTILE_MONSTER_COLLISION_HPP

#include "../ECS/coordinator.hpp"
#include "../ECS/system.hpp"
#include "../model/components.hpp"
#include "../model/event.hpp"
#include "../EMS/event_coordinator.hpp"

extern Coordinator gCoordinator;
extern EventCoordinator eventCoordinator;

class ProjectileMonsterCollisionSystem : public System {
private:
    const float Y_TOLERANCE = 20.0f; // Adjust this value based on testing

    bool checkCollision(const Transform &a, const Transform &b) {
        // Check X-axis overlap
        bool xOverlap = (a.x < b.x + b.w) && (a.x + a.w > b.x);

        // Check Y-axis overlap with tolerance
        bool yOverlap = (a.y - Y_TOLERANCE < b.y + b.h) && (a.y + a.h + Y_TOLERANCE > b.y);

        float centerAX = a.x + (a.w / 2);
        float centerAY = a.y + (a.h / 2);
        float centerBX = b.x + (b.w / 2);
        float centerBY = b.y + (b.h / 2);



        return xOverlap && yOverlap;
    }

    void debugDrawCollisionBox(const Transform &t, SDL_Color color) {
        // Draw collision box for debugging if you have access to the renderer
        SDL_SetRenderDrawColor(app->renderer, color.r, color.g, color.b, color.a);
        SDL_FRect rect = {t.x, t.y, t.w, t.h};
        SDL_RenderDrawRectF(app->renderer, &rect);
    }

public:
    void update() {
        auto monsters = std::vector<std::pair<Entity, Transform> >();
        auto projectiles = std::vector<std::pair<Entity, Transform> >();

        // First gather all valid entities and their transforms
        for (auto entity: entities) {
            if (!gCoordinator.getEntityIds().contains(gCoordinator.getEntityKey(entity))) {
                continue;
            }

            auto &transform = gCoordinator.getComponent<Transform>(entity);
            if (gCoordinator.hasComponent<Monster>(entity)) {
                monsters.emplace_back(entity, transform);
            }
            if (gCoordinator.hasComponent<Projectile>(entity)) {
                projectiles.emplace_back(entity, transform);
            }
        }

        // Check collisions
        for (const auto &[projectile, projectileTransform]: projectiles) {
            for (const auto &[monster, monsterTransform]: monsters) {
                // Skip if either entity has been destroyed
                if (!gCoordinator.getEntityIds().contains(gCoordinator.getEntityKey(projectile)) ||
                    !gCoordinator.getEntityIds().contains(gCoordinator.getEntityKey(monster))) {
                    continue;
                }

                float verticalDistance = std::abs(projectileTransform.y - monsterTransform.y);
                float horizontalDistance = std::abs(projectileTransform.x - monsterTransform.x);

                // Increased vertical check range
                if (verticalDistance < monsterTransform.h * 3 && horizontalDistance < monsterTransform.w * 2) {


                    // In ProjectileMonsterCollisionSystem::update()
                    if (checkCollision(projectileTransform, monsterTransform)) {


                        auto &monsterHealth = gCoordinator.getComponent<Monster>(monster);
                        auto &projectileComponent = gCoordinator.getComponent<Projectile>(projectile);

                        float previousHealth = monsterHealth.health;
                        monsterHealth.health -= projectileComponent.damage;



                        // Emit monster hit event
                        Event monsterHitEvent{
                            eventTypeToString(EventType::MonsterHit),
                            MonsterHitData{monster, projectile, projectileComponent.damage}
                        };
                        eventCoordinator.emit(std::make_shared<Event>(monsterHitEvent));

                        // Destroy projectile
                        gCoordinator.addComponent(projectile, Destroy{-1, true, false});

                        if (monsterHealth.health <= 0) {

                            // Emit monster defeated event
                            Event monsterDefeatedEvent{
                                eventTypeToString(EventType::MonsterDefeated),
                                {{"monsterId", monster}}
                            };
                            eventCoordinator.emit(std::make_shared<Event>(monsterDefeatedEvent));

                            // Add a slight delay before destroying the monster to allow for visual effects
                            gCoordinator.addComponent(monster, Destroy{-1, true, false});
                        }
                        break;
                    }
                }
            }
        }
    }
};

#endif
