#pragma once

#include <chrono>

class Timer
{
    using Clock = std::chrono::high_resolution_clock;
    using Timestamp = Clock::time_point;
    using Duration = std::chrono::duration<float>;

    Timestamp anchor = Clock::now();

public:
    void reset() {
        anchor = Clock::now();
    }

    float elapsed() const {
        Duration duration = std::chrono::duration_cast<Duration>(Clock::now() - anchor);
        return duration.count();
    }
};