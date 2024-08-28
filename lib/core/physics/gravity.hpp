//
// Created by Jayesh Gajbhar on 8/28/24.
//

#ifndef GRAVITY_HPP
#define GRAVITY_HPP

#include "physics.hpp"
#include "../../objects/object.hpp"

class Gravity : public Physics {
  public:
    Gravity(float gravityX, float gravityY) {
        this->gravityX = gravityX;
        this->gravityY = gravityY;
    };

    void calculate(Object &character) override;


    private:
      float gravityX;
      float gravityY;

};

#endif //GRAVITY_HPP
