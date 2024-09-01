//
// Created by Utsav Lal on 8/31/24.
//

#ifndef MANAGER_HPP
#define MANAGER_HPP
#include "../generic/singleton.hpp"

class GameManager : public Singleton<GameManager> {
public:
    friend class Singleton;

private:
    GameManager() {
        scaleWithScreenSize = false;
    }

public:
    bool scaleWithScreenSize;
};

#endif //MANAGER_HPP
