//
// Created by Jayesh Gajbhar on 9/22/24.
//

#ifndef SERVER_HPP
#define SERVER_HPP

#include <zmq.hpp>
#include <thread>
#include <atomic>
#include <vector>
#include <mutex>
#include "constants.hpp"
#include "../../main.hpp"
#include "../animation/controller/moveBetween2Points.hpp"
#include "../core/timeline.hpp"
#include "../objects/shapes/rectangle.hpp"
#include "../core/physics/keyMovement.hpp"
#include "../objects/factory.hpp"
#include "../generic/safe_queue.hpp"


std::atomic<bool> running{true};
std::vector<std::string> connectedPeers;
std::mutex peersMutex;

Timeline globalServerTimeline(nullptr,1000.f);
SafeQueue<std::array<float, 3>> messageQueue;


void handle_platform_movement(zmq::socket_t& pub_socket) {
    Timeline platformTimeline(&globalServerTimeline, 1);
    auto platform = Factory::createRectangle({255, 0, 0, 255}, {300, SCREEN_HEIGHT / 2.f, SCREEN_WIDTH / 2.f, 100},
                                             true, 100000.f, 0.8);
    MoveBetween2Points m(100.f, 400.f, LEFT, 2, platformTimeline);

    while (running) {
        m.moveBetween2Points(*platform);
        platform->update(FRAME_RATE);

        // Broadcast platform position
        std::array<float, 3> message = {MessageType::PLATFORM, platform->rect.x, platform->rect.y};
        pub_socket.send(zmq::buffer(message), zmq::send_flags::none);

        std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(FRAME_RATE)));
    }
}

void handle_peer_discovery(zmq::socket_t& rep_socket) {
    while (running) {
        zmq::message_t request;
        rep_socket.recv(request, zmq::recv_flags::none);
        std::string peer_address = request.to_string();

        {
            std::lock_guard<std::mutex> lock(peersMutex);
            if (std::find(connectedPeers.begin(), connectedPeers.end(), peer_address) == connectedPeers.end()) {
                connectedPeers.push_back(peer_address);
            }
        }

        // Send the list of connected peers back to the new peer
        std::string peer_list;
        {
            std::lock_guard<std::mutex> lock(peersMutex);
            for (const auto& peer : connectedPeers) {
                peer_list += peer + ";";
            }
        }
        rep_socket.send(zmq::buffer(peer_list), zmq::send_flags::none);
    }
}

void platform_movement(std::unique_ptr<Rectangle> &platform) {
    Timeline platformTimeline(&globalServerTimeline, 1);
    int64_t lastTime = platformTimeline.getElapsedTime();
    MoveBetween2Points m(100.f, 400.f, LEFT, 2, platformTimeline);
    // std::cout<<"Game running: "<<gameRunning<<std::endl;
    while (running) {
        int64_t currentTime = platformTimeline.getElapsedTime();
        float dT = (currentTime - lastTime) / 1000.f;
        lastTime = currentTime;

        if(dT > FRAME_RATE) {
            dT = FRAME_RATE;
        }

        m.moveBetween2Points(*platform);
        platform->update(dT);
        messageQueue.enqueue({MessageType::PLATFORM, platform->rect.x, platform->rect.y});

        if (dT < FRAME_RATE) {
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(FRAME_RATE - dT)));
        }
    }

    std::cout << "Kill platform thread" << std::endl;
}


#endif //SERVER_HPP
