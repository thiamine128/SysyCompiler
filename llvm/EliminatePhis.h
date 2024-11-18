//
// Created by slty5 on 24-11-16.
//

#ifndef ELIMINATEPHIS_H
#define ELIMINATEPHIS_H
#include "Pass.h"

namespace thm {

class EliminatePhis : public Pass {
public:
    EliminatePhis(Module *module) : Pass(module) {}

    void process() override;
    void processFunction(Function *func);
    void processBasicBlock(BasicBlock *bb);
    std::vector<MoveInst *> toSequence(std::vector<std::pair<Value *, Value *>> pc);
};

} // thm

#endif //ELIMINATEPHIS_H
