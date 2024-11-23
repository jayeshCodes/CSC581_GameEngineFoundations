//
// Created by Jayesh Gajbhar on 11/23/24.
//

#ifndef MONSTER_HIT_EFFECTS_HPP
#define MONSTER_HIT_EFFECTS_HPP

#include "../ECS/coordinator.hpp"
#include "../ECS/system.hpp"
#include "../model/components.hpp"
#include "../model/event.hpp"
#include "../EMS/event_coordinator.hpp"
#include <thread>

extern Coordinator gCoordinator;
extern EventCoordinator eventCoordinator;

class MonsterHitEffectsSystem : public System {
private:
    // Track monsters that are currently flashing
    std::unordered_map<Entity, std::pair<SDL_Color, float>> flashingMonsters;
    const float FLASH_DURATION = 0.2f; // Duration of flash in seconds

    EventHandler monsterHitHandler = [this](const std::shared_ptr<Event>& event) {
        if (event->type == eventTypeToString(EventType::MonsterHit)) {
            const MonsterHitData& data = event->data;

            std::cout << "Monster hit effect triggered for monster: " << data.monster << std::endl;

            if (gCoordinator.hasComponent<Sprite>(data.monster)) {
                auto& sprite = gCoordinator.getComponent<Sprite>(data.monster);
                std::cout << "Flashing monster sprite" << std::endl;
                // Store original sprite properties if needed
            }

            // Add to flashing monsters with red color and reset timer
            flashingMonsters[data.monster] = {SDL_Color{255, 0, 0, 255}, FLASH_DURATION};
        }

        if (event->type == eventTypeToString(EventType::MonsterDefeated)) {
            Entity monsterId = event->data["monsterId"];
            std::cout << "Monster defeated effect triggered for monster: " << monsterId << std::endl;

            // Create a visual effect for monster defeat (optional)
            // Could be particles, animation, etc.
        }
    };

public:
    MonsterHitEffectsSystem() {
        eventCoordinator.subscribe(monsterHitHandler, eventTypeToString(EventType::MonsterHit));
        eventCoordinator.subscribe(monsterHitHandler, eventTypeToString(EventType::MonsterDefeated));
    }

    ~MonsterHitEffectsSystem() {
        eventCoordinator.unsubscribe(monsterHitHandler, eventTypeToString(EventType::MonsterHit));
        eventCoordinator.unsubscribe(monsterHitHandler, eventTypeToString(EventType::MonsterDefeated));
    }

    void update(float dt) {
        // Update flashing effects
        auto it = flashingMonsters.begin();
        while (it != flashingMonsters.end()) {
            auto& [entity, flashData] = *it;
            auto& [flashColor, timeLeft] = flashData;

            if (!gCoordinator.getEntityIds().contains(gCoordinator.getEntityKey(entity))) {
                it = flashingMonsters.erase(it);
                continue;
            }

            timeLeft -= dt;

            // Update the monster's visual appearance
            if (gCoordinator.hasComponent<Sprite>(entity)) {
                auto& sprite = gCoordinator.getComponent<Sprite>(entity);
                // Update sprite tint based on flash state
                float flashIntensity = std::max(0.0f, timeLeft / FLASH_DURATION);
                // Apply tint to sprite (implementation depends on your rendering system)
            }

            if (timeLeft <= 0) {
                it = flashingMonsters.erase(it);
            } else {
                ++it;
            }
        }
    }
};

#endif