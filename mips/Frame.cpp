//
// Created by slty5 on 24-11-10.
//

#include "Frame.h"

#include <iostream>

#include "../llvm/LLVM.h"

namespace thm {
    Frame::Frame(Function *function) : function(function) {
    }

    void Frame::init(int maxCallArgs) {
        std::unordered_set<Register> usedRegister;
        for (auto inst : function->root->insts) {
            if (AllocaInst *alloca = dynamic_cast<AllocaInst*>(inst)) {
                int size = 0;
                if (ArrayValueType *arrayType = dynamic_cast<ArrayValueType *>(alloca->allocType)) {
                    if (BasicValueType *basicType = dynamic_cast<BasicValueType *>(arrayType->value)) {
                        switch (basicType->basicType) {
                            case BasicValueType::I32:
                                size = 4 * arrayType->arrayLen;
                                break;
                            case BasicValueType::I8:
                                size = arrayType->arrayLen;
                                break;
                            default:
                                break;
                        }
                    }
                } else {
                    size = 4;
                }
                size = (size + 3) / 4 * 4;
                frameSize += size;
                offset[alloca] = frameSize;
            }
        }
        for (auto bb : function->blocks) {
            for (auto inst : bb->insts) {
                if (generalRegs.find(inst->reg) != generalRegs.end()) {
                    usedRegister.insert(inst->reg);
                }
            }
        }
        if (function->name != "main") {
            usedRegister.insert(Register::RA);
            for (auto reg : usedRegister) {
                frameSize += 4;
                saved[reg] = frameSize;
            }
        }
        frameSize += 4 * maxCallArgs;
    }

    int Frame::getOffset(AllocaInst *slot) {
        return frameSize - offset[slot];
    }

    int Frame::getRegOffset(Register reg) {
        return frameSize - saved[reg];
    }

    int Frame::getCallArgOffset(int i) {
        return 4 * i;
    }
} // thm