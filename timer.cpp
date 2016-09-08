//
// Created by Bryn Keller on 5/19/16.
//

#include "timer.h"

Timer::Timer()
{
    start_time = std::chrono::system_clock::now();
}

long Timer::elapsed()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start_time).count();
}

void Timer::restart() { start_time = std::chrono::system_clock::now(); }

std::chrono::system_clock::time_point Timer::started() { return start_time; }
