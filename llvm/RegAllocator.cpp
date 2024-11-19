//
// Created by slty5 on 24-11-17.
//

#include "RegAllocator.h"

#include <iostream>

#include "../mips/Frame.h"

namespace thm {
    void RegAllocator::init() {
        for (auto arg : func->args) {
            addValue(arg);
        }
        for (auto bb : func->blocks) {
            for (auto inst : bb->insts) {
                if (inst->slot >= 0) {
                    addValue(inst);
                }
            }
        }
    }

    void RegAllocator::process() {
        func->livenessAnalysis();
        build();
        makeWorkList();
        do {
            if (!simplifyWorklist.empty()) simplify();
            else if (!worklistMoves.empty()) coalesce();
            else if (!freezeWorklist.empty()) freeze();
            else if (!spillWorklist.empty()) selectSpill();
        } while (!simplifyWorklist.empty() || !worklistMoves.empty() || !freezeWorklist.empty() || !spillWorklist.empty());
        assignColors();
        if (!spilledNodes.empty()) {
            rewriteProgram();
            process();
        }
    }

    void RegAllocator::build() {
        for (auto bb : func->blocks) {
            auto live = bb->out;
            for (auto iter = bb->insts.rbegin(); iter != bb->insts.rend(); ++iter) {
                auto inst = *iter;
                std::unordered_set<int> instDef;
                std::unordered_set<int> instUse;
                value[inst->slot] = inst;
                inst->getDefUse(instDef, instUse);
                if (MoveInst *move = dynamic_cast<MoveInst *>(inst)) {
                    if (move->src->slot >= 0) {
                        for (auto use : instUse) {
                            live.erase(use);
                            moveList[use].insert(move);
                        }
                        for (auto def : instDef) {
                            moveList[def].insert(move);
                        }
                        worklistMoves.insert(move);
                    }
                }
                for (auto def : instDef) {
                    live.insert(def);
                }

                for (auto def : instDef) {
                    for (auto l : live) {
                        addEdge(def, l);
                    }
                }
                for (auto def : instDef) {
                    live.erase(def);
                }
                for (auto use : instUse) {
                    live.insert(use);
                }
            }
        }
    }

    void RegAllocator::addValue(Value *val) {
        color[val->slot] = val->reg;
        if (val->reg != Register::NONE) {
            preColored.insert(val->slot);
        } else {
            initial.insert(val->slot);
        }
    }

    void RegAllocator::addEdge(int u, int v) {
        if (adjSet.find({u, v}) == adjSet.end() && u != v) {
            adjSet.insert({u, v});
            adjSet.insert({v, u});
            if (preColored.find(u) == preColored.end()) {
                adjList[u].insert(v);
                degree[u] += 1;
            }
            if (preColored.find(v) == preColored.end()) {
                adjList[v].insert(u);
                degree[v] += 1;
            }
        }
    }

    void RegAllocator::makeWorkList() {
        for (auto iter = initial.begin(); iter != initial.end(); ) {
            auto n = *iter;
            iter = initial.erase(iter);
            if (degree[n] >= K) {
                spillWorklist.insert(n);
            } else if (isMoveRelated(n)) {
                freezeWorklist.insert(n);
            } else {
                simplifyWorklist.insert(n);
            }
        }
    }

    bool RegAllocator::isMoveRelated(int u) {
        return !getMoves(u).empty();
    }

    std::unordered_set<MoveInst *> RegAllocator::getMoves(int u) {
        std::unordered_set<MoveInst *> moves;
        for (auto m : moveList[u]) {
            if (activeMoves.find(m) != activeMoves.end() || worklistMoves.find(m) != worklistMoves.end()) {
                moves.insert(m);
            }
        }
        return moves;
    }

    std::unordered_set<int> RegAllocator::adjacent(int u) {
        std::unordered_set<int> adj;
        for (auto v : adjList[u]) {
            if (selectSet.find(v) == selectSet.end() && coalescedNodes.find(v) == coalescedNodes.end()) {
                adj.insert(v);
            }
        }
        return adj;
    }

    void RegAllocator::simplify() {
        auto n = *simplifyWorklist.begin();
        simplifyWorklist.erase(simplifyWorklist.begin());
        selectStack.push_back(n);
        selectSet.insert(n);
        auto adj = adjacent(n);
        for (auto v : adj) {
            decrementDegree(v);
        }
    }

    void RegAllocator::decrementDegree(int n) {
        int d = degree[n];
        degree[n] -= 1;
        if (d == K) {
            auto adj = adjacent(n);
            adj.insert(n);
            enableMoves(adj);
            spillWorklist.erase(n);
            if (isMoveRelated(n)) {
                freezeWorklist.insert(n);
            } else {
                simplifyWorklist.insert(n);
            }
        }
    }

    void RegAllocator::enableMoves(std::unordered_set<int> const& nodes) {
        for (auto n : nodes) {
            auto moves = getMoves(n);
            for (auto m : moves) {
                if (activeMoves.find(m) != activeMoves.end()) {
                    activeMoves.erase(m);
                    worklistMoves.insert(m);
                }
            }
        }
    }

    void RegAllocator::coalesce() {
        auto m = *worklistMoves.begin();
        worklistMoves.erase(worklistMoves.begin());
        int x = m->src->slot;
        int y = m->dst->slot;
        x = getAlias(x);
        y = getAlias(y);
        int u, v;
        if (preColored.find(y) != preColored.end()) {
            u = y, v = x;
        } else {
            u = x, v = y;
        }
        if (u == v) {
            coalescedMoves.insert(m);
            addWorklist(u);
        } else if (preColored.find(v) != preColored.end() || adjSet.find({u, v}) != adjSet.end()) {
            constrainedMoves.insert(m);
            addWorklist(u);
            addWorklist(v);
        } else if ((preColored.find(u) != preColored.end() && judgeCoalesce(u, v)) || (preColored.find(u) == preColored.end() && judgeConservative(u, v))) {
            constrainedMoves.insert(m);
            combine(u, v);
            addWorklist(u);
        } else {
            activeMoves.insert(m);
        }
    }

    int RegAllocator::getAlias(int u) {
        if (coalescedNodes.find(u) != coalescedNodes.end()) {
            return getAlias(alias[u]);
        }
        return u;
    }

    bool RegAllocator::judgeCoalesce(int u, int v) {
        auto adj = adjacent(v);
        for (auto t : adj) {
            if (degree[t] < K || preColored.find(t) != preColored.end() || adjSet.find({t, u}) != adjSet.end()) {
                continue;
            }
            return false;
        }
        return true;
    }

    bool RegAllocator::judgeConservative(int u, int v) {
        auto adjU = adjacent(u);
        auto adjV = adjacent(v);
        int k = 0;
        for (auto t : adjU) {
            if (degree[t] >= K) k++;
        }
        for (auto t : adjV) {
            if (degree[t] >= K) k++;
        }
        return k < K;
    }

    void RegAllocator::combine(int u, int v) {
        if (freezeWorklist.find(v) != freezeWorklist.end()) {
            freezeWorklist.erase(v);
        } else {
            spillWorklist.erase(v);
        }
        coalescedNodes.insert(v);
        alias[v] = u;
        moveList[u].insert(moveList[v].begin(), moveList[v].end());
        enableMoves(std::unordered_set<int>({v}));
        auto adjV = adjacent(v);
        for (auto t : adjV) {
            addEdge(t, u);
            decrementDegree(t);
        }
        if (degree[u] >= K && freezeWorklist.find(u) != freezeWorklist.end()) {
            freezeWorklist.erase(u);
            spillWorklist.insert(u);
        }
    }

    void RegAllocator::addWorklist(int u) {
        if (preColored.find(u) == preColored.end() && !isMoveRelated(u) && degree[u] < K) {
            freezeWorklist.erase(u);
            simplifyWorklist.insert(u);
        }
    }

    void RegAllocator::freeze() {
        auto u = *freezeWorklist.begin();
        freezeWorklist.erase(u);
        simplifyWorklist.insert(u);
        freezeMoves(u);
    }

    void RegAllocator::freezeMoves(int u) {
        auto moves = getMoves(u);
        for (auto m : moves) {
            int x = m->src->slot;
            int y = m->dst->slot;
            int v;
            if (getAlias(y) == getAlias(u)) {
                v = getAlias(x);
            } else {
                v = getAlias(y);
            }
            activeMoves.erase(m);
            frozenMoves.insert(m);
            if (getMoves(v).empty() && degree[v] < K) {
                freezeWorklist.erase(v);
                simplifyWorklist.insert(v);
            }
        }
    }

    void RegAllocator::selectSpill() {
        auto m = *spillWorklist.begin();
        spillWorklist.erase(m);
        simplifyWorklist.insert(m);
        freezeMoves(m);
    }

    void RegAllocator::assignColors() {
        while (!selectStack.empty()) {
            auto n = selectStack.back();
            selectStack.pop_back();
            selectSet.erase(n);
            auto okColors = generalRegs;
            for (auto w : adjList[n]) {
                int wAlias = getAlias(w);
                if (colorNodes.find(wAlias) != colorNodes.end() || preColored.find(wAlias) != preColored.end()) {
                    okColors.erase(color[wAlias]);
                }
            }
            if (okColors.empty()) {
                spilledNodes.insert(n);
            } else {
                colorNodes.insert(n);
                color[n] = *okColors.begin();
            }
        }
        for (auto n : coalescedNodes) {
            color[n] = color[getAlias(n)];
        }
    }

    void RegAllocator::rewriteProgram() {
        std::unordered_map<int, AllocaInst *> pos;
        for (auto n : spilledNodes) {
            pos[n] = new AllocaInst(value[n]->valueType);
            pos[n]->slot = func->slotTracker.allocSlot();
            func->root->addInstAhead(pos[n]);
        }
        std::vector<LoadInst *> newTemps;
        // replace use
        for (auto n : spilledNodes) {
            for (auto bb : func->blocks) {
                for (auto iter = bb->insts.begin(); iter != bb->insts.end(); ++iter) {
                    auto inst = *iter;
                    LoadInst *l = nullptr;
                    for (auto use : inst->usings) {
                        if ((*use)->slot == n) {
                            if (l == nullptr) {
                                l = new LoadInst(pos[n]);
                                newTemps.push_back(l);
                                l->slot = func->slotTracker.allocSlot();
                                color[l->slot] = Register::NONE;
                            }
                            *use = l;
                        }
                    }
                    if (l != nullptr) {
                        bb->insts.insert(iter, l);
                    }
                }
            }
        }
        // replace def
        for (auto n : spilledNodes) {
            for (auto bb : func->blocks) {
                for (auto iter = bb->insts.begin(); iter != bb->insts.end(); ++iter) {
                    if ((*iter)->slot == n) {
                        bb->insts.insert(iter + 1, new StoreInst(value[n], pos[n]));
                    }
                }
            }
        }
        spilledNodes.clear();
        initial.clear();
        colorNodes.clear();
        coalescedNodes.clear();
        adjList.clear();
        adjSet.clear();
        degree.clear();
        moveList.clear();
        alias.clear();
        value.clear();
        coalescedMoves.clear();
        activeMoves.clear();
        worklistMoves.clear();
        frozenMoves.clear();
        constrainedMoves.clear();
        selectStack.clear();
        selectSet.clear();
        simplifyWorklist.clear();
        spillWorklist.clear();
        freezeWorklist.clear();
        preColored.clear();
        color.clear();
        init();
    }

    void RegAllocator::submitColors() {
        for (auto bb : func->blocks) {
            for (auto inst : bb->insts) {
                if (color.find(inst->slot) != color.end()) {
                    inst->reg = color[inst->slot];
                    if (MoveInst *move = dynamic_cast<MoveInst *>(inst)) {
                        move->dst->reg = move->reg;
                    }
                }
            }
        }
    }
} // thm