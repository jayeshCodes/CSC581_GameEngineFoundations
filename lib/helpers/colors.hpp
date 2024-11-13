//
// Created by Utsav Lal on 9/23/24.
//

#ifndef COLORS_HPP
#define COLORS_HPP
#include <SDL_pixels.h>

inline std::random_device rd;
inline std::mt19937 gen(rd());


/**
 * This namespace helps us define colors for the engine
 */
namespace shade_color {
    constexpr SDL_Color Blue = {0, 0, 255, 255};
    constexpr SDL_Color Red = {255, 0, 0, 255};
    constexpr SDL_Color Green = {0, 255, 0, 255};
    constexpr SDL_Color Yellow = {255, 255, 0, 255};
    constexpr SDL_Color White = {255, 255, 255, 255};
    constexpr SDL_Color Black = {0, 0, 0, 255};
    constexpr SDL_Color Orange = {255, 165, 0, 255};
    constexpr SDL_Color Purple = {128, 0, 128, 255};
    constexpr SDL_Color Pink = {255, 192, 203, 255};
    constexpr SDL_Color Brown = {165, 42, 42, 255};
    constexpr SDL_Color Cyan = {0, 255, 255, 255};
    constexpr SDL_Color Gray = {128, 128, 128, 255};
    constexpr SDL_Color LightGray = {211, 211, 211, 255};
    constexpr SDL_Color DarkGray = {169, 169, 169, 255};
    constexpr SDL_Color LightBlue = {173, 216, 230, 255};
    constexpr SDL_Color LightGreen = {144, 238, 144, 255};
    constexpr SDL_Color LightYellow = {255, 255, 224, 255};

    static SDL_Color generateRandomSolidColor() {
        Uint8 r = static_cast<Uint8>(Random::generateRandomInt(0, 255));
        Uint8 g = static_cast<Uint8>(Random::generateRandomInt(0, 255));
        Uint8 b = static_cast<Uint8>(Random::generateRandomInt(0, 255));
        return {r, g, b, 255};
    }

    static SDL_Color generateNonRepeatingColor(SDL_Color color1, SDL_Color color2) {
        std::vector<SDL_Color> colors = {Blue, Red, Green};
        SDL_Color newColor;
        std::uniform_int_distribution<> dis(0, colors.size() - 1);
        do {
            newColor = colors[dis(gen)];
        } while (
            newColor.r == color1.r && newColor.g == color1.g && newColor.b == color1.b &&
                    newColor.r == color2.r && newColor.g == color2.g && newColor.b == color2.b
        );

        return newColor;
    }
}

#endif //COLORS_HPP
