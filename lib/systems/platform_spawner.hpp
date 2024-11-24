//
// Created by Jayesh Gajbhar on 11/22/24.
//

#ifndef PLATFORM_SPAWNER_HPP
#define PLATFORM_SPAWNER_HPP

#include "../model/components.hpp"
#include "../ECS/coordinator.hpp"
#include "../ECS/system.hpp"
#include "../core/structs.hpp"
#include "../helpers/random.hpp"
#include "../core/defs.hpp"
#include "../helpers/texture_manager.hpp"
#include <mutex>

extern Coordinator gCoordinator;

class PlatformSpawnerSystem : public System {
private:
    std::mutex spawnMutex;
    std::queue<std::pair<float, float>> platformsToSpawn; // Queue of (x,y) coordinates
    const float DESPAWN_OFFSET = 100.0f; // How far below camera view to destroy platforms

    void queuePlatformSpawn(float x, float y) {
        std::lock_guard<std::mutex> lock(spawnMutex);
        platformsToSpawn.push({x, y});
    }

    void createPlatform(float x, float y) {
        auto& spawner = gCoordinator.getComponent<PlatformSpawner>(*entities.begin());

        Sprite platformSprite;
        platformSprite.texturePath = spawner.texturePath;
        platformSprite.srcRect = {0, 0, spawner.platformWidth, spawner.platformHeight};
        platformSprite.scale = 1.0f;
        platformSprite.origin = {0, 0};
        platformSprite.flipX = false;
        platformSprite.flipY = false;

        Entity platform = gCoordinator.createEntity();

        gCoordinator.addComponent(platform, Transform{
            x,
            y,
            30.0f,
            120.0f,
            0
        });
        gCoordinator.addComponent(platform, platformSprite);
        gCoordinator.addComponent(platform, CKinematic{});
        gCoordinator.addComponent(platform, ClientEntity{0, false});
        gCoordinator.addComponent(platform, Destroy{});
        gCoordinator.addComponent(platform, Collision{false, true, CollisionLayer::OTHER});
        gCoordinator.addComponent(platform, VerticalBoost{});
        gCoordinator.addComponent(platform, RigidBody{-1.f});
    }

    float getRandomX(const PlatformSpawner& spawner) {
        return spawner.minX + (Random::generateRandomFloat(0, 1) * (spawner.maxX - spawner.minX));
    }

    void checkAndDestroyPlatforms(float cameraY) {
        for (auto& [id, entity] : gCoordinator.getEntityIds()) {
            // Check if this entity has the components we're interested in
            if (gCoordinator.hasComponent<Transform>(entity) &&
                gCoordinator.hasComponent<Destroy>(entity) &&
                gCoordinator.hasComponent<VerticalBoost>(entity)) {

                auto& transform = gCoordinator.getComponent<Transform>(entity);

                // If platform is below camera view plus offset, mark for destruction
                if (transform.y > cameraY + SCREEN_HEIGHT + DESPAWN_OFFSET) {
                    auto& destroy = gCoordinator.getComponent<Destroy>(entity);
                    destroy.destroy = true;
                    destroy.isSent = false;
                }
            }
        }
    }

public:
    void update(Entity cameraEntity) {
        if (entities.empty()) {

            return;
        }

        auto& spawner = gCoordinator.getComponent<PlatformSpawner>(*entities.begin());
        auto& camera = gCoordinator.getComponent<Camera>(cameraEntity);

        // Check for and destroy out-of-view platforms
        checkAndDestroyPlatforms(camera.y);

        // Debug print for camera position and next spawn position


        // Check if we need to spawn new platforms
        if (camera.y < spawner.lastCameraY - spawner.baseSpawnInterval/2) {
            float x = getRandomX(spawner);
            queuePlatformSpawn(x, spawner.nextSpawnY);

            float variation = Random::generateRandomFloat(-spawner.spawnVariation, spawner.spawnVariation);
            spawner.nextSpawnY -= (spawner.baseSpawnInterval + variation);
            spawner.lastCameraY = camera.y;
        }

        // Process queued platforms
        std::lock_guard<std::mutex> lock(spawnMutex);
        while (!platformsToSpawn.empty()) {
            auto [x, y] = platformsToSpawn.front();
            createPlatform(x, y);
            platformsToSpawn.pop();
        }
    }

    void init() {
        for (auto entity: entities) {
            auto& spawner = gCoordinator.getComponent<PlatformSpawner>(entity);

            spawner.baseSpawnInterval = 170.0f;
            spawner.spawnVariation = 20.0f;
            spawner.minX = 50.0f;
            spawner.maxX = SCREEN_WIDTH - 170.0f;
            spawner.texturePath = "assets/images/platform.png";
            spawner.nextSpawnY = SCREEN_HEIGHT + 200.f;
            spawner.lastCameraY = 0;

            spawner.texture = TextureManager::getInstance()->loadTexture(spawner.texturePath);
            SDL_QueryTexture(spawner.texture, nullptr, nullptr,
                &spawner.platformWidth, &spawner.platformHeight);

            // Queue initial platforms instead of creating them immediately
            for (int i = 0; i < 5; i++) {
                float y = spawner.nextSpawnY;
                float x = getRandomX(spawner);
                queuePlatformSpawn(x, y);

                float variation = Random::generateRandomFloat(-spawner.spawnVariation, spawner.spawnVariation);
                spawner.nextSpawnY -= (spawner.baseSpawnInterval + variation);
            }
        }
    }
};

#endif //PLATFORM_SPAWNER_HPP