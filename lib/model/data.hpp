//
// Created by Utsav Lal on 10/8/24.
//

#ifndef DATA_HPP
#define DATA_HPP
#include <variant>

#include "components.hpp"
#include "../ECS/types.hpp"


namespace ShadeNetwork {

    using COMPONENTS = std::variant<Transform, Gravity, Color, CKinematic, Camera, KeyboardMovement, MovingPlatform>;

    enum Type {
        CREATE,
        DESTROY
    };


    struct Message {
        Type type;
        int entity{};
        std::vector<COMPONENTS> components{};
    };
}

#endif //DATA_HPP
