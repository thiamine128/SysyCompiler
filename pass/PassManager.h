//
// Created by slty5 on 24-11-24.
//

#ifndef PASSMANAGER_H
#define PASSMANAGER_H
#include <vector>

#include "../llvm/LLVM.h"

namespace thm {
    class Pass;

class PassManager {
public:
    Module *module;
    std::vector<Pass*> passes;

    void init();
    void process();
    PassManager* appendPass(Pass *pass);
};

} // thm

#endif //PASSMANAGER_H
