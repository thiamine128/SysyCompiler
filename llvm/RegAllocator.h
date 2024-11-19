//
// Created by slty5 on 24-11-17.
//

#ifndef REGALLOCATOR_H
#define REGALLOCATOR_H
#include <set>

#include "LLVM.h"

namespace thm {

class RegAllocator {
public:
    int K;
    Function *func;
    std::unordered_set<int> preColored;
    std::unordered_set<int> initial;
    std::unordered_set<int> simplifyWorklist;
    std::unordered_set<int> freezeWorklist;
    std::unordered_set<int> spillWorklist;
    std::unordered_set<int> spilledNodes;
    std::unordered_set<int> coalescedNodes;
    std::unordered_set<int> colorNodes;
    std::vector<int> selectStack;
    std::unordered_set<int> selectSet;

    std::unordered_set<MoveInst *> coalescedMoves;
    std::unordered_set<MoveInst *> constrainedMoves;
    std::unordered_set<MoveInst *> frozenMoves;
    std::unordered_set<MoveInst *> worklistMoves;
    std::unordered_set<MoveInst *> activeMoves;

    std::set<std::pair<int, int>> adjSet;
    std::unordered_map<int, std::unordered_set<int>> adjList;
    std::unordered_map<int, int> degree;
    std::unordered_map<int, std::unordered_set<MoveInst *>> moveList;
    std::unordered_map<int, int> alias;
    std::unordered_map<int, Register> color;
    std::unordered_map<int, Instruction *> value;

    RegAllocator(Function *func) : K(generalRegs.size()), func(func) {

    }

    void init();
    void process();
    void build();
    void addValue(Value *val);
    void addEdge(int u, int v);
    void makeWorkList();
    bool isMoveRelated(int u);
    std::unordered_set<MoveInst *> getMoves(int u);
    std::unordered_set<int> adjacent(int u);
    void simplify();
    void decrementDegree(int n);
    void enableMoves(std::unordered_set<int> const& nodes);
    void coalesce();
    int getAlias(int u);
    bool judgeCoalesce(int u, int v);
    bool judgeConservative(int u, int v);
    void combine(int u, int v);
    void addWorklist(int u);
    void freeze();
    void freezeMoves(int u);
    void selectSpill();
    void assignColors();
    void rewriteProgram();
    void submitColors();
};

} // thm

#endif //REGALLOCATOR_H
