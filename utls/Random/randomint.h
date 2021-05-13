//
// Created by zhao on 2021/5/12.
//

#ifndef RPCTEST_RANDOMINT_H
#define RPCTEST_RANDOMINT_H
#include <random>
class randomInt{
public:
    static int get(){
        return u_(e_);
    }
    static int get(int start, int end){
        std::uniform_int_distribution<int> u(start, end);
        return u(e_);
    }
private:
    static std::default_random_engine e_;
    static std::uniform_int_distribution<int> u_;
};

std::default_random_engine  randomInt::e_{};
// std::uniform_int_distribution<int> randomInt::u_{0, 1000};
#endif //RPCTEST_RANDOMINT_H
