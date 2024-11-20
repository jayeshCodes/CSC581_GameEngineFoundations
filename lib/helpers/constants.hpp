//
// Created by Utsav Lal on 9/23/24.
//

#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

namespace engine_constants {
    constexpr float FRAME_RATE = 1.f / 60.f;
    constexpr int SERVER_CONNECT_PORT = 5555;
}

namespace bubble_constants {
    constexpr float GRID_SIZE = 32.0f;
    constexpr float GRID_COLS = 16;
    constexpr float GRID_OFFSET_X = (SCREEN_WIDTH - (GRID_COLS * GRID_SIZE)) / 2.0f;
    constexpr float GRID_OFFSET_Y = 32.0f;
}

#endif //CONSTANTS_HPP
