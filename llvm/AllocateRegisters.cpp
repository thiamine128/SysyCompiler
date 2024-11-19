//
// Created by slty5 on 24-11-17.
//

#include "AllocateRegisters.h"
#include <complex>
#include <iostream>

#include "RegAllocator.h"

namespace thm {
    void AllocateRegisters::process() {
        for (auto func : module->functions) {
            processFunction(func);
        }
        processFunction(module->main);
    }

    void AllocateRegisters::processFunction(Function *function) {
        RegAllocator regAllocator(function);
        regAllocator.init();
        regAllocator.process();
        regAllocator.submitColors();
    }
} // thm