//
// Created by Utsav Lal on 11/13/24.
//

#pragma once
#include "../../../lib/ECS/coordinator.hpp"
#include "../../../lib/EMS/types.hpp"
#include "../../../lib/strategy/send_strategy.hpp"
#include "../model/component.hpp"
#include "../model/data_model.hpp"

extern Coordinator gCoordinator;

class BrickBreakerStrategy : public Send_Strategy {
public:
    std::string get_message(Entity entity, Message type) override {
        // Get the message
        BBMessage json_message{};
        nlohmann::json json;
        json_message.type = type;
        json_message.entity_key = gCoordinator.getEntityKey(entity);
        if (type == SYNC || type == DELETE) {
            json = json_message;
        } else if (type == CREATE) {
            if (gCoordinator.hasComponent<Transform>(entity)) {
                nlohmann::json component_json = gCoordinator.getComponent<Transform>(entity);
                component_json["type"] = "Transform";
                json_message.components.push_back(component_json);
            }
            if (gCoordinator.hasComponent<Color>(entity)) {
                nlohmann::json component_json = gCoordinator.getComponent<Color>(entity);
                component_json["type"] = "Color";
                json_message.components.push_back(component_json);
            }
            if (gCoordinator.hasComponent<Collision>(entity)) {
                nlohmann::json component_json = gCoordinator.getComponent<Collision>(entity);
                component_json["type"] = "Collision";
                json_message.components.push_back(component_json);
            }
            if (gCoordinator.hasComponent<CKinematic>(entity)) {
                nlohmann::json component_json = gCoordinator.getComponent<CKinematic>(entity);
                component_json["type"] = "CKinematic";
                json_message.components.push_back(component_json);
            }
            if (gCoordinator.hasComponent<Destroy>(entity)) {
                nlohmann::json component_json = gCoordinator.getComponent<Destroy>(entity);
                component_json["type"] = "Destroy";
                json_message.components.push_back(component_json);
            }
            if (gCoordinator.hasComponent<Brick>(entity)) {
                nlohmann::json component_json = gCoordinator.getComponent<Brick>(entity);
                component_json["type"] = "Brick";
                json_message.components.push_back(component_json);
            }
            json = json_message;
        } else if (type == UPDATE) {
            if (gCoordinator.hasComponent<Transform>(entity)) {
                json_message.components.emplace_back(gCoordinator.getComponent<Transform>(entity));
            }
            json = json_message;
        }
        return json.dump();
    }

    nlohmann::json parse_message(zmq::message_t &message) override {
        try {
            std::string jsonString(static_cast<char *>(message.data()), message.size());
            const nlohmann::json received_msg = nlohmann::json::parse(jsonString);
            return received_msg;
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
