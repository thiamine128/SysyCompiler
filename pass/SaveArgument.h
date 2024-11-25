//
// Created by slty5 on 24-11-21.
//

#ifndef SAVEARGUMENT_H
#define SAVEARGUMENT_H
#include "../llvm/LLVM.h"
#include "Pass.h"

namespace thm {

class SaveArgument : public Pass {
public:
    SaveArgument(Module *module) : Pass(module) {}

    void process() override;
    void processFunction(Function *func);
};

} // thm

#endif //SAVEARGUMENT_H
