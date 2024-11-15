//
// Created by Utsav Lal on 11/13/24.
//

#pragma once


struct Ball {
    bool isLaunched = false;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Ball, isLaunched)

struct Brick {
    bool isDestroyed = false;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Brick, isDestroyed)

struct Launcher {
    bool isLaunched = false;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Launcher, isLaunched)

enum PowerUpType {
    Elongate,
    Shorten,
    SpeedUp,
    SpeedDown,
};

inline std::vector PowerUpColors{shade_color::Purple, shade_color::LightGreen, shade_color::Brown, shade_color::Orange};

struct PowerUp {
    bool isCollected = false;
    PowerUpType type = Elongate;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PowerUp, isCollected)
