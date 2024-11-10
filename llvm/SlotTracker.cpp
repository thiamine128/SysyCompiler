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

    void SlotTracker::useStack(int slot) {
        frameSize += 4;
        slotSize[slot] = 4;
    }

    void SlotTracker::useArray(int slot, int size) {
        size = (size + 3) / 4 * 4;
        frameSize += size;
        slotSize[slot] = size;
    }

    void SlotTracker::prepareCallArgs(int num) {
        callArgsNum = num;
        frameSize += 4 * num;
    }

    Frame *SlotTracker::establishFrame() {
        frameSize += 4;
        Frame *frame = new Frame();
        frame->frameSize = frameSize;
        frame->raOffset = frameSize - 4;
        frame->argsNum = allocArgs.size();
        int curOffset = frame->raOffset;
        for (auto iter = slotSize.rbegin(); iter != slotSize.rend(); iter++) {
            curOffset -= iter->second;
            frame->slotOffset[iter->first] = curOffset;
        }
        for (int i = 0; i < allocArgs.size(); i++) {
            frame->slotOffset[allocArgs[i]->slot] = frameSize + 4 * i;
        }
        return frame;
    }
} // thm