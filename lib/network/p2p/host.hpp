//
// Created by Utsav Lal on 9/24/24.
//

#ifndef HOST_HPP
#define HOST_HPP
#include <array>
#include <zmq.hpp>
#include <thread>

#include "../../core/timeline.hpp"
#include "../../enum/message_type.hpp"
#include "../../generic/safe_queue.hpp"
#include "../../objects/shapes/rectangle.hpp"

/**
 * This is the general Host class which enables P2P communication between the clients
 * Right now it can only support 3 clients
 * Essentially you have to first create a host object and then call the start method
 */
class Host {
public:
    void process_message(std::unique_ptr<Rectangle> &c2, std::unique_ptr<Rectangle> &c3,
                         std::unique_ptr<Rectangle> &c1, std::unique_ptr<Rectangle> &platform);

    void receive_message();

    void send_message();

    explicit Host(const std::string &server_socket, const std::string &p2_socket,
                  const std::string &p3_socket, zmq::context_t &context, const std::string &publisher_socket);

    void start(std::unique_ptr<Rectangle> &c2, std::unique_ptr<Rectangle> &c3, std::unique_ptr<Rectangle> &c1, std::unique_ptr<
               Rectangle> &platform, Timeline &anchorTimeline);

    void move_platform(std::unique_ptr<Rectangle> &platform, Timeline &anchorTimeline);

    SafeQueue<std::array<float, 3> > send_queue;

private:
    zmq::socket_t server_listener;
    zmq::socket_t p2_receiver;
    zmq::socket_t p3_receiver;
    zmq::socket_t publisher;
    SafeQueue<std::array<float, 3> > receive_queue;
};


#endif //HOST_HPP
