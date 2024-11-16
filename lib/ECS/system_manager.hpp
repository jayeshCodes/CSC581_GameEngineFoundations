//
// Created by Utsav Lal on 10/2/24.
//

#pragma once
#include <cassert>
#include <unordered_map>

#include "types.hpp"
#include "system.hpp"


class SystemManager {
private:
    std::unordered_map<const char*, Signature> signatures{};
    std::unordered_map<const char*, std::shared_ptr<System>> systems{};

public:
    template<typename T>
    std::shared_ptr<T> registerSystem() {
        const char* typeName = typeid(T).name();
        assert(systems.find(typeName) == systems.end() && "Registering system more than once.");

        auto system = std::make_shared<T>();
        systems.insert({typeName, system});
        return system;
    }

    template<typename T>
    std::shared_ptr<T> getSystem() {
        const char* typeName = typeid(T).name();
        auto it = systems.find(typeName);
        if(it == systems.end()) {
            return nullptr;
        }

        return std::static_pointer_cast<T>(systems[typeName]);
    }

    template<typename T>
    void setSignature(Signature signature) {
        const char* typeName = typeid(T).name();
        assert(systems.find(typeName) != systems.end() && "System used before registered.");

        signatures.insert({typeName, signature});
    }

    void entityDestroyed(Entity entity) {
        for (auto const& pair : systems) {
            auto const& system = pair.second;

            system->entities.erase(entity);
        }
    }

    void entitySignatureChanged(Entity entity, Signature entitySignature) {
        for (auto const& pair : systems) {
            auto const& type = pair.first;
            auto const& system = pair.second;
            auto const& systemSignature = signatures[type];

            if ((entitySignature & systemSignature) == systemSignature) {
                system->entities.insert(entity);
            } else {
                system->entities.erase(entity);
            }
        }
    }
};
