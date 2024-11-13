//
// Created by Utsav Lal on 11/13/24.
//

#pragma once


struct Ball {
    bool isLaunched = false;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Ball, isLaunched)
