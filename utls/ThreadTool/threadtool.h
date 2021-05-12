//
// Created by zhao on 2021/5/12.
//

#ifndef RPCTEST_THREADTOOL_H
#define RPCTEST_THREADTOOL_H

#include <thread>
#include <strstream>
unsigned long getTid()
{
    std::thread::id tid = std::this_thread::get_id();
    std::ostrstream os;
    os << tid;
    unsigned long tidx = std::stol(os.str());
    return tidx;
}

#endif //RPCTEST_THREADTOOL_H
