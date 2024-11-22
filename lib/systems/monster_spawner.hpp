//
// Created by Jayesh Gajbhar on 11/22/24.
//

#ifndef MONSTER_SPAWNER_HPP
#define MONSTER_SPAWNER_HPP

#include "../ECS/coordinator.hpp"
#include "../ECS/system.hpp"
#include "../helpers/random.hpp"
#include "../model/components.hpp"
#include <deque>

extern Coordinator gCoordinator;

class MonsterSpawnerSystem : public System {
private:
    const float SPAWN_INTERVAL_Y = 1000.0f;
    const float SPAWN_OFFSET_X = 100.0f;
    const int MAX_MONSTERS = 2;
    const float DESPAWN_OFFSET = 100.0f;      // Distance below camera view to despawn
    float lastSpawnY = 0.0f;
    std::deque<Entity> activeMonsters;
    const std::vector<std::string> monsterSprites = {
        "assets/images/monsters/1.png",
        "assets/images/monsters/2.png",
        "assets/images/monsters/3.png"
    };

    void spawnMonster(float x, float y) {
        Entity monster = gCoordinator.createEntity();

        float desiredHeight = SCREEN_HEIGHT / 11.0f;
        SDL_Texture* tempTexture = TextureManager::getInstance()->loadTexture(
            monsterSprites[Random::generateRandomInt(0, monsterSprites.size() - 1)]);

        int texWidth, texHeight;
        SDL_QueryTexture(tempTexture, nullptr, nullptr, &texWidth, &texHeight);
        float scale = desiredHeight / texHeight;
        float aspectRatio = static_cast<float>(texWidth) / texHeight;
        float desiredWidth = desiredHeight * aspectRatio;

        Sprite sprite;
        sprite.texturePath = monsterSprites[Random::generateRandomInt(0, monsterSprites.size() - 1)];
        sprite.srcRect = {0, 0, texWidth, texHeight};
        sprite.scale = scale;
        sprite.flipX = false;

        gCoordinator.addComponent(monster, Transform{x, y, desiredHeight, desiredWidth, 0});
        gCoordinator.addComponent(monster, sprite);
        gCoordinator.addComponent(monster, CKinematic{});
        gCoordinator.addComponent(monster, RigidBody{1.0f});
        gCoordinator.addComponent(monster, Collision{true, false, CollisionLayer::OTHER});
        gCoordinator.addComponent(monster, ClientEntity{0, true});
        gCoordinator.addComponent(monster, MovingPlatform{x, x +50.f , TO, 0, HORIZONTAL});
        gCoordinator.addComponent(monster, Destroy{});

        activeMonsters.push_back(monster);
        lastSpawnY = y;
    }

    void checkAndDestroyMonsters(float cameraY) {
        for (auto it = activeMonsters.begin(); it != activeMonsters.end();) {
            Entity monster = *it;

            if (gCoordinator.getEntityIds().contains(gCoordinator.getEntityKey(monster))) {
                auto& transform = gCoordinator.getComponent<Transform>(monster);

                // If monster is below camera view plus offset, destroy it
                if (transform.y > cameraY + SCREEN_HEIGHT + DESPAWN_OFFSET) {
                    gCoordinator.addComponent(monster, Destroy{-1, true, false});
                    it = activeMonsters.erase(it);
                } else {
                    ++it;
                }
            } else {
                it = activeMonsters.erase(it);
            }
        }
    }

public:
    void update(Entity mainCamera) {
        auto& camera = gCoordinator.getComponent<Camera>(mainCamera);
        float currentY = camera.y;

        // Check for and destroy out-of-view monsters
        checkAndDestroyMonsters(currentY);

        // Only spawn if we have less than maximum monsters
        if (activeMonsters.size() >= MAX_MONSTERS) {
            return;
        }

        // Check if we should spawn a new monster based on Y position
        if (currentY < lastSpawnY - SPAWN_INTERVAL_Y) {
            float newX = SCREEN_WIDTH/2 + Random::generateRandomFloat(-SPAWN_OFFSET_X, SPAWN_OFFSET_X);
            float newY = lastSpawnY - SPAWN_INTERVAL_Y;

            spawnMonster(newX, newY);
        }
    }

    void init() {
        lastSpawnY = SCREEN_HEIGHT - 100.0f;
    }
};

#endif