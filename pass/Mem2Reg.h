//
// Created by slty5 on 24-11-13.
//

#ifndef MEM2REG_H
#define MEM2REG_H
#include "Pass.h"

namespace thm {

class Mem2Reg : public Pass {
public:
    Mem2Reg(Module *module) : Pass(module) {};
    void process() override;
    void processFunction(Function *function);
    void promote(Function *function, AllocaInst *alloca);
    void replaceUse(Instruction *inst, Value *src, Value *dst);
    void collapsePhi(Function *function);
};

} // thm

#endif //MEM2REG_H
