//
// Created by slty5 on 24-11-24.
//

#ifndef SIDEEFFECTANALYZER_H
#define SIDEEFFECTANALYZER_H
#include <unordered_map>

#include "Pass.h"
#include "../llvm/LLVM.h"

namespace thm {

class AnalyzeSideEffect : public Pass {
public:
    AnalyzeSideEffect(Module *module) : Pass(module) {}
    bool analyzeFunction(Function *func);
    void process() override;
    void clean();
};

} // thm

#endif //SIDEEFFECTANALYZER_H
