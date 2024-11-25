//
// Created by slty5 on 24-11-24.
//

#include "PassManager.h"

#include "Pass.h"

namespace thm {
    void PassManager::init() {

    }

    void PassManager::process() {
        for (auto pass : passes) {
            pass->process();
        }
    }

    PassManager * PassManager::appendPass(Pass *pass) {
        passes.push_back(pass);
        return this;
    }
} // thm