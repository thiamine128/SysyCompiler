//
// Created by slty5 on 24-11-25.
//

#ifndef ANALYZEDOMINANCE_H
#define ANALYZEDOMINANCE_H
#include "Pass.h"

namespace thm {

class AnalyzeDominance : public Pass {
public:
    AnalyzeDominance(Module *module) : Pass(module) {}

    void process() override;
    void analyzeFunction(Function *func);
};

} // thm

#endif //ANALYZEDOMINANCE_H
