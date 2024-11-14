//
// Created by slty5 on 24-11-14.
//

#ifndef GCM_H
#define GCM_H
#include "Pass.h"

namespace thm {

class GCM : public Pass {
public:
    std::unordered_map<Instruction *, bool> vis;
    GCM(Module *module) : Pass(module) {}

    void process() override;
    void processFunction(Function *function);
    void scheduleEarly(Function *func, Instruction *inst);
    void scheduleLate(Function *func, Instruction *inst);
    BasicBlock *findLCA(BasicBlock *bb1, BasicBlock *bb2);
};

} // thm

#endif //GCM_H
