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

    static int generateRandomInt(const int min, const int max) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(min, max);
        return dis(gen);
    }

    static double generateRandomFloat(const float min, const float max) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_real_distribution<> dis(min, max);
        return std::round(dis(gen) * 100.0) / 100.0;
    }

    static int generateRandom185percent() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::bernoulli_distribution dis(0.15);
        return dis(gen);
    }
};

#endif //RANDOM_HPP
