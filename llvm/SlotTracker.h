//
// Created by slty5 on 24-10-29.
//

#ifndef SLOTTRACKER_H
#define SLOTTRACKER_H

namespace thm {

class SlotTracker {
public:
    int currentSlot = 0;

    int allocSlot();
};

} // thm

#endif //SLOTTRACKER_H
