//
// Created by slty5 on 24-11-25.
//

#include "AnalyzeDominance.h"

namespace thm {
    void AnalyzeDominance::process() {
        for (auto function : module->functions) {
            analyzeFunction(function);
        }
        analyzeFunction(module->main);
    }

    void AnalyzeDominance::analyzeFunction(Function *func) {
        std::unordered_set<BasicBlock *> all;
        for (auto bb : func->blocks) {
            all.insert(bb);
        }
        for (auto bb : func->blocks) {
            if (bb != func->blocks[0])
                bb->doms = all;
        }
        func->blocks[0]->doms.insert(func->blocks[0]);
        bool changed = true;
        while (changed) {
            changed = false;
            for (int i = 1; i < func->blocks.size(); i++) {
                std::unordered_set<BasicBlock *> newDom;
                newDom.insert(func->blocks[i]);
                if (func->blocks[i]->froms.empty()) continue;
                for (auto candidate : func->blocks[i]->froms[0]->doms) {
                    bool valid = true;
                    for (auto from : func->blocks[i]->froms) {
                        if (from->doms.find(candidate) == from->doms.end()) {
                            valid = false;
                            break;
                        }
                    }
                    if (valid) {
                        newDom.insert(candidate);
                    }
                }
                if (newDom != func->blocks[i]->doms) {
                    changed = true;
                    func->blocks[i]->doms = newDom;
                }
            }
        }
        for (int i = 1; i < func->blocks.size(); ++i) {
            for (auto dom : func->blocks[i]->doms) {
                // dom is strict dom of i
                if (dom != func->blocks[i]) {
                    bool valid = true;
                    for (auto d : func->blocks[i]->doms) {
                        // d is strict dom of i
                        if (d != func->blocks[i] && d != dom) {
                            if (d->doms.find(dom) != d->doms.end()) {
                                valid = false;
                                break;
                            }
                        }
                    }
                    if (valid) {
                        func->blocks[i]->iDom = dom;
                        dom->iDomChildren.push_back(func->blocks[i]);
                        break;
                    }
                }
            }
        }
        for (int i = 0; i < func->blocks.size(); ++i) {
            for (auto to : func->blocks[i]->tos) {
                BasicBlock *x = func->blocks[i];
                while (!(x != to && to->doms.find(x) != to->doms.end())) {
                    x->df.insert(to);
                    x = x->iDom;
                    if (x == nullptr) break;
                }
            }
        }
    }
} // thm