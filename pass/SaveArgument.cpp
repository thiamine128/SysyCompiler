//
// Created by slty5 on 24-11-21.
//

#include "SaveArgument.h"

namespace thm {
    void SaveArgument::process() {
        for (auto func : module->functions) {
            processFunction(func);
        }
        processFunction(module->main);
    }

    void SaveArgument::processFunction(Function *func) {
        for (auto bb : func->blocks) {
            for (auto iter = bb->insts.begin(); iter != bb->insts.end(); iter++) {
                if (CallInst *callInst = dynamic_cast<CallInst *>(*iter)) {
                    // TODO: find call inst recursively to find to proper maxUsed
                    int maxUsed = 4;
                    if (callInst->function->name == "getint" || callInst->function->name == "getchar" || callInst->function->name == "putint" || callInst->function->name == "putch" || callInst->function->name == "putstr") {
                        maxUsed = 1;
                    }
                    std::vector<BackupArg *> backups;
                    for (int i = 0; i < func->args.size() && i < maxUsed; i++) {
                        auto backup = new BackupArg(i);
                        backups.push_back(backup);
                        backup->block = bb;
                        iter = bb->insts.insert(iter, backup) + 1;
                    }

                    for (int i = 0; i < func->args.size() && i < maxUsed; i++) {
                        auto recover = new RecoverArg(backups[i]);
                        recover->block = bb;
                        iter = bb->insts.insert(iter + 1, recover);
                    }
                }
            }
        }
    }
} // thm