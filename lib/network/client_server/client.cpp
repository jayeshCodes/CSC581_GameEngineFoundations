//
// Created by Utsav Lal on 9/25/24.
//

#include "client.hpp"

#include "../../enum/message_type.hpp"
#include "../../game/GameManager.hpp"


Client::Client() = default;

void Client::Initialize(const int &PUB_PORT, const int &SUB_PORT, zmq::context_t &context) {
    pub = zmq::socket_t(context, ZMQ_PUB);
    sub = zmq::socket_t(context, ZMQ_SUB);

    pub.connect("tcp://localhost:" + std::to_string(SUB_PORT));
    sub.connect("tcp://localhost:" + std::to_string(PUB_PORT));
    sub.set(zmq::sockopt::subscribe, "");
}

void Client::Start(std::unique_ptr<Rectangle> &char_1, std::unique_ptr<Rectangle> &char_2,
                   std::unique_ptr<Rectangle> &char_3, std::unique_ptr<Rectangle> &platform) {
    std::thread send_thread(&Client::send_position, this);
    std::thread receive_thread(&Client::get_messages, this);
    std::thread process_thread(&Client::process_messages, this, std::ref(char_1), std::ref(char_2), std::ref(char_3),
                               std::ref(platform));

    send_thread.detach();
    receive_thread.detach();
    process_thread.detach();
}

void Client::send_position() {
    while (GameManager::getInstance()->gameRunning) {
        if (messageQueue.notEmpty()) {
            std::array<float, 3> position = messageQueue.dequeue();
            pub.send(zmq::buffer(position, sizeof(position)), zmq::send_flags::none);
        }
    }
}

void Client::get_messages() {
    while (GameManager::getInstance()->gameRunning) {
        std::array<float, 3> position{};
        if (sub.recv(zmq::buffer(position, sizeof(position)), zmq::recv_flags::dontwait)) {
            receiveQueue.enqueue(position);
        }
    }
}

void Client::process_messages(std::unique_ptr<Rectangle> &char_1, std::unique_ptr<Rectangle> &char_2,
                              std::unique_ptr<Rectangle> &char_3, std::unique_ptr<Rectangle> &platform) {
    while (GameManager::getInstance()->gameRunning) {
        if (receiveQueue.notEmpty()) {
            std::array<float, 3> pos = receiveQueue.dequeue();
            auto messageType = static_cast<MessageType>(pos[0]);
            switch (messageType) {
                case MessageType::CHAR_1:
                    char_1->rect.x = pos[1];
                    char_1->rect.y = pos[2];
                    break;
                case MessageType::CHAR_2:
                    char_2->rect.x = pos[1];
                    char_2->rect.y = pos[2];
                    break;
                case MessageType::CHAR_3:
                    char_3->rect.x = pos[1];
                    char_3->rect.y = pos[2];
                    break;
                case MessageType::PLATFORM:
                    platform->rect.x = pos[1];
                    platform->rect.y = pos[2];
                    break;
                default: break;
            }
        }
    }
}
