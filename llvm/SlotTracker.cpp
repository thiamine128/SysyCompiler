//
// Created by slty5 on 24-10-29.
//

#include "SlotTracker.h"
#include "../mips/Frame.h"
#include "LLVM.h"

namespace thm {
    int SlotTracker::allocSlot() {
        return currentSlot++;
    }
} // thm