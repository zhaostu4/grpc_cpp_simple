//
// Created by zhao on 2021/5/12.
//

#ifndef RPCTEST_TIMER_H
#define RPCTEST_TIMER_H

#include <chrono>

class timer{
public:
    static std::time_t getNow(){
        return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    }
};

#endif //RPCTEST_TIMER_H
