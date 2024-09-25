//
// Created by Utsav Lal on 9/24/24.
//

#include "host.hpp"

#include <iostream>

#include "../../animation/controller/moveBetween2Points.hpp"
#include "../../enum/message_type.hpp"
#include "../../game/GameManager.hpp"
#include "../../helpers/constants.hpp"

/**
 * Function to process messages received from the clients
 * @param c2
 * @param c3
 * @param c1
 * @param platform
 */
void Host::process_message(std::unique_ptr<Rectangle> &c2, std::unique_ptr<Rectangle> &c3,
                           std::unique_ptr<Rectangle> &c1, std::unique_ptr<Rectangle> &platform) {
    while (GameManager::getInstance()->gameRunning) {
        if (receive_queue.notEmpty()) {
            const std::array<float, 3> pos = receive_queue.dequeue();
            auto type = static_cast<P2PMessageType>(pos[0]);
            switch (type) {
                case P2PMessageType::P2P_CHARACTER_3:
                    c3->rect.x = pos[1];
                    c3->rect.y = pos[2];
                    break;
                case P2PMessageType::P2P_CHARACTER_2:
                    c2->rect.x = pos[1];
                    c2->rect.y = pos[2];
                    break;
                case P2PMessageType::P2P_CHARACTER_1:
                    c1->rect.x = pos[1];
                    c1->rect.y = pos[2];
                    break;
                case P2PMessageType::P2P_PLATFORM:
                    platform->rect.x = pos[1];
                    platform->rect.y = pos[2];
                    break;
                default: break;
            }
        }
    }
    std::cout << "Closing process thread" << std::endl;
}

/**
 * Function to receive messages from the clients
 */
void Host::receive_message() {
    while (GameManager::getInstance()->gameRunning) {
        std::array<float, 3> position{};
        if (server_listener.recv(zmq::buffer(position, sizeof(position)), zmq::recv_flags::dontwait)) {
            receive_queue.enqueue(position);
        }
        if (p2_receiver.recv(zmq::buffer(position, sizeof(position)), zmq::recv_flags::dontwait)) {
            receive_queue.enqueue(position);
        }
        if (p3_receiver.recv(zmq::buffer(position, sizeof(position)), zmq::recv_flags::dontwait)) {
            receive_queue.enqueue(position);
        }
    }
    std::cout << "Closing receive thread" << std::endl;
}

/**
 * Function to send messages to the clients
 */
void Host::send_message() {
    while (GameManager::getInstance()->gameRunning) {
        if (send_queue.notEmpty()) {
            std::array<float, 3> position = send_queue.dequeue();
            publisher.send(zmq::buffer(position, sizeof(position)), zmq::send_flags::dontwait);
        }
    }
    std::cout << "Closing send thread" << std::endl;
}

/**
 * This will initialize the host with the server, client 2, and client 3 sockets.
 * @param server_socket - Port to the server
 * @param p2_socket - Port to the other client
 * @param p3_socket - Port to the other client
 * @param context - ZeroMQ context
 * @param publisher_socket - Port to publish messages
 */
Host::Host(const std::string &server_socket, const std::string &p2_socket,
           const std::string &p3_socket, zmq::context_t &context,
           const std::string &publisher_socket) {
    try {
        this->server_listener = zmq::socket_t(context, ZMQ_SUB);
        this->p2_receiver = zmq::socket_t(context, ZMQ_SUB);
        this->p3_receiver = zmq::socket_t(context, ZMQ_SUB);
        this->publisher = zmq::socket_t(context, ZMQ_PUB);

        this->p2_receiver.connect("tcp://localhost:" + p2_socket);
        this->p3_receiver.connect("tcp://localhost:" + p3_socket);
        this->server_listener.connect("tcp://localhost:" + server_socket);
        this->publisher.bind("tcp://*:" + publisher_socket);

        this->server_listener.set(zmq::sockopt::subscribe, "");
        this->p2_receiver.set(zmq::sockopt::subscribe, "");
        this->p3_receiver.set(zmq::sockopt::subscribe, "");
    } catch (const zmq::error_t &e) {
        std::cerr << "ZeroMQ error: " << e.what() << std::endl;
        throw;
    }
}

/**
 * This function will start the host functionalities. It includes moving the platform if the server, sending and receiving messages between the clients
 * @param c2 Character 2
 * @param c3 Character 3
 * @param c1 Character 1
 * @param platform Platform
 * @param anchorTimeline Timeline to anchor to
 */
void Host::start(std::unique_ptr<Rectangle> &c2, std::unique_ptr<Rectangle> &c3,
                 std::unique_ptr<Rectangle> &c1, std::unique_ptr<Rectangle> &platform, Timeline &anchorTimeline) {
    std::thread sendThread(&Host::send_message, this);
    std::thread receiveThread(&Host::receive_message, this);
    std::thread processThread(&Host::process_message, this, std::ref(c2), std::ref(c3), std::ref(c1),
                              std::ref(platform));
    std::thread platformThread(&Host::move_platform, this, std::ref(platform), std::ref(anchorTimeline));

    sendThread.detach();
    receiveThread.detach();
    processThread.detach();
    platformThread.detach();
}

/**
 * This function moves the platform between two points. This will only run on the server
 * @param platform - platform to move
 * @param anchorTimeline - timeline to anchor the platform to
 */
void Host::move_platform(std::unique_ptr<Rectangle> &platform, Timeline &anchorTimeline) {
    if (!GameManager::getInstance()->isServer) return;
    Timeline platformTimeline(&anchorTimeline, 1);
    int64_t lastTime = platformTimeline.getElapsedTime();
    MoveBetween2Points m(100.f, 900.f, LEFT, 2, platformTimeline);
    while (GameManager::getInstance()->gameRunning) {
        int64_t currentTime = platformTimeline.getElapsedTime();
        float dT = (currentTime - lastTime) / 1000.f;
        lastTime = currentTime;

        if (dT > engine_constants::FRAME_RATE) {
            dT = engine_constants::FRAME_RATE;
        }

        m.moveBetween2Points(*platform);
        platform->update(dT);
        send_queue.enqueue({P2PMessageType::P2P_PLATFORM, platform->rect.x, platform->rect.y});

        if (dT < engine_constants::FRAME_RATE) {
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(engine_constants::FRAME_RATE - dT)));
        }
    }
    std::cout << "Closing platform thread" << std::endl;
}
