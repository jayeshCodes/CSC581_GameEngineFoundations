//
// Created by Utsav Lal on 10/2/24.
//

#ifndef ENTITYMANAGER_HPP
#define ENTITYMANAGER_HPP
#include <cassert>
#include <queue>

#include "types.hpp"

/**
 * This class helps manage the entities in the game engine
 * It will manage a queue of available entities and a signature array to keep track of the components
 */
class EntityManager {
private:
    std::queue<Entity> available_entities;
    std::array<Signature, MAX_ENTITIES> signatures{};
    uint32_t living_entity_count{};

public:
    EntityManager() {
        for (Entity entity = 0; entity < MAX_ENTITIES; ++entity) {
            available_entities.push(entity);
        }
    }

    Entity createEntity() {
        assert(living_entity_count < MAX_ENTITIES && "Too many entities in existence.");
        const Entity id = available_entities.front();
        available_entities.pop();
        living_entity_count++;
        return id;
    }

    void destroyEntity(Entity entity) {
        assert(entity < MAX_ENTITIES && "Entity out of range.");
        signatures[entity].reset();
        available_entities.push(entity);
        living_entity_count--;
    }

    void setSignature(Entity entity, Signature signature) {
        assert(entity < MAX_ENTITIES && "Entity out of range.");
        signatures[entity] = signature;
    }

    Signature getSignature(const Entity entity) const {
        assert(entity < MAX_ENTITIES && "Entity out of range.");
        return signatures[entity];
    }
};


#endif //ENTITYMANAGER_HPP
