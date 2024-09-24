//
// Created by Utsav Lal on 8/31/24.
//

#ifndef MANAGER_HPP
#define MANAGER_HPP
#include <atomic>

#include "../generic/singleton.hpp"

class GameManager : public Singleton<GameManager> {
public:
    friend class Singleton;

private:
    GameManager() {
        scaleWithScreenSize = false;
        gameRunning = false;
        isServer = false;
    }

public:
    bool scaleWithScreenSize;
    std::atomic<bool> gameRunning{false};
    bool isServer;
};

#endif //MANAGER_HPP
