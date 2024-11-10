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
    std::map<int, int> slotSize;
    int frameSize = 0;
    int callArgsNum = 0;
    std::vector<AllocaInst *> allocArgs;

    int allocSlot();
    void useStack(int slot);
    void useArray(int slot, int size);
    void prepareCallArgs(int num);
    Frame *establishFrame();
};

} // thm

#endif //SLOTTRACKER_H
