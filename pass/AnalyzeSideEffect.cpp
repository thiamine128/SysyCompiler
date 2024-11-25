//
// Created by slty5 on 24-11-24.
//

#include "AnalyzeSideEffect.h"

#include <iostream>

namespace thm {

    bool AnalyzeSideEffect::analyzeFunction(Function *func) {
        bool changed = false;
        for (auto bb : func->blocks) {
            for (auto inst : bb->insts) {
                if (CallInst *call = dynamic_cast<CallInst *>(inst)) {
                    for (int i = 0; i < call->args.size(); i++) {
                        auto callee = call->args[i];
                        Value *unpack = callee;
                        while (GetElementPtr *gep = dynamic_cast<GetElementPtr*>(unpack)) {
                            unpack = gep->ptr;
                        }
                        if (Argument *arg = dynamic_cast<Argument *>(unpack)) {
                            if (call->function->argSideEffects.find(call->function->args[i]) != call->function->argSideEffects.end()) {
                                auto [it, inserted] = func->argSideEffects.insert(arg);
                                changed |= inserted;
                            }
                        }
                        if (GlobalVariable *global = dynamic_cast<GlobalVariable*>(unpack)) {
                            if (call->function != func && call->function->argSideEffects.find(call->function->args[i]) != call->function->argSideEffects.end()) {
                                auto [it, inserted] = func->globalVarSideEffects.insert(global);
                                changed |= inserted;
                            }
                        }
                        for (auto globalVar : call->function->globalVarSideEffects) {
                            func->globalVarSideEffects.insert(globalVar);
                        }
                        func->ioSideEffects |= call->function->ioSideEffects;
                    }

                }
                if (StoreInst *store = dynamic_cast<StoreInst *>(inst)) {
                    for (auto arg : func->args) {
                        if (isPtrAlias(arg, store->ptr)) {
                            auto [it, inserted] = func->argSideEffects.insert(arg);
                            changed |= inserted;
                        }
                    }
                    Value *unpack = store->ptr;
                    while (GetElementPtr *gep = dynamic_cast<GetElementPtr*>(unpack)) {
                        unpack = gep->ptr;
                    }
                    if (GlobalVariable *global = dynamic_cast<GlobalVariable*>(unpack)) {
                        auto [it, inserted] = func->globalVarSideEffects.insert(global);
                        changed |= inserted;
                    }
                }
            }
        }
        return changed;
    }

    void AnalyzeSideEffect::process() {
        clean();
        bool changed = true;
        while (changed) {
            changed = false;
            for (auto func : module->functions) {
                changed |= analyzeFunction(func);
            }
            changed |= analyzeFunction(module->main);
        }
    }

    void AnalyzeSideEffect::clean() {
        for (auto func : module->functions) {
            func->argSideEffects.clear();
            func->globalVarSideEffects.clear();
            func->ioSideEffects = false;
        }
        module->main->argSideEffects.clear();
        module->main->globalVarSideEffects.clear();
        module->main->ioSideEffects = false;
        module->getInt->ioSideEffects = true;
        module->getChar->ioSideEffects = true;
        module->putInt->ioSideEffects = true;
        module->putChar->ioSideEffects = true;
        module->putStr->ioSideEffects = true;
    }
} // thm