//
// Created by slty5 on 24-11-13.
//

#include "Mem2Reg.h"

#include <assert.h>
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
        vis[blocks[0]] = true;
        while (!blocks.empty()) {
            auto bb = blocks.front();
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
                        if (BasicValueType *basicType = dynamic_cast<BasicValueType *>(alloca->allocType)) {
                            bb->allocaTracker[alloca] = new Undef(basicType->basicType);
                        }
                        break;
                    case LLVMType::STORE_INST:
                        store = static_cast<StoreInst *>(inst);
                        if (AllocaInst *alloc = dynamic_cast<AllocaInst *>(store->ptr)) {
                            if (promoted.find(alloc) != promoted.end()) {
                                bb->allocaTracker[alloc] = store->value;
                                shouldRemove = true;
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
                    inst->onRemove();
                } else {
                    ++iter;
                }
            }
            for (auto to : bb->tos) {
                for (auto ent : to->phis) {
                    ent.second->addOpt(bb, bb->allocaTracker[ent.first]);
                }
                to->allocaTracker = bb->allocaTracker;
                if (!vis[to]) {
                    vis[to] = true;
                    blocks.push_back(to);
                }
            }
        }
        collapsePhi(function);
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
                int bid;
                for ( bid = 0; function->blocks[bid] != b; bid++) {

                }
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
                dst->usedBys.push_back(inst);
                for (auto it = src->usedBys.begin(); it != src->usedBys.end();) {
                    if (*it == inst) {
                        src->usedBys.erase(it);
                        break;
                    }
                    it++;
                }
            }
        }
    }

    void Mem2Reg::collapsePhi(Function *function) {
        std::unordered_set<PhiInst *> phis;
        std::unordered_set<PhiInst *> toRemove;
        for (auto bb : function->blocks) {
            for (auto inst : bb->insts) {
                if (PhiInst *phi = dynamic_cast<PhiInst *>(inst)) {
                    if (!phi->isNecessary()) {
                        phis.insert(phi);
                        toRemove.insert(phi);
                    }
                }
            }
        }
        while (!phis.empty()) {
            auto phi = *phis.begin();
            phis.erase(phi);
            Value *elm = nullptr;
            for (auto ent : phi->opt) {
                if (ent.second != phi) {
                    elm = ent.second;
                    break;
                }
            }
            auto usedBys = phi->usedBys;
            for (auto used : usedBys) {
                if (used == phi) continue;
                if (Instruction *inst = dynamic_cast<Instruction *>(used)) {
                    replaceUse(inst, phi, elm);
                }
                if (PhiInst *p = dynamic_cast<PhiInst *>(used)) {
                    if (!p->isNecessary()) {
                        phis.insert(p);
                        toRemove.insert(p);
                    }
                }
            }
        }
        for (auto bb : function->blocks) {
            for (auto iter = bb->insts.begin(); iter != bb->insts.end();) {
                if (toRemove.find(static_cast<PhiInst *>(*iter)) != toRemove.end()) {
                    Instruction *inst = static_cast<Instruction *>(*iter);
                    inst->onRemove();
                    iter = bb->insts.erase(iter);
                } else {
                    ++iter;
                }
            }
        }
    }
} // thm