//
// Created by slty5 on 24-10-29.
//

#include "SlotTracker.h"

namespace thm {
    int SlotTracker::allocSlot() {
        return currentSlot++;
    }
} // thm