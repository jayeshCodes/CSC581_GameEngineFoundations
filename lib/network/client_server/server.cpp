//
// Created by Utsav Lal on 9/25/24.
//

#include "server.hpp"

#include <iostream>
#include <thread>

#include "../../animation/controller/moveBetween2Points.hpp"
#include "../../core/timeline.hpp"
#include "../../enum/message_type.hpp"
#include "../../game/GameManager.hpp"
#include "../../helpers/constants.hpp"
#include "../../helpers/network_constants.hpp"
#include "../../objects/factory.hpp"

void Server::Initialize(zmq::context_t &context) {
    sub_1 = zmq::socket_t(context, ZMQ_SUB);
    sub_2 = zmq::socket_t(context, ZMQ_SUB);
    sub_3 = zmq::socket_t(context, ZMQ_SUB);

    pub_1 = zmq::socket_t(context, ZMQ_PUB);
    pub_2 = zmq::socket_t(context, ZMQ_PUB);
    pub_3 = zmq::socket_t(context, ZMQ_PUB);

    sub_1.set(zmq::sockopt::subscribe, "");
    sub_2.set(zmq::sockopt::subscribe, "");
    sub_3.set(zmq::sockopt::subscribe, "");

    sub_1.bind("tcp://*:" + std::to_string(SUB_1));
    pub_1.bind("tcp://*:" + std::to_string(PUB_1));

    sub_2.bind("tcp://*:" + std::to_string(SUB_2));
    pub_2.bind("tcp://*:" + std::to_string(PUB_2));

    sub_3.bind("tcp://*:" + std::to_string(SUB_3));
    pub_3.bind("tcp://*:" + std::to_string(PUB_3));

    std::cout << "Server initialized and sockets bound" << std::endl;
}

void Server::Start(Timeline &globalServerTimeline) {
    std::thread pull_thread(&Server::pull_messages, this);
    std::thread process_thread(&Server::process_messages, this);
    std::thread send_thread(&Server::send_messages, this);
    std::thread platform_thread(&Server::platform_movement, this, std::ref(globalServerTimeline));

    pull_thread.detach();
    process_thread.detach();
    send_thread.detach();
    platform_thread.detach();
}

void Server::pull_messages() {
    while (GameManager::getInstance()->gameRunning) {
        std::array<float, 3> position{};
        if (sub_1.recv(zmq::buffer(position, sizeof(position)), zmq::recv_flags::dontwait)) {
            messageQueue.enqueue(position);
        }
        if (sub_2.recv(zmq::buffer(position, sizeof(position)), zmq::recv_flags::dontwait)) {
            messageQueue.enqueue(position);
        }
        if (sub_3.recv(zmq::buffer(position, sizeof(position)), zmq::recv_flags::dontwait)) {
            messageQueue.enqueue(position);
        }
    }
    std::cout << "Kill pull messages thread" << std::endl;
}

void Server::send_messages() {
    while (GameManager::getInstance()->gameRunning) {
        if (pub1_queue.notEmpty()) {
            std::array<float, 3> latestPos = pub1_queue.dequeue();
            pub_1.send(zmq::buffer(latestPos, sizeof(latestPos)), zmq::send_flags::none);
        }
        if (pub2_queue.notEmpty()) {
            std::array<float, 3> latestPos = pub2_queue.dequeue();
            pub_2.send(zmq::buffer(latestPos, sizeof(latestPos)), zmq::send_flags::none);
        }
        if (pub3_queue.notEmpty()) {
            std::array<float, 3> latestPos = pub3_queue.dequeue();
            pub_3.send(zmq::buffer(latestPos, sizeof(latestPos)), zmq::send_flags::none);
        }
    }
    std::cout << "Kill send messages thread" << std::endl;
}

void Server::process_messages() {
    while (GameManager::getInstance()->gameRunning) {
        std::array<float, 3> position{};
        if (messageQueue.notEmpty()) {
            position = messageQueue.dequeue();
            switch (static_cast<MessageType>(position[0])) {
                case MessageType::CHAR_1:
                    pub2_queue.enqueue(position);
                    pub3_queue.enqueue(position);
                    break;
                case MessageType::CHAR_2:
                    pub1_queue.enqueue(position);
                    pub3_queue.enqueue(position);
                    break;
                case MessageType::CHAR_3:
                    pub1_queue.enqueue(position);
                    pub2_queue.enqueue(position);
                    break;
                case MessageType::PLATFORM:
                    pub1_queue.enqueue(position);
                    pub2_queue.enqueue(position);
                    pub3_queue.enqueue(position);
                    break;
            }
        }
    }
    std::cout << "Killed process messages thread" << std::endl;
}

void Server::platform_movement(Timeline &globalServerTimeline) {
    Timeline platformTimeline(&globalServerTimeline, 1);
    int64_t lastTime = platformTimeline.getElapsedTime();
    MoveBetween2Points m(100.f, 400.f, TO, 2, platformTimeline);
    auto platform = Factory::createRectangle({255, 0, 0, 255}, {300, SCREEN_HEIGHT * 3 / 4.f, SCREEN_WIDTH / 2.f, 100},
                                             true, 100000.f, 0.8);
    while (GameManager::getInstance()->gameRunning) {
        int64_t currentTime = platformTimeline.getElapsedTime();
        float dT = (currentTime - lastTime) / 1000.f;
        lastTime = currentTime;

        if (dT > engine_constants::FRAME_RATE) {
            dT = engine_constants::FRAME_RATE;
        }

        m.moveBetween2Points(*platform);
        platform->update(dT);
        messageQueue.enqueue({MessageType::PLATFORM, platform->rect.x, platform->rect.y});

        if (dT < engine_constants::FRAME_RATE) {
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(engine_constants::FRAME_RATE - dT)));
        }
    }

    std::cout << "Kill platform thread" << std::endl;
}

