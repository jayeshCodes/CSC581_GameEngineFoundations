//
// Created by Jayesh Gajbhar on 9/10/24.
//

#ifndef TIMELINE_H
#define TIMELINE_H

#include <mutex>
#include <cstdint>
#include <chrono>

class Timeline {
private:
    std::mutex m; // Mutex for thread safety
    int64_t start_time; // Time when the anchor was created
    int64_t elapsed_paused_time; // Total time spent paused
    int64_t last_paused_time; // Time when the timeline was last paused
    double tic; // Units of anchor timeline per step
    bool paused; // Is the timeline currently paused?
    Timeline* anchor; // Pointer to another timeline (e.g., system time or another timeline)

public:
    // Constructor
    explicit Timeline(Timeline* anchor_timeline = nullptr, double tic_interval = 1);

    // Destructor
    ~Timeline();

    // Starts or resumes the timeline
    void start();

    // Pauses the timeline
    void pause();

    // Resets the timeline
    void reset();

    // Get the time since the timeline started, minus the paused time
    int64_t getElapsedTime();

    bool isPaused();

    void changeTic(double new_tic);

    // Get current system time (or anchor time if anchored)
    int64_t getCurrentTime(); // changed visibility to public
};

#endif // TIMELINE_H
