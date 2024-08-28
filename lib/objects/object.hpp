//
// Created by Utsav Lal on 8/28/24.
//

#ifndef OBJECT_HPP
#define OBJECT_HPP
#include <SDL_rect.h>

class Object {
public:
    Object(SDL_Rect rect, SDL_Color color);

    virtual ~Object();

protected:
    SDL_Rect rect;
    SDL_Color color;
};

#endif //OBJECT_HPP
