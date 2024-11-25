//
// Created by slty5 on 24-11-25.
//

#include "MemorySSA.h"

#include <iostream>

namespace thm {
    void MemorySSA::process() {
        for (auto func : module->functions) {
            analyzeFunction(func);
        }
        analyzeFunction(module->main);
    }

    void MemorySSA::analyzeFunction(Function *func) {
        std::unordered_map<Value *, LoadInfo> loadInfos;
        for (auto bb : func->blocks) {
            for (auto inst : bb->insts) {
                if (LoadInst *load = dynamic_cast<LoadInst *>(inst)) {
                    auto address = unpackPtr(load->ptr);
                    auto [it, inserted] = loadInfos.insert({address, LoadInfo(loadInfos.size())});
                    LoadInfo &info = it->second;
                    info.loads.insert(load);
                    if (!inserted) {
                        continue;
                    }
                    for (auto bb1 : func->blocks) {
                        for (auto inst1 : bb1->insts) {
                            if (StoreInst *store = dynamic_cast<StoreInst *>(inst1)) {
                                if (isPtrAlias(store->ptr, load->ptr)) {
                                    info.stores.insert(store);
                                }
                            }
                            if (CallInst *call = dynamic_cast<CallInst *>(inst1)) {
                                if (call->hasSideEffectOn(address)) {
                                    info.stores.insert(call);
                                }
                            }
                        }
                    }
                }
            }
        }

        // for (auto ent : loadInfos) {
        //     ent.first->print(std::cout);
        //     std::cout << "loads :" << std::endl;
        //     for (auto l : ent.second.loads) {
        //         std::cout << "\t";
        //         l->print(std::cout);
        //     }
        //     std::cout << "stores :" << std::endl;
        //     for (auto ss : ent.second.stores) {
        //         std::cout << "\t";
        //         ss->print(std::cout);
        //     }
        //     std::cout << std::endl;
        // }
    }
} // thm