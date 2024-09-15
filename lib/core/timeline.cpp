#include "timeline.hpp"

#include <mutex>
#include <cstdint>


/**
 * Constructor for Timeline
 * @param anchor_timeline - Pointer to another timeline (e.g., system time or another timeline)
 * @param tic_interval - Units of anchor timeline per step
 */
Timeline::Timeline(Timeline *anchor_timeline, double tic_interval)
    : anchor(anchor_timeline), tic(tic_interval), paused(false), elapsed_paused_time(0) {
    start_time = getCurrentTime();
}

// Destructor
Timeline::~Timeline() = default;

// Starts or resumes the timeline
void Timeline::start() {
    std::lock_guard<std::mutex> lock(m);
    if (paused) {
        paused = false;
        elapsed_paused_time += (getCurrentTime() - last_paused_time);
    }
}

// Pauses the timeline
void Timeline::pause() {
    std::lock_guard<std::mutex> lock(m);
    if (!paused) {
        paused = true;
        last_paused_time = getCurrentTime();
    }
}

// Resets the timeline
void Timeline::reset() {
    std::lock_guard<std::mutex> lock(m);
    start_time = getCurrentTime();
    elapsed_paused_time = 0;
    paused = false;
}

// Get the time since the timeline started, minus the paused time
int64_t Timeline::getElapsedTime() {
    std::lock_guard<std::mutex> lock(m);
    if (paused) {
        return (last_paused_time - start_time - elapsed_paused_time) / tic;
    } else {
        return (getCurrentTime() - start_time - elapsed_paused_time) / tic;
    }
}


/**
 * Get current system time (or anchor time if anchored)
 * @return time in microseconds if no anchor or anchored time
 */
int64_t Timeline::getCurrentTime() {
    if (anchor) {
        return anchor->getElapsedTime();
    } else {
        return std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }
}

bool Timeline::isPaused() {
    return paused;
}
