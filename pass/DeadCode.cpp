//
// Created by slty5 on 24-11-15.
//

#include "DeadCode.h"

namespace thm {
    void DeadCode::process() {
        processFunction(module->main);
        // First stage:
        // printf are marked as used
        // some alloc and global variables are marked as used
        // Second stage:
        // store / functions modifying those variables are used too

        for (auto fun : module->functions) {
            if (fun != module->main) {

            }
        }
    }

    void DeadCode::processFunction(Function *function) {
        for (auto bb : function->blocks) {
            for (auto inst : bb->insts) {
                if (CallInst *call = dynamic_cast<CallInst *>(inst)) {
                    markUsed(call);
                }
                if (RetInst *ret = dynamic_cast<RetInst *>(inst)) {
                    markUsed(ret);
                }
                if (LoadInst *load = dynamic_cast<LoadInst *>(inst)) {
                    markUsed(load);
                }
            }
        }
    }

    void DeadCode::markUsed(Value *value) {
        if (!value->used) {
            value->used = true;
            if (User *user = dynamic_cast<User *>(value)) {
                for (auto use : user->usings) {
                    markUsed(*use);
                }
            }
            if (Function *function = dynamic_cast<Function *>(value)) {
                processFunction(function);
            }
        }
    }
} // thm