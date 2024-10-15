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

    static std::string generateRandomID(const int length) {
        static const char alphanum[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";
        std::random_device rd;
        std::mt19937 generator(rd());
        std::uniform_int_distribution<int> distribution(0, sizeof(alphanum) - 2);

        std::string id;
        for (int i = 0; i < length; ++i) {
            id += alphanum[distribution(generator)];
        }
        return id;
    }

};

#endif //RANDOM_HPP
