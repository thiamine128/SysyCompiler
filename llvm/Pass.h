//
// Created by slty5 on 24-11-13.
//

#ifndef PASS_H
#define PASS_H
#include "LLVM.h"

namespace thm {

class Pass {
public:
    Module *module;

    Pass(Module *module) : module(module) {};

    virtual ~Pass() = default;

    virtual void process();
};

} // thm

#endif //PASS_H
