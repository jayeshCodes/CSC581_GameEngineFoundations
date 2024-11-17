//
// Created by Utsav Lal on 10/16/24.
//

#ifndef SEND_STRATEGY_HPP
#define SEND_STRATEGY_HPP

#include "../ECS/types.hpp"
#include "../enum/enum.hpp"
#include "../model/components.hpp"
#include "../model/data_model.hpp"


class Send_Strategy {
public:
    virtual ~Send_Strategy() = default;

    virtual std::string get_message(Entity entity, Message type) = 0;

    virtual SimpleMessage parse_message(zmq::message_t &message) = 0;

    virtual std::string copy_message(zmq::message_t &message) = 0;

    virtual Event parse_event(zmq::message_t &message) = 0;

    virtual std::string get_event(Event &event) = 0;
};

extern Coordinator gCoordinator;

class JSON_Strategy : public Send_Strategy {
public:
    std::string get_message(Entity entity, Message type) override {
        // Get the message
        SimpleMessage json_message{};
        nlohmann::json json;
        json_message.type = type;
        json_message.entity_key = gCoordinator.getEntityKey(entity);
        if (type == SYNC || type == DELETE) {
            json = json_message;
        }
        else if (type == CREATE) {
            if (gCoordinator.hasComponent<Transform>(entity)) {
                json_message.components.emplace_back(gCoordinator.getComponent<Transform>(entity));
            }
            if (gCoordinator.hasComponent<Color>(entity)) {
                json_message.components.emplace_back(gCoordinator.getComponent<Color>(entity));
            }
            if (gCoordinator.hasComponent<RigidBody>(entity)) {
                json_message.components.emplace_back(gCoordinator.getComponent<RigidBody>(entity));
            }
            if (gCoordinator.hasComponent<Collision>(entity)) {
                json_message.components.emplace_back(gCoordinator.getComponent<Collision>(entity));
            }
            if (gCoordinator.hasComponent<CKinematic>(entity)) {
                json_message.components.emplace_back(gCoordinator.getComponent<CKinematic>(entity));
            }
            if (gCoordinator.hasComponent<Destroy>(entity)) {
                json_message.components.emplace_back(gCoordinator.getComponent<Destroy>(entity));
            }
            json = json_message;
        }
        else if (type == UPDATE) {
            if (gCoordinator.hasComponent<Transform>(entity)) {
                json_message.components.emplace_back(gCoordinator.getComponent<Transform>(entity));
            }
            json = json_message;
        }
        return json.dump();
    }

    SimpleMessage parse_message(zmq::message_t &message) override {
        try {
            std::string jsonString(static_cast<char *>(message.data()), message.size());
            const nlohmann::json received_msg = nlohmann::json::parse(jsonString);
            SimpleMessage jMsg = received_msg;
            return jMsg;
        } catch (std::exception &e) {
            throw std::runtime_error("Error parsing JSON message");
        }
    }

    std::string copy_message(zmq::message_t &message) override {
        std::string jsonString(static_cast<char *>(message.data()), message.size());
        return jsonString;
    }

    std::string get_event(Event &event) override {
        nlohmann::json event_json;
        to_json(event_json, event);
        return event_json.dump();
    }

    Event parse_event(zmq::message_t &message) override {
        std::string jsonString(static_cast<char *>(message.data()), message.size());
        const nlohmann::json received_msg = nlohmann::json::parse(jsonString);
        Event event;
        from_json(received_msg, event);
        return event;
    }
};

#endif //SEND_STRATEGY_HPP
