//
// Created by Utsav Lal on 9/11/24.
//

#ifndef RANDOM_HPP
#define RANDOM_HPP

#include <random>

class Random {
public:
    static int generateRandomId() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> dis(1, 10000);
        return dis(gen);
    }
};

#endif //RANDOM_HPP
