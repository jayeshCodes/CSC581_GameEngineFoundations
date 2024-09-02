//
// Created by Utsav Lal on 9/2/24.
//

#ifndef HORIZONTALMOVEMENTCONTROLLER_HPP
#define HORIZONTALMOVEMENTCONTROLLER_HPP
#include "../../objects/object.hpp"

class MoveBetween2Points {
public:
    MoveBetween2Points(float point1, float point2, bool movingLeft);

    void moveBetween2Points(Object &object);

private:
    float left;
    float right;
    bool movingLeft;
};

#endif //HORIZONTALMOVEMENTCONTROLLER_HPP
