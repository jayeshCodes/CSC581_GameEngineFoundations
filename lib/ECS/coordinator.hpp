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
#include "../helpers/random.hpp"

class Coordinator {
private:
    std::unique_ptr<ComponentManager> component_manager;
    std::unique_ptr<EntityManager> entity_manager;
    std::unique_ptr<SystemManager> system_manager;
    std::unordered_map<std::string, Entity> entities;
    mutable std::shared_mutex mutex;

    Snapshot snapshot{};

public:
    void init() {
        component_manager = std::make_unique<ComponentManager>();
        entity_manager = std::make_unique<EntityManager>();
        system_manager = std::make_unique<SystemManager>();
        snapshot.reserve(MAX_ENTITIES);
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

        entities.erase(getEntityKey(entity));
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

    // get all the entities with a certain component type
    template<typename T>
    std::vector<Entity> getEntitiesWithComponent() const {
        std::shared_lock lock(mutex);
        std::vector<Entity> entitiesWithComponent;

        for (const auto &[key, entity]: entities) {
            if (component_manager->hasComponent<T>(entity)) {
                entitiesWithComponent.push_back(entity);
            }
        }

        return entitiesWithComponent;
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
        for (auto &[key, entity]: entities) {
            //if key starts with key
            if (key.find(searchTerm) == 0) {
                ans.push_back(entity);
            }
        }
        return ans;
    }

    std::string getEntityKey(const Entity id) {
        for (auto &[key, value]: entities) {
            if (value == id) {
                return key;
            }
        }
        return "";
    }

    static std::string createKey(Entity id) {
        return Random::generateRandomID(12);
    }

    EntitySnapshot createSnapshot(const Entity entity, const std::string id) const {
        EntitySnapshot snapshot{};
        snapshot.entity = entity;
        snapshot.id = id;
        if (component_manager->hasComponent<Transform>(entity)) {
            snapshot.components.emplace_back(component_manager->getComponent<Transform>(entity));
        }
        if (component_manager->hasComponent<Color>(entity)) {
            snapshot.components.emplace_back(component_manager->getComponent<Color>(entity));
        }
        if (component_manager->hasComponent<RigidBody>(entity)) {
            snapshot.components.emplace_back(component_manager->getComponent<RigidBody>(entity));
        }
        if (component_manager->hasComponent<Gravity>(entity)) {
            snapshot.components.emplace_back(component_manager->getComponent<Gravity>(entity));
        }
        if (component_manager->hasComponent<CKinematic>(entity)) {
            snapshot.components.emplace_back(component_manager->getComponent<CKinematic>(entity));
        }
        if (component_manager->hasComponent<Jump>(entity)) {
            snapshot.components.emplace_back(component_manager->getComponent<Jump>(entity));
        }
        if (component_manager->hasComponent<Respawnable>(entity)) {
            snapshot.components.emplace_back(component_manager->getComponent<Respawnable>(entity));
        }
        if (component_manager->hasComponent<Camera>(entity)) {
            snapshot.components.emplace_back(component_manager->getComponent<Camera>(entity));
        }
        if (component_manager->hasComponent<VerticalBoost>(entity)) {
            snapshot.components.emplace_back(component_manager->getComponent<VerticalBoost>(entity));
        }
        if (component_manager->hasComponent<Stomp>(entity)) {
            snapshot.components.emplace_back(component_manager->getComponent<Stomp>(entity));
        }
        if (component_manager->hasComponent<Dash>(entity)) {
            snapshot.components.emplace_back(component_manager->getComponent<Dash>(entity));
        }
        if (component_manager->hasComponent<Destroy>(entity)) {
            snapshot.components.emplace_back(component_manager->getComponent<Destroy>(entity));
        }
        if (component_manager->hasComponent<MovingPlatform>(entity)) {
            snapshot.components.emplace_back(component_manager->getComponent<MovingPlatform>(entity));
        }
        if (component_manager->hasComponent<Collision>(entity)) {
            snapshot.components.emplace_back(component_manager->getComponent<Collision>(entity));
        }
        if (component_manager->hasComponent<Receiver>(entity)) {
            snapshot.components.emplace_back(component_manager->getComponent<Receiver>(entity));
        }
        if (component_manager->hasComponent<ClientEntity>(entity)) {
            snapshot.components.emplace_back(component_manager->getComponent<ClientEntity>(entity));
        }
        if (component_manager->hasComponent<KeyboardMovement>(entity)) {
            snapshot.components.emplace_back(component_manager->getComponent<KeyboardMovement>(entity));
        }
        return snapshot;
    }

    void backup() {
        this->snapshot.clear();
        for (auto &[id, entity]: entities) {
            this->snapshot.emplace_back(createSnapshot(entity, id));
        }
    }

    void restoreEntity(const EntitySnapshot &entitySnap) {
        createEntity(entitySnap.id);
        for (auto &component: entitySnap.components) {
            if (std::holds_alternative<Transform>(component)) {
                addComponent(entitySnap.entity, std::get<Transform>(component));
                auto &transform = getComponent<Transform>(entitySnap.entity);
                transform = std::get<Transform>(component);
            }
            if (std::holds_alternative<Color>(component)) {
                addComponent(entitySnap.entity, std::get<Color>(component));
                auto &color = getComponent<Color>(entitySnap.entity);
                color = std::get<Color>(component);
            }
            if (std::holds_alternative<RigidBody>(component)) {
                addComponent(entitySnap.entity, std::get<RigidBody>(component));
                auto &rigidBody = getComponent<RigidBody>(entitySnap.entity);
                rigidBody = std::get<RigidBody>(component);
            }
            if (std::holds_alternative<Gravity>(component)) {
                addComponent(entitySnap.entity, std::get<Gravity>(component));
                auto &gravity = getComponent<Gravity>(entitySnap.entity);
                gravity = std::get<Gravity>(component);
            }
            if (std::holds_alternative<CKinematic>(component)) {
                addComponent(entitySnap.entity, std::get<CKinematic>(component));
                auto &cKinematic = getComponent<CKinematic>(entitySnap.entity);
                cKinematic = std::get<CKinematic>(component);
            }
            if (std::holds_alternative<Jump>(component)) {
                addComponent(entitySnap.entity, std::get<Jump>(component));
                auto &jump = getComponent<Jump>(entitySnap.entity);
                jump = std::get<Jump>(component);
            }
            if (std::holds_alternative<Respawnable>(component)) {
                addComponent(entitySnap.entity, std::get<Respawnable>(component));
                auto &respawnable = getComponent<Respawnable>(entitySnap.entity);
                respawnable = std::get<Respawnable>(component);
            }
            if (std::holds_alternative<Camera>(component)) {
                addComponent(entitySnap.entity, std::get<Camera>(component));
                auto &camera = getComponent<Camera>(entitySnap.entity);
                camera = std::get<Camera>(component);
            }
            if (std::holds_alternative<VerticalBoost>(component)) {
                addComponent(entitySnap.entity, std::get<VerticalBoost>(component));
                auto &verticalBoost = getComponent<VerticalBoost>(entitySnap.entity);
                verticalBoost = std::get<VerticalBoost>(component);
            }
            if (std::holds_alternative<Stomp>(component)) {
                addComponent(entitySnap.entity, std::get<Stomp>(component));
                auto &stomp = getComponent<Stomp>(entitySnap.entity);
                stomp = std::get<Stomp>(component);
            }
            if (std::holds_alternative<Dash>(component)) {
                addComponent(entitySnap.entity, std::get<Dash>(component));
                auto &dash = getComponent<Dash>(entitySnap.entity);
                dash = std::get<Dash>(component);
            }
            if (std::holds_alternative<Destroy>(component)) {
                addComponent(entitySnap.entity, std::get<Destroy>(component));
                auto &destroy = getComponent<Destroy>(entitySnap.entity);
                destroy = std::get<Destroy>(component);
            }
            if (std::holds_alternative<MovingPlatform>(component)) {
                addComponent(entitySnap.entity, std::get<MovingPlatform>(component));
                auto &movingPlatform = getComponent<MovingPlatform>(entitySnap.entity);
                movingPlatform = std::get<MovingPlatform>(component);
            }
            if (std::holds_alternative<Collision>(component)) {
                addComponent(entitySnap.entity, std::get<Collision>(component));
                auto &collision = getComponent<Collision>(entitySnap.entity);
                collision = std::get<Collision>(component);
            }
            if (std::holds_alternative<ClientEntity>(component)) {
                addComponent(entitySnap.entity, std::get<ClientEntity>(component));
                auto &clientEntity = getComponent<ClientEntity>(entitySnap.entity);
                clientEntity = std::get<ClientEntity>(component);
            }
            if (std::holds_alternative<KeyboardMovement>(component)) {
                addComponent(entitySnap.entity, std::get<KeyboardMovement>(component));
                auto &keyboardMovement = getComponent<KeyboardMovement>(entitySnap.entity);
                keyboardMovement = std::get<KeyboardMovement>(component);
            }
            if (std::holds_alternative<Receiver>(component)) {
                addComponent(entitySnap.entity, std::get<Receiver>(component));
                auto &receiver = getComponent<Receiver>(entitySnap.entity);
                receiver = std::get<Receiver>(component);
            }
        }
    }

    void restore() {
        for (auto &entitySnap: this->snapshot) {
            restoreEntity(entitySnap);
        }
    }
};
