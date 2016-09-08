//
// Created by Bryn Keller on 5/19/16.
//

#ifndef RIVET_CONSOLE_TIMER_H
#define RIVET_CONSOLE_TIMER_H


#include <chrono>

class Timer {
public:
    Timer();
    std::chrono::system_clock::time_point started();
    void restart();
    long elapsed();
private:
    std::chrono::system_clock::time_point start_time;
};


#endif //RIVET_CONSOLE_TIMER_H
