//
// Created by slty5 on 24-11-17.
//

#ifndef ALLOCATEREGISTERS_H
#define ALLOCATEREGISTERS_H
#include "Pass.h"

namespace thm {

class AllocateRegisters : public Pass {
public:

    AllocateRegisters(Module *module) : Pass(module) {}

    void process() override;
    void processFunction(Function *function);
};

} // thm

#endif //ALLOCATEREGISTERS_H
