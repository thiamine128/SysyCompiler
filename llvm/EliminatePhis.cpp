//
// Created by slty5 on 24-11-16.
//

#include "EliminatePhis.h"

#include <iostream>

namespace thm {
    void EliminatePhis::process() {
        for (auto func : module->functions) {
            processFunction(func);
        }
        processFunction(module->main);
    }

    void EliminatePhis::processFunction(Function *func) {
        processBasicBlockDebug(func->root);

        for (auto bb : func->blocks) {
            for (auto inst : bb->insts) {
                for (auto use : inst->usings) {
                    if (PhiInst *phi = dynamic_cast<PhiInst *>(*use)) {
                        *use = phiLoad[phi];
                    }
                }
            }
        }
    }

    void EliminatePhis::processBasicBlock(BasicBlock *bb) {
        std::unordered_map<BasicBlock *, BasicBlock *> splited;
        std::unordered_map<BasicBlock *, std::vector<std::pair<Value *, Value *>>> pcs;

        for (auto iter = bb->insts.begin(); iter != bb->insts.end(); ) {
            if (PhiInst *phi = dynamic_cast<PhiInst *>(*iter)) {
                for (auto ent : phi->opt) {
                    if (ent.first->tos.size() == 1) {
                        pcs[ent.first].push_back(std::make_pair(phi, ent.second));
                    } else {
                        // critical edge
                        BasicBlock *split = nullptr;
                        if (splited.find(ent.first) == splited.end()) {
                            split = new BasicBlock(bb->function, bb->loopNest);
                            bb->function->blocks.push_back(split);
                            BranchInst *br = static_cast<BranchInst *>(ent.first->insts.back());
                            if (br->ifTrue == bb) {
                                br->ifTrue = split;
                            }
                            if (br->ifFalse == bb) {
                                br->ifFalse = split;
                            }
                            split->addInst(new BranchInst(bb));
                            splited[ent.first] = split;
                        } else {
                            split = splited[ent.first];
                        }

                        pcs[split].push_back(std::make_pair(phi, ent.second));
                    }
                }
                iter = bb->insts.erase(iter);
            } else {
                ++iter;
            }
        }
        for (auto pc : pcs) {
            auto moves = toSequence(pc.second);
            for (auto move : moves) {
                pc.first->addInstLastSecond(move);
            }
        }
        for (auto child : bb->iDomChildren) {
            processBasicBlock(child);
        }
    }

    std::vector<MoveInst *> EliminatePhis::toSequence(std::vector<std::pair<Value *, Value *>> pc) {
        std::vector<MoveInst *> moves;
        while (true) {
            bool allTheSame = true;
            std::unordered_set<Value *> used;
            for (auto mv : pc) {
                if (mv.first != mv.second) {
                    allTheSame = false;
                }
                used.insert(mv.second);
            }
            if (allTheSame) break;
            bool directRemove = false;
            std::vector<std::pair<Value *, Value *>>::iterator cp;
            for (auto iter = pc.begin(); iter != pc.end(); ++iter) {
                cp = iter;
                if (used.find(iter->first) == used.end()) {
                    directRemove = true;
                    moves.push_back(new MoveInst(iter->first, iter->second));
                    pc.erase(iter);
                    break;
                }
            }
            if (directRemove) continue;
            MoveTmp* moveTmp = new MoveTmp();
            moves.push_back(new MoveInst(moveTmp, cp->second));
            cp->second = moveTmp;
        }
        return moves;
    }

    void EliminatePhis::processBasicBlockDebug(BasicBlock *bb) {
        std::unordered_map<BasicBlock *, BasicBlock *> splited;
        std::unordered_map<BasicBlock *, std::vector<std::pair<Value *, Value *>>> pcs;

        for (auto iter = bb->insts.begin(); iter != bb->insts.end(); ) {
            if (PhiInst *phi = dynamic_cast<PhiInst *>(*iter)) {
                phiLoc[phi] = new AllocaInst(phi->valueType);
                for (auto ent : phi->opt) {
                    if (ent.first->tos.size() == 1) {
                        pcs[ent.first].push_back(std::make_pair(phiLoc[phi], ent.second));
                    } else {
                        // critical edge
                        BasicBlock *split = nullptr;
                        if (splited.find(ent.first) == splited.end()) {
                            split = new BasicBlock(bb->function, bb->loopNest);
                            bb->function->blocks.push_back(split);
                            BranchInst *br = static_cast<BranchInst *>(ent.first->insts.back());
                            if (br->ifTrue == bb) {
                                br->ifTrue = split;
                            }
                            if (br->ifFalse == bb) {
                                br->ifFalse = split;
                            }
                            split->addInst(new BranchInst(bb));
                            splited[ent.first] = split;
                        } else {
                            split = splited[ent.first];
                        }

                        pcs[split].push_back(std::make_pair(phiLoc[phi], ent.second));
                    }
                }
                auto load = new LoadInst(phiLoc[phi]);
                phiLoad[phi] = load;
                iter = bb->insts.erase(iter);
                iter = bb->insts.insert(iter, load);
            } else {
                ++iter;
            }
        }
        for (auto ent : phiLoc) {
            bb->function->blocks[0]->addInstAhead(ent.second);
        }
        for (auto pc : pcs) {
            auto moves = toMemory(pc.second);
            for (auto move : moves) {
                pc.first->addInstLastSecond(move);
            }
        }
        for (auto child : bb->iDomChildren) {
            processBasicBlockDebug(child);
        }
    }

    std::vector<StoreInst *> EliminatePhis::toMemory(std::vector<std::pair<Value *, Value *>> pc) {
        std::vector<StoreInst *> moves;
        for (auto iter = pc.begin(); iter != pc.end(); ++iter) {
            moves.push_back(new StoreInst(iter->second, iter->first));
        }
        return moves;
    }
} // thm