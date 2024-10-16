//
// Created by Utsav Lal on 10/16/24.
//

#ifndef STRATEGY_SELECTOR_HPP
#define STRATEGY_SELECTOR_HPP
#include "send_strategy.hpp"

namespace Strategy {
    static std::unique_ptr<Send_Strategy> select_message_strategy(const std::string &messageFormat) {
        if (messageFormat == "json") {
            std::cout << "Sending message in JSON format" << std::endl;
            return std::make_unique<JSON_Strategy>();
        }
        std::cout << "Sending message in float format" << std::endl;
        return std::make_unique<Float_Strategy>();
    }
}

#endif //STRATEGY_SELECTOR_HPP
