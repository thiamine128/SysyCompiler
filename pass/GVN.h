//
// Created by slty5 on 24-11-25.
//

#ifndef GVN_H
#define GVN_H
#include "Pass.h"

namespace thm {

class GVN : public Pass {
public:
    GVN(Module *module) : Pass(module) {}

    void process() override;
    void processFunction(Function *func);
    Value *tryFold(Value *val);
};

} // thm

#endif //GVN_H
