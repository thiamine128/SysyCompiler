//
// Created by slty5 on 24-11-10.
//

#ifndef FRAME_H
#define FRAME_H
#include <unordered_map>
#include <vector>

#include "../llvm/LLVM.h"

namespace thm {
class Frame {
public:
    Function *function;
    int raOffset = 0;
    int frameSize = 0;
    int argsNum = 0;

    std::unordered_map<Register, int> saved;
    std::unordered_map<AllocaInst *, int> offset;
    std::unordered_map<ArgumentAddress *, int> args;

    Frame(Function *function);
    void init(int maxCallArgs);
    int getOffset(AllocaInst *slot);
    int getRegOffset(Register reg);
    int getCallArgOffset(int i);
    int getArgAddress(ArgumentAddress *addr);
};

} // thm

#endif //FRAME_H
