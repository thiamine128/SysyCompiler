//
// Created by slty5 on 24-11-10.
//

#ifndef FRAME_H
#define FRAME_H
#include <unordered_map>

namespace thm {

class Frame {
public:
    int raOffset;
    int frameSize;
    int argsNum;

    std::unordered_map<int, int> slotOffset;

    bool isInFrame(int slot);
    int getCallArgOffset(int idx);
};

} // thm

#endif //FRAME_H
