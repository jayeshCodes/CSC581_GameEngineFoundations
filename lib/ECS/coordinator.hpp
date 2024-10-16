//
// Created by Utsav Lal on 10/2/24.
//

#pragma once
#include <memory>
#include <unordered_map>
#include <shared_mutex>

#include "component_manager.hpp"
#include "entity_manager.hpp"
#include "system_manager.hpp"

class Coordinator {
private:
    std::unique_ptr<ComponentManager> component_manager;
    std::unique_ptr<EntityManager> entity_manager;
    std::unique_ptr<SystemManager> system_manager;
    std::unordered_map<std::string, Entity> entities;
    mutable std::shared_mutex mutex;

public:
    void init() {
        component_manager = std::make_unique<ComponentManager>();
        entity_manager = std::make_unique<EntityManager>();
        system_manager = std::make_unique<SystemManager>();
    }

    Entity createEntity() {
        std::lock_guard<std::shared_mutex> lock(mutex);
        const Entity id = entity_manager->createEntity();
        entities[createKey(id)] = id;
        return id;
    }

    Entity createEntity(const std::string &key) {
        std::lock_guard<std::shared_mutex> lock(mutex);
        if (entities.contains(key)) {
            return entities[key];
        }
        const Entity id = entity_manager->createEntity();
        entities[key] = id;
        return id;
    }

    void destroyEntity(Entity entity) {
        std::lock_guard<std::shared_mutex> lock(mutex);
        entity_manager->destroyEntity(entity);

        component_manager->entityDestroyed(entity);
        system_manager->entityDestroyed(entity);

        entities.erase(createKey(entity));
    }

    template<typename T>
    void registerComponent() const {
        std::lock_guard<std::shared_mutex> lock(mutex);
        component_manager->registerComponent<T>();
    }

    template<typename T>
    void addComponent(Entity entity, T component) const {
        std::lock_guard<std::shared_mutex> lock(mutex);
        if (component_manager->hasComponent<T>(entity)) {
            return;
        }
        component_manager->addComponent<T>(entity, component);

        auto signature = entity_manager->getSignature(entity);
        signature.set(component_manager->getComponentType<T>(), true);
        entity_manager->setSignature(entity, signature);

        system_manager->entitySignatureChanged(entity, signature);
    }

    template<typename T>
    void removeComponent(Entity entity) const {
        std::lock_guard<std::shared_mutex> lock(mutex);
        component_manager->removeComponent<T>(entity);

        auto signature = entity_manager->getSignature(entity);
        signature.set(component_manager->getComponentType<T>(), false);
        entity_manager->setSignature(entity, signature);

        system_manager->entitySignatureChanged(entity, signature);
    }

    template<typename T>
    T &getComponent(Entity entity) const {
        std::shared_lock lock(mutex);
        return component_manager->getComponent<T>(entity);
    }

    template<typename T>
    bool hasComponent(Entity entity) const {
        std::shared_lock lock(mutex);
        return component_manager->hasComponent<T>(entity);
    }

    template<typename T>
    ComponentType getComponentType() const {
        std::shared_lock lock(mutex);
        return component_manager->getComponentType<T>();
    }

    template<typename T>
    std::shared_ptr<T> registerSystem() const {
        std::lock_guard<std::shared_mutex> lock(mutex);
        return system_manager->registerSystem<T>();
    }

    template<typename T>
    void setSystemSignature(Signature signature) const {
        std::lock_guard<std::shared_mutex> lock(mutex);
        system_manager->setSignature<T>(signature);
    }

    std::unordered_map<std::string, Entity> &getEntityIds() {
        std::shared_lock lock(mutex);
        return entities;
    }

    std::vector<Entity> getEntitiesStartsWith(std::string searchTerm) {
        std::vector<Entity> ans;
        ans.reserve(MAX_ENTITIES);
        for(auto &[key, entity]: entities) {
            //if key starts with key
            if(key.find(searchTerm) == 0) {
                ans.push_back(entity);
            }
        }
        return ans;
    }

    std::string getEntityKey(const Entity id) {
        std::shared_lock lock(mutex);
        for(auto &[key, value]: entities) {
            if(value == id) {
                return key;
            }
        }
        return "";
    }

    static std::string createKey(Entity id) {
        return "Entity" + std::to_string(id);
    }
};
