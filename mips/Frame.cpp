//
// Created by slty5 on 24-11-10.
//

#include "Frame.h"

namespace thm {
    bool Frame::isInFrame(int slot) {
        return slotOffset.find(slot) != slotOffset.end();
    }

    int Frame::getCallArgOffset(int idx) {
        return idx * 4;
    }
} // thm