//
// Created by slty5 on 24-11-14.
//

#include "GCM.h"

#include <functional>
#include <functional>
#include <functional>
#include <iostream>

namespace thm {
    void GCM::process() {
        for (auto function : module->functions) {
            processFunction(function);
        }
        processFunction(module->main);
    }

    void GCM::processFunction(Function *function) {
        function->root->domDepth = 0;
        std::unordered_set<BasicBlock *> q;
        q.insert(function->root);
        while (!q.empty()) {
            auto bb = *q.begin();
            q.erase(q.begin());
            for (auto ch : bb->iDomChildren) {
                ch->domDepth = bb->domDepth + 1;
                q.insert(ch);
            }
        }
        vis.clear();
        for (auto bb : function->blocks) {
            for (auto inst : bb->insts) {
                if (inst->pinned) {
                    vis[inst] = true;
                    for (auto use : inst->usings) {
                        if (Instruction *i = dynamic_cast<Instruction *>(*use)) {
                            scheduleEarly(function, i);
                        }
                    }
                }
            }
        }
        vis.clear();
        for (auto bb : function->blocks) {
            for (auto inst : bb->insts) {
                if (inst->pinned) {
                    vis[inst] = true;
                    for (auto user : inst->usedBys) {
                        if (Instruction *i = dynamic_cast<Instruction *>(user)) {
                            scheduleLate(function, i);
                        }
                    }
                }
            }
        }
        for (auto bb : function->blocks) {
            for (auto iter = bb->insts.begin(); iter != bb->insts.end();) {
                if ((*iter)->block != bb) {
                    auto inst = *iter;
                    iter = bb->insts.erase(iter);
                    inst->block->addInstLastSecond(inst);
                } else {
                    ++iter;
                }
            }
        }
    }

    void GCM::scheduleEarly(Function *func, Instruction *inst) {
        if (vis[inst] || inst->pinned) return;
        vis[inst] = true;
        inst->block = func->root;
        for (auto use : inst->usings) {
            if (Instruction *i = dynamic_cast<Instruction *>(*use)) {
                scheduleEarly(func, i);
                if (i->block->domDepth > inst->block->domDepth && !inst->pinned) {
                    inst->block = i->block;
                }
            }
        }
    }

    void GCM::scheduleLate(Function *func, Instruction *inst) {
        if (vis[inst] || inst->pinned) return;
        vis[inst] = true;
        BasicBlock *lca = nullptr;
        for (auto used : inst->usedBys) {
            if (Instruction *i = dynamic_cast<Instruction *>(used)) {
                scheduleLate(func, i);
                BasicBlock *use = i->block;
                if (PhiInst *phi = dynamic_cast<PhiInst *>(i)) {
                    for (auto ent : phi->opt) {
                        if (ent.second == inst) {
                            use = ent.first;
                        }
                    }
                }
                lca = findLCA(lca, use);
            }
        }
        if (lca != nullptr) {
            BasicBlock *best = lca;
            while (lca != nullptr && lca->domDepth >= inst->block->domDepth) {
                if (lca->loopNest < best->loopNest) {
                    best = lca;
                }
                lca = lca->iDom;
            }
            inst->block = best;
        }
    }

    BasicBlock * GCM::findLCA(BasicBlock *bb1, BasicBlock *bb2) {
        if (bb1 == nullptr) return bb2;
        while (bb1->domDepth > bb2->domDepth) {
            bb1 = bb1->iDom;
        }
        while (bb2->domDepth > bb1->domDepth) {
            bb2 = bb2->iDom;
        }
        while (bb1 != bb2) {
            bb1 = bb1->iDom;
            bb2 = bb2->iDom;
        }
        return bb1;
    }
} // thm