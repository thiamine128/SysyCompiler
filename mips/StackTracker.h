//
// Created by slty5 on 24-11-7.
//

#ifndef STACKTRACKER_H
#define STACKTRACKER_H
#include <unordered_map>

#include "MIPS.h"

namespace thm {

class StackTracker {
public:
    std::unordered_map<int, int> offset;
    int fsize = 0;

    void reset();
    MIPSInst *alloc(int slot);
};

} // thm

#endif //STACKTRACKER_H
