//
// Created by Utsav Lal on 9/25/24.
//

#ifndef CLIENT_HPP
#define CLIENT_HPP
#include <string>
#include <zmq.hpp>
#include <thread>

#include "../../generic/safe_queue.hpp"
#include "../../objects/shapes/rectangle.hpp"


class Client {
public:
    Client();

    void Initialize(const int &PUB_PORT, const int &SUB_PORT, zmq::context_t &context);

    void Start(std::unique_ptr<Rectangle> &char_1, std::unique_ptr<Rectangle> &char_2,
               std::unique_ptr<Rectangle> &char_3, std::unique_ptr<Rectangle> &platform);

    SafeQueue<std::array<float, 3> > messageQueue;

private:
    void send_position();

    void get_messages();

    void process_messages(std::unique_ptr<Rectangle> &char_1, std::unique_ptr<Rectangle> &char_2,
                          std::unique_ptr<Rectangle> &char_3, std::unique_ptr<Rectangle> &platform);

    SafeQueue<std::array<float, 3> > receiveQueue;

    zmq::socket_t pub;
    zmq::socket_t sub;
};


#endif //CLIENT_HPP
