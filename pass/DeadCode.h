//
// Created by slty5 on 24-11-15.
//

#ifndef DEADCODE_H
#define DEADCODE_H
#include "Pass.h"

namespace thm {

class DeadCode : public Pass {
public:
    std::unordered_set<Value *> q;

    DeadCode(Module *module) : Pass(module) {};

    void process() override;
    void processFunction(Function *function);
    void markUsed(Value *value);
};

} // thm

#endif //DEADCODE_H
