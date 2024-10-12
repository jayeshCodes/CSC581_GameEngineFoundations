//
// Created by Utsav Lal on 10/2/24.
//

#include <cassert>
#include <unordered_map>
#include "types.hpp"
#include  "component_array.hpp"

class ComponentManager {
private:
    std::unordered_map<const char *, ComponentType> component_types{};
    std::unordered_map<const char *, std::shared_ptr<IComponentArray> > component_arrays{};

    ComponentType next_component_type{};

    template<typename T>
    std::shared_ptr<ComponentArray<T> > getComponentArray() {
        const char *typeName = typeid(T).name();

        assert(component_types.find(typeName) != component_types.end() && "Component not registered before use.");

        return std::static_pointer_cast<ComponentArray<T> >(component_arrays[typeName]);
    }

public:
    template<typename T>
    void registerComponent() {
        const char *typeName = typeid(T).name();

        assert(component_types.find(typeName) == component_types.end() && "Registering component type more than once.");

        component_types.insert({typeName, next_component_type});
        component_arrays.insert({typeName, std::make_shared<ComponentArray<T> >()});

        next_component_type++;
    }

    template<typename T>
    ComponentType getComponentType() {
        const char *typeName = typeid(T).name();

        assert(component_types.find(typeName) != component_types.end() && "Component not registered before use.");

        return component_types[typeName];
    }

    template<typename T>
    void addComponent(Entity entity, T component) {
        getComponentArray<T>()->insertData(entity, component);
    }

    template<typename T>
    void removeComponent(Entity entity) {
        getComponentArray<T>()->removeData(entity);
    }

    template<typename T>
    T &getComponent(Entity entity) {
        return getComponentArray<T>()->getData(entity);
    }

    template<typename T>
    bool hasComponent(Entity entity) {
        return getComponentArray<T>()->hasData(entity);
    }

    void entityDestroyed(Entity entity) {
        for (auto const &pair : component_arrays) {
            auto const &component = pair.second;

            component->entityDestroyed(entity);
        }
    }
};
