//
// Created by Jayesh Gajbhar on 9/10/24.
//

#ifndef TIMELINE_HPP
#define TIMELINE_HPP
#include <cstdint>
#include <mutex>
#include <chrono>

// Taken from slides presented in class
class Timeline {
public:
    // constructor
    Timeline(Timeline *anchor, int64_t tic);

    // for local time
    Timeline(): start_time(0), elapsed_paused_time(0), paused(false) {
    };

    int64_t currentSystemTime();

    void start();

    int64_t getTime();

    void pause();

    void unpause();

    void changeTic(int tic);

    bool isPaused();

private:
    std::mutex m; //if tics can change size and the game is multithreaded
    int64_t start_time; //the time of the *anchor when created
    int64_t elapsed_paused_time;
    int64_t last_paused_time;
    int64_t tic; //units of anchor timeline per step
    bool paused;
    Timeline *anchor; //for most general game time, system library pointer
};


#endif //TIMELINE_HPP
