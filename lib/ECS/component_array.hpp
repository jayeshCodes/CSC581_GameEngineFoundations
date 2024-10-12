//
// Created by Utsav Lal on 10/2/24.
//

#pragma once
#include <cassert>
#include <unordered_map>

#include "types.hpp"

class IComponentArray {
public:
    virtual ~IComponentArray() = default;

    virtual void entityDestroyed(Entity entity) = 0;
};

/**
 * The main idea of this class is to make sure that components are always stored in a packed array.
 * For example if we have 3 components [1,2,3] and we remove 2 then the array will look like [1,3,3]
 * and the size variable will keep track of the number of components in the array
 * @tparam T The individual component type
 */
template<typename T>
class ComponentArray : public IComponentArray {
private:
    std::array<T, MAX_ENTITIES> component_array;
    std::unordered_map<Entity, size_t> entity_to_index_map;
    std::unordered_map<size_t, Entity> index_to_entity_map;
    size_t size = 0;

public:
    void insertData(Entity entity, T component) {
        assert(
            entity_to_index_map.find(entity) == entity_to_index_map.end() &&
            "Component added to same entity more than once.");

        size_t new_index = size;
        entity_to_index_map[entity] = new_index;
        index_to_entity_map[new_index] = entity;
        component_array[new_index] = component;
        size++;
    }

    void removeData(Entity entity) {
        assert(entity_to_index_map.find(entity) != entity_to_index_map.end() &&
            "Removing non-existent component.");
        size_t index = entity_to_index_map[entity];
        size_t last_index = size - 1;
        component_array[index] = component_array[last_index];

        const Entity entity_of_last_element = index_to_entity_map[last_index];
        entity_to_index_map[entity_of_last_element] = index;
        index_to_entity_map[index] = entity_of_last_element;

        entity_to_index_map.erase(entity);
        index_to_entity_map.erase(last_index);

        size--;
    }

    T &getData(Entity entity) {
        assert(entity_to_index_map.find(entity) != entity_to_index_map.end() &&
            "Retrieving non-existent component.");
        return component_array[entity_to_index_map[entity]];
    }

    bool hasData(Entity entity) {
        return entity_to_index_map.find(entity) != entity_to_index_map.end();
    }

    void entityDestroyed(Entity entity) override {
        if (entity_to_index_map.find(entity) != entity_to_index_map.end()) {
            removeData(entity);
        }
    }
};
