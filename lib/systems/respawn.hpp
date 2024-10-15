//
// Created by Jayesh Gajbhar on 10/14/24.
//

#ifndef RESPAWN_HPP
#define RESPAWN_HPP

#include "../ECS/system.hpp"
#include "../model/components.hpp"
#include "../ECS/coordinator.hpp"

extern Coordinator gCoordinator;

class RespawnSystem : public System {
public:
    void update() {
        std::vector<Entity> entities = gCoordinator.getEntitiesWithComponent<Respawnable>();

        for (auto &entity: entities) {
            if (!hasRequiredComponents(entity)) {
                std::cerr << "Entity missing required components for respawn" << std::endl;
                return;
            }

            auto &transform = gCoordinator.getComponent<Transform>(entity);
            auto &respawnable = gCoordinator.getComponent<Respawnable>(entity);
            auto &collision = gCoordinator.getComponent<Collision>(entity);

            if (shouldRespawn(transform, respawnable)) {
                respawn(entity, transform, respawnable);
            }
        }
    }

private:
    const float DEATH_Y = SCREEN_HEIGHT;
    const float RESPAWN_HEIGHT = 10.f;

    bool shouldRespawn(Transform &transform, Respawnable &respawnable) {
        return transform.y > DEATH_Y || respawnable.isRespawn;
    }

    void respawn (Entity &entity, Transform &transform, Respawnable& respawnable) {
        transform.x = respawnable.lastSafePosition.x;
        transform.y = respawnable.lastSafePosition.y + RESPAWN_HEIGHT;

        // reset velocity
        if(gCoordinator.hasComponent<CKinematic>(entity)) {
            auto &kinematic = gCoordinator.getComponent<CKinematic>(entity);
            kinematic.velocity.x = 0;
            kinematic.velocity.y = 0;
            kinematic.acceleration.x = 0;
            kinematic.acceleration.y = 0;
        }

        respawnable.isRespawn = false;
    }

    void updateLastSafePosition (Entity &entity, Transform &transform, Respawnable &respawnable) {
        respawnable.lastSafePosition.x = transform.x;
        respawnable.lastSafePosition.y = transform.y;
    }

    bool hasRequiredComponents(Entity entity) {
        return gCoordinator.hasComponent<Transform>(entity) &&
               gCoordinator.hasComponent<Respawnable>(entity) &&
               gCoordinator.hasComponent<Collision>(entity);
    }
};

#endif //RESPAWN_HPP
