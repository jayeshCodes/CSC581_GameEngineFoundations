//
// Created by Jayesh Gajbhar on 8/29/24.
//

#ifndef COLLISION_HPP
#define COLLISION_HPP

#include "physics.hpp"
#include "../../objects/object.hpp"
#include <memory>

class Collision : public Physics{
    public:
    Collision(){};

    void calculate(Object &character, std::vector<std::unique_ptr<Rectangle>> &objects) override;
};

#endif //COLLISION_HPP
