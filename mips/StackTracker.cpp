//
// Created by slty5 on 24-11-7.
//

#include "StackTracker.h"

namespace thm {
    void StackTracker::reset() {
        offset.clear();
        fsize = 0;
    }

    MIPSInst * StackTracker::alloc(int slot) {
        offset[slot] = fsize;
        fsize += 4;
        return MIPSInst::AddImm(Register::SP, Register::SP, -4);
    }
} // thm