//
// Created by Utsav Lal on 9/15/24.
//

#ifndef PUBLISHER_HPP
#define PUBLISHER_HPP

#include <zmq.hpp>

#include "../generic/singleton.hpp"



class Publisher : public Singleton<Publisher> {
    zmq::socket_t socket;
    bool is_connected = false;

    explicit Publisher(zmq::context_t context);

public:
    friend class Singleton;
    void connect(const std::string &address);
    void publish(const std::string &message);
};



#endif //PUBLISHER_HPP
