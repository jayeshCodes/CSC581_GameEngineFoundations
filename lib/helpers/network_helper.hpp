//
// Created by Utsav Lal on 10/30/24.
//

#pragma once

#include <zmq.hpp>

#include "../ECS/types.hpp"


namespace NetworkHelper {
    const std::string EVENT_ENTITY_ID = "ThisIsAnEvent";

    inline void sendMessageClient(zmq::socket_t &socket, const std::string &entity_id,
                                  const std::variant<std::vector<float>, std::string> &message
    ) {
        socket.send(zmq::buffer(entity_id), zmq::send_flags::sndmore);
        if (std::holds_alternative<std::string>(message)) {
            const auto str = std::get<std::string>(message);
            socket.send(zmq::buffer(str), zmq::send_flags::none);
        } else {
            auto vec = std::get<std::vector<float> >(message);
            socket.send(zmq::buffer(vec), zmq::send_flags::none);
        }
    }

    inline void receiveMessageClient(zmq::socket_t &socket, zmq::message_t &message, std::string &entity_id
    ) {
        zmq::message_t eID;
        zmq::message_t data;

        auto rcv_res_1 = socket.recv(eID, zmq::recv_flags::none);
        auto rcv_res_2 = socket.recv(data, zmq::recv_flags::none);

        entity_id = eID.to_string();
        message.copy(data);
    }

    inline void sendMessageServer(zmq::socket_t &socket, const std::string& client_id, const std::string &entity_id,
                                  const std::variant<std::vector<char>, std::vector<float>> &message) {
        socket.send(zmq::buffer(client_id + "R"), zmq::send_flags::sndmore);
        socket.send(zmq::buffer(entity_id), zmq::send_flags::sndmore);
        if (std::holds_alternative<std::vector<float> >(message)) {
            auto str = std::get<std::vector<float> >(message);
            socket.send(zmq::buffer(str), zmq::send_flags::none);
        } else {
            auto vec = std::get<std::vector<char> >(message);
            socket.send(zmq::buffer(vec), zmq::send_flags::none);
        }
    }

    inline void sendMessageServer(zmq::socket_t &socket, const std::string& client_id, const std::string &entity_id,
                                  const std::variant<std::vector<float>, std::string> &message) {
        socket.send(zmq::buffer(client_id + "R"), zmq::send_flags::sndmore);
        socket.send(zmq::buffer(entity_id), zmq::send_flags::sndmore);
        if (std::holds_alternative<std::string>(message)) {
            const auto str = std::get<std::string>(message);
            socket.send(zmq::buffer(str), zmq::send_flags::none);
        } else {
            auto vec = std::get<std::vector<float> >(message);
            socket.send(zmq::buffer(vec), zmq::send_flags::none);
        }
    }

    inline void receiveMessageServer(zmq::socket_t &socket, zmq::message_t &identify, zmq::message_t &entity_id,
                                       zmq::message_t &entity_data) {
        auto _ = socket.recv(identify, zmq::recv_flags::none);
        auto _1 =socket.recv(entity_id, zmq::recv_flags::none);
        auto _2 = socket.recv(entity_data, zmq::recv_flags::none);

    }

    inline void sendEventClient(zmq::socket_t &socket, const std::shared_ptr<Event> &event) {
        nlohmann::json eventJson;
        to_json(eventJson, *event);
        const std::string entity_id = EVENT_ENTITY_ID;
        sendMessageClient(socket, entity_id, eventJson.dump());
    }

    inline void receiveEventClient(zmq::socket_t &socket, const std::shared_ptr<Event> &event) {
        std::string entity_id;
        zmq::message_t message;
        receiveMessageClient(socket, message, entity_id);
        const nlohmann::json eventJson = nlohmann::json::parse(message.to_string());
        from_json(eventJson, *event);
    }
} // namespace NetworkHelper
