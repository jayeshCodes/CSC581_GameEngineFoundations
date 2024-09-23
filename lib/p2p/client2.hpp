//
// Created by Jayesh Gajbhar on 9/23/24.
//

#ifndef CLIENT2_HPP
#define CLIENT2_HPP

#include <zmq.hpp>
#include <thread>
#include <atomic>
#include <vector>
#include <mutex>
#include "constants.hpp"
#include "../../main.hpp"
#include "../objects/shapes/rectangle.hpp"
#include "../core/physics/keyMovement.hpp"
#include "../objects/factory.hpp"

std::vector<std::string> peerAddresses;
std::mutex peersMutex;

Timeline anchorTimeline(nullptr, 1000.f);

void receive_server_messages(zmq::socket_t& sub_socket, std::unique_ptr<Rectangle>& platform) {
    while (gameRunning) {
        std::array<float, 3> message;
        if (sub_socket.recv(zmq::buffer(message), zmq::recv_flags::dontwait)) {
            if (static_cast<MessageType>(message[0]) == MessageType::PLATFORM) {
                platform->rect.x = message[1];
                platform->rect.y = message[2];
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void send_player_position(zmq::socket_t& pub_socket, const std::unique_ptr<Rectangle>& player) {
    while (gameRunning) {
        std::array<float, 3> message = {MessageType::CHARACTER, player->rect.x, player->rect.y};
        pub_socket.send(zmq::buffer(message), zmq::send_flags::none);
        std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(FRAME_RATE)));
    }
}

void receive_peer_messages(zmq::socket_t& sub_socket, std::vector<std::unique_ptr<Rectangle>>& otherPlayers) {
    while (gameRunning) {
        std::array<float, 3> message;
        if (sub_socket.recv(zmq::buffer(message), zmq::recv_flags::dontwait)) {
            if (static_cast<MessageType>(message[0]) == MessageType::CHARACTER) {
                // Update or create a new player rectangle
                // This is a simplified version; you might want to implement a more robust player management system
                if (otherPlayers.empty()) {
                    otherPlayers.push_back(Factory::createRectangle({0, 255, 0, 255}, {message[1], message[2], 100, 100}, false, 1, 1));
                } else {
                    otherPlayers[0]->rect.x = message[1];
                    otherPlayers[0]->rect.y = message[2];
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void discover_peers(zmq::socket_t& req_socket, const std::string& own_address) {
    req_socket.send(zmq::buffer(own_address), zmq::send_flags::none);
    zmq::message_t reply;
    req_socket.recv(reply, zmq::recv_flags::none);

    std::string peer_list = reply.to_string();
    std::istringstream iss(peer_list);
    std::string peer;

    std::lock_guard<std::mutex> lock(peersMutex);
    while (std::getline(iss, peer, ';')) {
        if (!peer.empty() && peer != own_address) {
            peerAddresses.push_back(peer);
        }
    }
}


#endif //CLIENT2_HPP
