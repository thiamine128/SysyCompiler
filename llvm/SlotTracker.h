//
// Created by slty5 on 24-10-29.
//

#ifndef SLOTTRACKER_H
#define SLOTTRACKER_H
#include <map>
#include <unordered_map>
#include <vector>


namespace thm {
    class Frame;
    class AllocaInst;

class SlotTracker {
public:
    int currentSlot = 0;

    int allocSlot();
};

} // thm

#endif //SLOTTRACKER_H
