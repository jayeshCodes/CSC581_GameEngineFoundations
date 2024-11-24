//
// Created by Jayesh Gajbhar on 11/23/24.
//

#ifndef MONSTER_DEFEATED_HANDLER_HPP
#define MONSTER_DEFEATED_HANDLER_HPP

#include "../ECS/coordinator.hpp"
#include "../ECS/system.hpp"
#include "../model/components.hpp"
#include "../model/event.hpp"
#include "../EMS/event_coordinator.hpp"
#include "../helpers/font_helper.hpp"

extern Coordinator gCoordinator;
extern EventCoordinator eventCoordinator;
extern std::string mainCharID;

struct DeathAnimation {
    float duration;
    float currentTime;
    float alpha;
    float initialWidth;
    float initialHeight;
    SDL_Color color;
};

class MonsterDefeatedHandler : public System {
private:
    std::unordered_map<Entity, DeathAnimation> dyingMonsters;
    const float DEATH_ANIMATION_DURATION = 0.5f;
    const int POINTS_PER_MONSTER = 100;

    void handleMonsterDefeat(Entity monster) {
        std::cout << "Monster defeated: " << monster << std::endl;

        // Get monster's current color
        SDL_Color monsterColor = {255, 255, 255, 255}; // Default white
        if (gCoordinator.hasComponent<Color>(monster)) {
            monsterColor = gCoordinator.getComponent<Color>(monster).color;
        }

        // Store initial dimensions
        float initialWidth = 0;
        float initialHeight = 0;
        if (gCoordinator.hasComponent<Transform>(monster)) {
            auto &transform = gCoordinator.getComponent<Transform>(monster);
            initialWidth = transform.w;
            initialHeight = transform.h;
        }

        // Add death animation
        dyingMonsters[monster] = DeathAnimation{
            DEATH_ANIMATION_DURATION,
            0.0f,
            1.0f,
            initialWidth,
            initialHeight,
            monsterColor
        };

        // Disable collision immediately
        if (gCoordinator.hasComponent<Collision>(monster)) {
            gCoordinator.removeComponent<Collision>(monster);
        }

        // Set destroy component
        if (gCoordinator.hasComponent<Destroy>(monster)) {
            auto &destroy = gCoordinator.getComponent<Destroy>(monster);
            destroy.destroy = true;
            destroy.isSent = false; // Ensure it gets synced over network
        } else {
            gCoordinator.addComponent(monster, Destroy{-1, true, false});
        }

        // Update player's score if it exists
        for (auto &[id, entity]: gCoordinator.getEntityIds()) {
            if (id == mainCharID && gCoordinator.hasComponent<Score>(entity)) {
                auto &score = gCoordinator.getComponent<Score>(entity);
                score.score += POINTS_PER_MONSTER;
                std::cout << "Updated score: " << score.score << std::endl;
                break;
            }
        }
    }

    void createScorePopup(Entity monster) {
        if (!gCoordinator.hasComponent<Transform>(monster)) {
            return;
        }

        auto &monsterTransform = gCoordinator.getComponent<Transform>(monster);
        Entity popup = gCoordinator.createEntity();

        std::string pointsText = "+" + std::to_string(POINTS_PER_MONSTER);

        gCoordinator.addComponent(popup, Transform{
                                      monsterTransform.x + monsterTransform.w / 2,
                                      monsterTransform.y,
                                      20,
                                      60,
                                      0
                                  });

        gCoordinator.addComponent(popup, CKinematic{
                                      {0, -50},
                                      0,
                                      {0, 0},
                                      0
                                  });

        gCoordinator.addComponent(popup, Color{{255, 255, 0, 255}});
        gCoordinator.addComponent(popup, ScorePopup{1.0f, 0.0f, pointsText});
    }

    EventHandler monsterDefeatedHandler = [this](const std::shared_ptr<Event> &event) {
        if (event->type == eventTypeToString(EventType::MonsterDefeated)) {
            Entity monsterId = event->data["monsterId"];
            handleMonsterDefeat(monsterId);
        }
    };

public:
    MonsterDefeatedHandler() {
        eventCoordinator.subscribe(monsterDefeatedHandler, eventTypeToString(EventType::MonsterDefeated));
    }

    ~MonsterDefeatedHandler() {
        eventCoordinator.unsubscribe(monsterDefeatedHandler, eventTypeToString(EventType::MonsterDefeated));
    }

    void update(float dt) {
        // Update dying monsters
        auto it = dyingMonsters.begin();
        while (it != dyingMonsters.end()) {
            auto &[entity, anim] = *it;

            if (!gCoordinator.getEntityIds().contains(gCoordinator.getEntityKey(entity))) {
                it = dyingMonsters.erase(it);
                continue;
            }

            anim.currentTime += dt;
            float progress = anim.currentTime / anim.duration;

            if (progress >= 1.0f) {
                // Animation finished, destroy the monster
                gCoordinator.destroyEntity(entity);
                it = dyingMonsters.erase(it);
                continue;
            }

            // Update visual effects
            if (gCoordinator.hasComponent<Transform>(entity)) {
                auto &transform = gCoordinator.getComponent<Transform>(entity);

                // Scale effect using initial dimensions
                float scale = 1.0f + (progress * 0.5f); // Expand by 50%
                transform.w = anim.initialWidth * scale;
                transform.h = anim.initialHeight * scale;

                // Center the scaling
                transform.x -= (transform.w - anim.initialWidth) / 2;
                transform.y -= (transform.h - anim.initialHeight) / 2;

                // Fade out effect
                if (gCoordinator.hasComponent<Color>(entity)) {
                    auto &color = gCoordinator.getComponent<Color>(entity);
                    color.color.a = static_cast<Uint8>((1.0f - progress) * 255);
                }

                // Add spin effect
                if (gCoordinator.hasComponent<CKinematic>(entity)) {
                    auto &kinematic = gCoordinator.getComponent<CKinematic>(entity);
                    kinematic.rotation = progress * 360.0f;
                }
            }

            ++it;
        }
    }
};

#endif
