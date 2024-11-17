//
// Created by Utsav Lal on 11/16/24.
//

#pragma once
#include <vector>

namespace SnakeQuantizer {
    inline std::vector<int> quantize(const float x, float y, const int length) {
        return {static_cast<int>(x / length), static_cast<int>(y / length)};
    }

    inline std::vector<float> dequantize(const int row, int col, int length) {
        return {static_cast<float>(col * length), static_cast<float>(row * length)};
    }
}
