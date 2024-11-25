//
// Created by slty5 on 24-11-25.
//

#ifndef ANALYZEMEMDEP_H
#define ANALYZEMEMDEP_H
#include "Pass.h"
#include "../llvm/LLVM.h"

namespace thm {

class AnalyzeMemDep : public Pass {
public:
    struct LoadInfo {
        int id;
        std::unordered_set<LoadInst *> loads;
        std::unordered_set<Instruction *> stores;
    };
    AnalyzeMemDep(Module *module) : Pass(module) {}

    void process() override;
    void analyzeFunction(Function *func);
};

} // thm

#endif //ANALYZEMEMDEP_H
