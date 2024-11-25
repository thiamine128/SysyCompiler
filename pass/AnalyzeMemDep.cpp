//
// Created by slty5 on 24-11-25.
//

#include "AnalyzeMemDep.h"

namespace thm {
    void AnalyzeMemDep::process() {
        for (auto func : module->functions) {
            analyzeFunction(func);
        }
        analyzeFunction(module->main);
    }

    void AnalyzeMemDep::analyzeFunction(Function *func) {
        std::unordered_map<Value *, LoadInfo> loadInfos;
        for (auto bb : func->blocks) {

        }
    }
} // thm