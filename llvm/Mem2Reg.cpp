//
// Created by slty5 on 24-11-13.
//

#include "Mem2Reg.h"

#include <functional>
#include <iostream>
#include <ostream>

namespace thm {
    void Mem2Reg::process() {
        for (auto func : module->functions) {
            processFunction(func);
        }
        processFunction(module->main);
    }

    void Mem2Reg::processFunction(Function *function) {
        std::unordered_set<AllocaInst *> promoted;
        for (AllocaInst *alloca : function->allocas) {
            if (alloca->isPromotable()) {
                promoted.insert(alloca);
                promote(function, alloca);
            } else {
            }
        }

        std::vector<BasicBlock *> blocks;
        blocks.push_back(function->blocks[0]);
        std::unordered_map<BasicBlock *, bool> vis;
        while (!blocks.empty()) {
            auto bb = blocks.front();
            vis[bb] = true;
            blocks.erase(blocks.begin());
            for (auto iter = bb->insts.begin(); iter != bb->insts.end();) {
                auto inst = *iter;
                bool shouldRemove = false;
                AllocaInst *alloca = nullptr;
                StoreInst *store;
                LoadInst *load;
                PhiInst *phi;
                switch (inst->type()) {
                    case LLVMType::ALLOCA_INST:
                        alloca = static_cast<AllocaInst *>(inst);
                        if (promoted.find(alloca) != promoted.end()) {
                            shouldRemove = true;
                        }
                        break;
                    case LLVMType::STORE_INST:
                        store = static_cast<StoreInst *>(inst);
                        if (AllocaInst *alloc = dynamic_cast<AllocaInst *>(store->ptr)) {
                            if (promoted.find(alloc) != promoted.end()) {
                                shouldRemove = true;
                                bb->allocaTracker[alloc] = store->value;
                            }
                        }
                        break;
                    case LLVMType::LOAD_INST:
                        load = static_cast<LoadInst *>(inst);
                        if (AllocaInst *alloc = dynamic_cast<AllocaInst *>(load->ptr)) {
                            if (promoted.find(alloc) != promoted.end()) {
                                shouldRemove = true;
                                for (auto toReplace : bb->insts) {
                                    replaceUse(toReplace, load, bb->allocaTracker[alloc]);
                                }
                            }
                        }
                        break;
                    case LLVMType::PHI_INST:
                        phi = static_cast<PhiInst *>(inst);
                        bb->allocaTracker[phi->alloc] = phi;
                    break;
                    default:
                        break;
                }
                if (shouldRemove) {
                    iter = bb->insts.erase(iter);
                } else {
                    ++iter;
                }
            }
            for (auto to : bb->tos) {
                for (auto ent : to->phis) {
                    if (bb->allocaTracker.find(ent.first) != bb->allocaTracker.end()) {
                        ent.second->opt[bb] = bb->allocaTracker[ent.first];
                    }
                }
                to->allocaTracker = bb->allocaTracker;
                if (!vis[to]) {
                    blocks.push_back(to);
                }
            }
        }
    }

    void Mem2Reg::promote(Function *function, AllocaInst *alloca) {
        std::unordered_set<BasicBlock *> defs, w;
        for (auto user : alloca->usedBys) {
            if (StoreInst *store = dynamic_cast<StoreInst *>(user)) {
                defs.insert(store->block);
            }
        }
        std::unordered_map<BasicBlock *, bool> vis;
        w = defs;
        while (!w.empty()) {
            auto block = *w.begin();
            w.erase(w.begin());
            for (auto b : block->df) {
                if (!vis[b]) {
                    b->phis[alloca] = new PhiInst(alloca);
                    b->addInstAhead(b->phis[alloca]);
                    vis[b] = true;
                    if (defs.find(b) == defs.end()) {
                        w.insert(b);
                    }
                }
            }
        }
    }

    void Mem2Reg::replaceUse(Instruction *inst, Value *src, Value *dst) {
        for (auto v : inst->usings) {
            if (*v == src) {
                *v = dst;
            }
        }
    }
} // thm