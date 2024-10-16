//
// Created by Utsav Lal on 10/16/24.
//

#ifndef SEND_STRATEGY_HPP
#define SEND_STRATEGY_HPP
#include <variant>

#include "../ECS/types.hpp"
#include "../enum/message_type.hpp"
#include "../model/components.hpp"

class Send_Strategy {
public:
    virtual ~Send_Strategy() = default;

    virtual std::variant<std::vector<float>, std::string> get_message(Entity entity, Transform &transform, Color &color,
                                                                      Message messageType)
    = 0;

    virtual std::tuple<Message, Transform, Color> parse_message(zmq::message_t &message) = 0;

    virtual std::variant<std::vector<char>, std::vector<float> > copy_message(zmq::message_t &message) = 0;
};

class Float_Strategy : public Send_Strategy {
public:
    std::variant<std::vector<float>, std::string> get_message(Entity entity, Transform &transform, Color &color,
                                                              Message messageType) final {
        std::vector<float> message;
        message.reserve(10);
        message.emplace_back(messageType);
        message.emplace_back(entity);
        message.emplace_back(transform.x);
        message.emplace_back(transform.y);
        message.emplace_back(transform.h);
        message.emplace_back(transform.w);
        message.emplace_back(color.color.r);
        message.emplace_back(color.color.g);
        message.emplace_back(color.color.b);
        message.emplace_back(color.color.a);
        return message;
    }

    std::tuple<Message, Transform, Color> parse_message(zmq::message_t &message) final {
        const std::vector<float> received_msg(static_cast<float *>(message.data()),
                                              static_cast<float *>(message.data()) + message.size() / sizeof
                                              (float));
        auto messageType = static_cast<Message>(received_msg[0]);
        Transform transform{};
        Color color{};
        transform.x = received_msg[2];
        transform.y = received_msg[3];
        transform.h = received_msg[4];
        transform.w = received_msg[5];
        color.color.r = static_cast<Uint8>(received_msg[6]);
        color.color.g = static_cast<Uint8>(received_msg[7]);
        color.color.b = static_cast<Uint8>(received_msg[8]);
        color.color.a = static_cast<Uint8>(received_msg[9]);
        return std::make_tuple(messageType, transform, color);
    }

    std::variant<std::vector<char>, std::vector<float> > copy_message(zmq::message_t &message) override {
        std::vector received_msg(static_cast<float *>(message.data()),
                                 static_cast<float *>(message.data()) + message.size() /
                                 sizeof
                                 (float));
        return received_msg;
    }
};

class JSON_Strategy : public Send_Strategy {
public:
    std::variant<std::vector<float>, std::string> get_message(Entity entity, Transform &transform, Color &color,
                                                              Message messageType) override {
        // Get the message
        nlohmann::json message;
        message["m"] = messageType;
        message["e"] = entity;
        message["t"]["x"] = std::round(transform.x * 100) / 100.0;
        message["t"]["y"] = std::round(transform.y * 100) / 100.0;
        message["t"]["h"] = std::round(transform.h * 100) / 100.0;
        message["t"]["w"] = std::round(transform.w * 100) / 100.0;
        message["c"]["r"] = color.color.r;
        message["c"]["g"] = color.color.g;
        message["c"]["b"] = color.color.b;
        message["c"]["a"] = color.color.a;
        return message.dump();
    }

    std::tuple<Message, Transform, Color> parse_message(zmq::message_t &message) override {
        // parse message
        try {
            std::string jsonString(static_cast<char *>(message.data()), message.size());
            nlohmann::json received_msg = nlohmann::json::parse(jsonString);
            auto messageType = static_cast<Message>(received_msg["m"].get<int>());
            Transform transform{};
            Color color{};
            transform.x = received_msg["t"]["x"].get<float>();
            transform.y = received_msg["t"]["y"].get<float>();
            transform.h = received_msg["t"]["h"].get<float>();
            transform.w = received_msg["t"]["w"].get<float>();
            color.color.r = static_cast<Uint8>(received_msg["c"]["r"].get<int>());
            color.color.g = static_cast<Uint8>(received_msg["c"]["g"].get<int>());
            color.color.b = static_cast<Uint8>(received_msg["c"]["b"].get<int>());
            color.color.a = static_cast<Uint8>(received_msg["c"]["a"].get<int>());
            return std::make_tuple(messageType, transform, color);
        } catch (std::exception &e) {
            throw std::runtime_error("Error parsing JSON message");
        }
    }

    std::variant<std::vector<char>, std::vector<float> > copy_message(zmq::message_t &message) override {
        std::vector received_msg(static_cast<char *>(message.data()),
                                 static_cast<char *>(message.data()) + message.size() /
                                 sizeof
                                 (char));
        return received_msg;
    }
};

#endif //SEND_STRATEGY_HPP
