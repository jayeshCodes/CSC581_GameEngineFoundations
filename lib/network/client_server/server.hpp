//
// Created by Utsav Lal on 9/25/24.
//

#ifndef SERVER_HPP
#define SERVER_HPP
#include <zmq.hpp>

#include "../../core/timeline.hpp"
#include "../../generic/safe_queue.hpp"


class Server {

public:
    Server() = default;

    void Initialize(zmq::context_t &context);

    void Start(Timeline &globalServerTimeline);

private:
    void pull_messages();

    void process_messages();

    void send_messages();

    void platform_movement(Timeline &timeline);

    zmq::socket_t sub_1;
    zmq::socket_t sub_2;
    zmq::socket_t sub_3;

    zmq::socket_t pub_1;
    zmq::socket_t pub_2;
    zmq::socket_t pub_3;

    SafeQueue<std::array<float, 3> > messageQueue;
    SafeQueue<std::array<float, 3> > pub1_queue;
    SafeQueue<std::array<float, 3> > pub2_queue;
    SafeQueue<std::array<float, 3> > pub3_queue;

};



#endif //SERVER_HPP
