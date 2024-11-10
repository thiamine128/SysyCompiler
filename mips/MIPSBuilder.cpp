//
// Created by slty5 on 24-11-7.
//

#include "MIPSBuilder.h"

#include <complex>
#include <functional>
#include <functional>
#include <iostream>
#include <ostream>

#include "Frame.h"
#include "../util/util.h"

namespace thm {
    Register regParams[4] = {Register::A0, Register::A1, Register::A2, Register::A3};

    void MIPSBuilder::build() {
        os << ".data" << std::endl;
        for (GlobalVariable *global : module->variables) {
            addGlobalVariable(global);
        }
        for (StringLiteral *str : module->strings) {
            addStringLiteral(str);
        }
        os << ".text" << std::endl;
        translateFunction(module->main);
        for (Function *function : module->functions) {
            translateFunction(function);
        }
    }

    void MIPSBuilder::addGlobalVariable(GlobalVariable *global) {
        os << "\t" << global->name << ": ";
        PtrValueType *ptr = static_cast<PtrValueType *>(global->valueType);
        if (ptr->value->type() == ValueType::BASIC) {
            BasicValueType *basic = static_cast<BasicValueType *>(ptr->value);
            switch (basic->basicType) {
                case BasicValueType::I32:
                    os << " .word ";
                    break;
                case BasicValueType::I8:
                    os << " .byte ";
                    break;
                default:
                    break;
            }
            os << global->initVal << std::endl;
        } else {
            ArrayValueType *array = static_cast<ArrayValueType *>(ptr->value);
            BasicValueType *basic = static_cast<BasicValueType *>(array->value);
            switch (basic->basicType) {
                case BasicValueType::I32:
                    os << " .word ";
                break;
                case BasicValueType::I8:
                    os << " .byte ";
                break;
                default:
                    break;
            }
            if (global->zeroInit) {
                os << "0:" << array->arrayLen << std::endl;
            } else {
                for (int i = 0; i < array->arrayLen; i++) {
                    os << global->initVals[i];
                    if (i + 1 < array->arrayLen) {
                        os << ", ";
                    }
                }
                os << std::endl;
            }
        }
    }

    void MIPSBuilder::addStringLiteral(StringLiteral *str) {
        os << "\t" << ".str";
        if (str->refId) {
            os << "." << str->refId;
        }
        os << ": .asciiz \"";
        for (int i = 0; i < str->value.length(); i++) {
            os << unescape(str->value[i]);
        }
        os << "\"" << std::endl;
    }

    void MIPSBuilder::translateFunction(Function *function) {
        function->slotTracker.prepareCallArgs(function->getMaxArgs());
        function->frame = function->slotTracker.establishFrame();
        submitText(new MIPSLabel(function->name));
        submitText(MIPSInst::AddImm(Register::SP, Register::SP, -function->frame->frameSize));
        submitText(MIPSInst::SaveWord(Register::RA, function->frame->frameSize - 4, Register::SP));
        for (int i = 0; i < function->args.size(); i++) {
            if (i < 4) {
                submitText(MIPSInst::SaveWord(regParams[i], function->frame->frameSize + 4 * i, Register::SP));
            }
        }
        for (BasicBlock *block : function->blocks) {
            translateBlock(block);
        }
    }

    void MIPSBuilder::translateBlock(BasicBlock *block) {
        os << block->function->name << "." << block->blockIdx << ":" << std::endl;
        for (Instruction *inst : block->insts) {
            switch (inst->type()) {
                case LLVMType::BINARY_INST:
                    translateBinaryInst(block->function, static_cast<BinaryInst *>(inst));
                    break;
                case LLVMType::CALL_INST:
                    translateCallInst(block->function, static_cast<CallInst *>(inst));
                    break;
                case LLVMType::ALLOCA_INST:
                    break;
                case LLVMType::LOAD_INST:
                    translateLoadInst(block->function, static_cast<LoadInst *>(inst));
                    break;
                case LLVMType::STORE_INST:
                    translateStoreInst(block->function, static_cast<StoreInst *>(inst));
                    break;
                case LLVMType::GET_ELEMENT_PTR_INST:
                    translateGetElementPtrInst(block->function, static_cast<GetElementPtr *>(inst));
                    break;
                case LLVMType::BRANCH_INST:
                    translateBranchInst(block->function, static_cast<BranchInst *>(inst));
                    break;
                case LLVMType::RET_INST:
                    translateRetInst(block->function, static_cast<RetInst *>(inst));
                    break;
                case LLVMType::ZEXT_INST:
                    translateZextInst(block->function, static_cast<ZextInst *>(inst));
                    break;
                case LLVMType::TRUNC_INST:
                    translateTruncInst(block->function, static_cast<TruncInst *>(inst));
                    break;
                default:
                    break;
            }
        }
    }

    void MIPSBuilder::translateBinaryInst(Function *function, BinaryInst *binaryInst) {
        bool lConst = true;
        bool rConst = true;
        if (binaryInst->l->type() != LLVMType::NUMERIC_LITERAL) {
            loadSlot(function, binaryInst->l->slot, Register::T0);
            lConst = false;
        }
        if (binaryInst->r->type() != LLVMType::NUMERIC_LITERAL) {
            loadSlot(function, binaryInst->r->slot, Register::T1);
            rConst = false;
        }
        switch (binaryInst->op) {
            case BinaryInst::ADD:
                if (lConst) {
                    NumericLiteral *lNum = static_cast<NumericLiteral *>(binaryInst->l);
                    if (rConst) {
                        NumericLiteral *rNum = static_cast<NumericLiteral *>(binaryInst->r);
                        submitText(MIPSInst::LoadImm(Register::T0, lNum->value));
                        submitText(MIPSInst::AddImm(Register::T2, Register::T0, rNum->value));
                    } else {
                        submitText(MIPSInst::AddImm(Register::T2, Register::T1, lNum->value));
                    }
                } else {
                    if (rConst) {
                        NumericLiteral *rNum = static_cast<NumericLiteral *>(binaryInst->r);
                        submitText(MIPSInst::AddImm(Register::T2, Register::T0, rNum->value));
                    } else {
                        submitText(MIPSInst::Add(Register::T2, Register::T0, Register::T1));
                    }
                }
                break;
            case BinaryInst::SUB:
                if (lConst) {
                    NumericLiteral *lNum = static_cast<NumericLiteral *>(binaryInst->l);
                    if (rConst) {
                        NumericLiteral *rNum = static_cast<NumericLiteral *>(binaryInst->r);
                        submitText(MIPSInst::LoadImm(Register::T0, lNum->value));
                        submitText(MIPSInst::SubImm(Register::T2, Register::T0, rNum->value));
                    } else {
                        submitText(MIPSInst::LoadImm(Register::T0, lNum->value));
                        submitText(MIPSInst::Sub(Register::T2, Register::T0, Register::T1));
                    }
                } else {
                    if (rConst) {
                        NumericLiteral *rNum = static_cast<NumericLiteral *>(binaryInst->r);
                        submitText(MIPSInst::SubImm(Register::T2, Register::T0, rNum->value));
                    } else {
                        submitText(MIPSInst::Sub(Register::T2, Register::T0, Register::T1));
                    }
                }
                break;
            case BinaryInst::MUL:
                if (lConst) {
                    NumericLiteral *lNum = static_cast<NumericLiteral *>(binaryInst->l);
                    if (rConst) {
                        NumericLiteral *rNum = static_cast<NumericLiteral *>(binaryInst->r);
                        submitText(MIPSInst::LoadImm(Register::T0, lNum->value));
                        submitText(MIPSInst::MulImm(Register::T2, Register::T0, rNum->value));
                    } else {
                        submitText(MIPSInst::MulImm(Register::T2, Register::T1, lNum->value));
                    }
                } else {
                    if (rConst) {
                        NumericLiteral *rNum = static_cast<NumericLiteral *>(binaryInst->r);
                        submitText(MIPSInst::MulImm(Register::T2, Register::T0, rNum->value));
                    } else {
                        submitText(MIPSInst::Mul(Register::T2, Register::T0, Register::T1));
                    }
                }
                break;
            case BinaryInst::SDIV:
                if (lConst) {
                    NumericLiteral *lNum = static_cast<NumericLiteral *>(binaryInst->l);
                    if (rConst) {
                        NumericLiteral *rNum = static_cast<NumericLiteral *>(binaryInst->r);
                        submitText(MIPSInst::LoadImm(Register::T0, lNum->value));
                        submitText(MIPSInst::DivImm(Register::T2, Register::T0, rNum->value));
                    } else {
                        submitText(MIPSInst::LoadImm(Register::T0, lNum->value));
                        submitText(MIPSInst::Div(Register::T2, Register::T0, Register::T1));
                    }
                } else {
                    if (rConst) {
                        NumericLiteral *rNum = static_cast<NumericLiteral *>(binaryInst->r);
                        submitText(MIPSInst::DivImm(Register::T2, Register::T0, rNum->value));
                    } else {
                        submitText(MIPSInst::Div(Register::T2, Register::T0, Register::T1));
                    }
                }
                break;
            case BinaryInst::SREM:
                if (lConst) {
                    NumericLiteral *lNum = static_cast<NumericLiteral *>(binaryInst->l);
                    if (rConst) {
                        NumericLiteral *rNum = static_cast<NumericLiteral *>(binaryInst->r);
                        submitText(MIPSInst::LoadImm(Register::T0, lNum->value));
                        submitText(MIPSInst::RemImm(Register::T2, Register::T0, rNum->value));
                    } else {
                        submitText(MIPSInst::LoadImm(Register::T0, lNum->value));
                        submitText(MIPSInst::Rem(Register::T2, Register::T0, Register::T1));
                    }
                } else {
                    if (rConst) {
                        NumericLiteral *rNum = static_cast<NumericLiteral *>(binaryInst->r);
                        submitText(MIPSInst::RemImm(Register::T2, Register::T0, rNum->value));
                    } else {
                        submitText(MIPSInst::Rem(Register::T2, Register::T0, Register::T1));
                    }
                }
                break;
            case BinaryInst::AND:
                // And is not used
                break;
            case BinaryInst::OR:
                // Or is not used
                break;
            case BinaryInst::EQ:
                if (lConst) {
                    NumericLiteral *lNum = static_cast<NumericLiteral *>(binaryInst->l);
                    if (rConst) {
                        NumericLiteral *rNum = static_cast<NumericLiteral *>(binaryInst->r);
                        submitText(MIPSInst::LoadImm(Register::T0, lNum->value));
                        submitText(MIPSInst::EqImm(Register::T2, Register::T0, rNum->value));
                    } else {
                        submitText(MIPSInst::EqImm(Register::T2, Register::T1, lNum->value));
                    }
                } else {
                    if (rConst) {
                        NumericLiteral *rNum = static_cast<NumericLiteral *>(binaryInst->r);
                        submitText(MIPSInst::EqImm(Register::T2, Register::T0, rNum->value));
                    } else {
                        submitText(MIPSInst::Eq(Register::T2, Register::T0, Register::T1));
                    }
                }
                break;
            case BinaryInst::NE:
                if (lConst) {
                    NumericLiteral *lNum = static_cast<NumericLiteral *>(binaryInst->l);
                    if (rConst) {
                        NumericLiteral *rNum = static_cast<NumericLiteral *>(binaryInst->r);
                        submitText(MIPSInst::LoadImm(Register::T0, lNum->value));
                        submitText(MIPSInst::NeqImm(Register::T2, Register::T0, rNum->value));
                    } else {
                        submitText(MIPSInst::NeqImm(Register::T2, Register::T1, lNum->value));
                    }
                } else {
                    if (rConst) {
                        NumericLiteral *rNum = static_cast<NumericLiteral *>(binaryInst->r);
                        submitText(MIPSInst::NeqImm(Register::T2, Register::T0, rNum->value));
                    } else {
                        submitText(MIPSInst::Neq(Register::T2, Register::T0, Register::T1));
                    }
                }
                break;
            case BinaryInst::SGE:
                if (lConst) {
                    NumericLiteral *lNum = static_cast<NumericLiteral *>(binaryInst->l);
                    if (rConst) {
                        NumericLiteral *rNum = static_cast<NumericLiteral *>(binaryInst->r);
                        submitText(MIPSInst::LoadImm(Register::T0, lNum->value));
                        submitText(MIPSInst::SgeImm(Register::T2, Register::T0, rNum->value));
                    } else {
                        submitText(MIPSInst::LoadImm(Register::T0, lNum->value));
                        submitText(MIPSInst::Sge(Register::T2, Register::T0, Register::T1));
                    }
                } else {
                    if (rConst) {
                        NumericLiteral *rNum = static_cast<NumericLiteral *>(binaryInst->r);
                        submitText(MIPSInst::SgeImm(Register::T2, Register::T0, rNum->value));
                    } else {
                        submitText(MIPSInst::Sge(Register::T2, Register::T0, Register::T1));
                    }
                }
                break;
            case BinaryInst::SGT:
                if (lConst) {
                    NumericLiteral *lNum = static_cast<NumericLiteral *>(binaryInst->l);
                    if (rConst) {
                        NumericLiteral *rNum = static_cast<NumericLiteral *>(binaryInst->r);
                        submitText(MIPSInst::LoadImm(Register::T0, lNum->value));
                        submitText(MIPSInst::SgtImm(Register::T2, Register::T0, rNum->value));
                    } else {
                        submitText(MIPSInst::LoadImm(Register::T0, lNum->value));
                        submitText(MIPSInst::Sgt(Register::T2, Register::T0, Register::T1));
                    }
                } else {
                    if (rConst) {
                        NumericLiteral *rNum = static_cast<NumericLiteral *>(binaryInst->r);
                        submitText(MIPSInst::SgtImm(Register::T2, Register::T0, rNum->value));
                    } else {
                        submitText(MIPSInst::Sgt(Register::T2, Register::T0, Register::T1));
                    }
                }
                break;
            case BinaryInst::SLE:
                if (lConst) {
                    NumericLiteral *lNum = static_cast<NumericLiteral *>(binaryInst->l);
                    if (rConst) {
                        NumericLiteral *rNum = static_cast<NumericLiteral *>(binaryInst->r);
                        submitText(MIPSInst::LoadImm(Register::T0, lNum->value));
                        submitText(MIPSInst::SleImm(Register::T2, Register::T0, rNum->value));
                    } else {
                        submitText(MIPSInst::LoadImm(Register::T0, lNum->value));
                        submitText(MIPSInst::Sle(Register::T2, Register::T0, Register::T1));
                    }
                } else {
                    if (rConst) {
                        NumericLiteral *rNum = static_cast<NumericLiteral *>(binaryInst->r);
                        submitText(MIPSInst::SleImm(Register::T2, Register::T0, rNum->value));
                    } else {
                        submitText(MIPSInst::Sle(Register::T2, Register::T0, Register::T1));
                    }
                }
                break;
            case BinaryInst::SLT:
                if (lConst) {
                    NumericLiteral *lNum = static_cast<NumericLiteral *>(binaryInst->l);
                    if (rConst) {
                        NumericLiteral *rNum = static_cast<NumericLiteral *>(binaryInst->r);
                        submitText(MIPSInst::LoadImm(Register::T0, lNum->value));
                        submitText(MIPSInst::SltImm(Register::T2, Register::T0, rNum->value));
                    } else {
                        submitText(MIPSInst::LoadImm(Register::T0, lNum->value));
                        submitText(MIPSInst::Slt(Register::T2, Register::T0, Register::T1));
                    }
                } else {
                    if (rConst) {
                        NumericLiteral *rNum = static_cast<NumericLiteral *>(binaryInst->r);
                        submitText(MIPSInst::SltImm(Register::T2, Register::T0, rNum->value));
                    } else {
                        submitText(MIPSInst::Slt(Register::T2, Register::T0, Register::T1));
                    }
                }
                break;
        }
        submitText(MIPSInst::SaveWord(Register::T2, function->frame->slotOffset[binaryInst->slot], Register::SP));
    }

    void MIPSBuilder::translateCallInst(Function *function, CallInst *callInst) {
        if (callInst->name == "getint") {
            submitText(MIPSInst::LoadImm(Register::V0, 5));
            submitText(MIPSInst::Syscall());
        } else if (callInst->name == "getchar") {
            submitText(MIPSInst::LoadImm(Register::V0, 12));
            submitText(MIPSInst::Syscall());
        } else if (callInst->name == "putint") {
            submitText(MIPSInst::LoadImm(Register::V0, 1));
            loadValue(function, callInst->args[0], Register::A0);
            submitText(MIPSInst::Syscall());
        } else if (callInst->name == "putch") {
            submitText(MIPSInst::LoadImm(Register::V0, 11));
            loadValue(function, callInst->args[0], Register::A0);
            submitText(MIPSInst::Syscall());
        } else if (callInst->name == "putstr") {
            submitText(MIPSInst::LoadImm(Register::V0, 4));
            loadValue(function, callInst->args[0], Register::A0);
            submitText(MIPSInst::Syscall());
        } else {
            for (int i = 0; i < callInst->args.size(); i++) {
                if (i < 4) {
                    loadValue(function, callInst->args[i], regParams[i]);
                } else {
                    loadValue(function, callInst->args[i], Register::T0);
                    submitText(MIPSInst::SaveWord(Register::T0, function->frame->getCallArgOffset(i), Register::SP));
                }
            }
            submitText(MIPSInst::JumpAndLink(callInst->name));
        }

        if (callInst->slot >= 0) {
            submitText(MIPSInst::SaveWord(Register::V0, function->frame->slotOffset[callInst->slot], Register::SP));
        }
    }

    void MIPSBuilder::translateLoadInst(Function *function, LoadInst *loadInst) {
        int offset = 0;
        Register reg = Register::SP;
        if (loadInst->ptr->type() == LLVMType::ALLOCA_INST) {
            offset = function->frame->slotOffset[loadInst->ptr->slot];
            reg = Register::SP;
        } else {
            loadValue(function, loadInst->ptr, Register::T1);
            offset = 0;
            reg = Register::T1;
        }
        if (loadInst->valueType->type() == ValueType::BASIC) {
            BasicValueType *basicType = static_cast<BasicValueType *>(loadInst->valueType);
            switch (basicType->basicType) {
                case BasicValueType::I8:
                    submitText(MIPSInst::LoadByte(Register::T0, offset, reg));
                    submitText(MIPSInst::SaveByte(Register::T0, function->frame->slotOffset[loadInst->slot], Register::SP));
                break;
                case BasicValueType::I32:
                    submitText(MIPSInst::LoadWord(Register::T0, offset, reg));
                    submitText(MIPSInst::SaveWord(Register::T0, function->frame->slotOffset[loadInst->slot], Register::SP));
                break;
                default:
                    break;
            }
        } else {
            submitText(MIPSInst::LoadWord(Register::T0, offset, reg));
            submitText(MIPSInst::SaveWord(Register::T0, function->frame->slotOffset[loadInst->slot], Register::SP));
        }
    }

    void MIPSBuilder::translateStoreInst(Function *function, StoreInst *storeInst) {
        if (storeInst->isArgInit) return;
        loadValue(function, storeInst->value, Register::T0);
        int offset = 0;
        Register reg = Register::SP;
        if (storeInst->ptr->type() == LLVMType::ALLOCA_INST) {
            offset = function->frame->slotOffset[storeInst->ptr->slot];
            reg = Register::SP;
        } else {
            loadValue(function, storeInst->ptr, Register::T1);
            offset = 0;
            reg = Register::T1;
        }
        BasicValueType *basicType = static_cast<BasicValueType *>(storeInst->value->valueType);
        switch (basicType->basicType) {
            case BasicValueType::I8:
                submitText(MIPSInst::SaveByte(Register::T0, offset, reg));
            break;
            case BasicValueType::I32:
                submitText(MIPSInst::SaveWord(Register::T0, offset, reg));
            break;
            default:
                break;
        }
    }

    void MIPSBuilder::translateGetElementPtrInst(Function *function, GetElementPtr *getElementPtr) {
        if (getElementPtr->ptr->type() == LLVMType::ALLOCA_INST) {
            int offset = function->frame->slotOffset[getElementPtr->ptr->slot];
            submitText(MIPSInst::AddImm(Register::T1, Register::SP, offset));
        } else {
            loadValue(function, getElementPtr->ptr, Register::T1);
        }
        int size = 4;
        PtrValueType *ptrType = static_cast<PtrValueType *>(getElementPtr->valueType);
        if (ptrType->value->type() == ValueType::BASIC) {
            BasicValueType *basicType = static_cast<BasicValueType *>(ptrType->value);
            if (basicType->basicType == BasicValueType::I8) {
                size = 1;
            }
        }
        if (getElementPtr->idx->type() == LLVMType::NUMERIC_LITERAL) {
            NumericLiteral *num = static_cast<NumericLiteral *>(getElementPtr->idx);
            submitText(MIPSInst::AddImm(Register::T0, Register::T1, num->value * size));
        } else {
            loadValue(function, getElementPtr->idx, Register::T2);
            if (size != 1) {
                submitText(MIPSInst::MulImm(Register::T2, Register::T2, size));
            }
            submitText(MIPSInst::Add(Register::T0, Register::T1, Register::T2));
        }
        submitText(MIPSInst::SaveWord(Register::T0, function->frame->slotOffset[getElementPtr->slot], Register::SP));
    }

    void MIPSBuilder::translateBranchInst(Function *function, BranchInst *branchInst) {
        if (branchInst->cond != nullptr) {
            loadValue(function, branchInst->cond, Register::T0);
            submitText(MIPSInst::BranchNE(Register::T0, Register::ZERO, function->name + "." + std::to_string(branchInst->ifTrue->blockIdx)));
            submitText(MIPSInst::Jump(function->name + "." + std::to_string(branchInst->ifFalse->blockIdx)));
        } else {
            submitText(MIPSInst::Jump(function->name + "." + std::to_string(branchInst->ifTrue->blockIdx)));
        }
    }

    void MIPSBuilder::translateRetInst(Function *function, RetInst *retInst) {
        submitText(MIPSInst::LoadWord(Register::RA, function->frame->frameSize - 4, Register::SP));
        if (function->name != "main") {
            if (retInst->value != nullptr) {
                loadValue(function, retInst->value, Register::V0);
            }
            submitText(MIPSInst::AddImm(Register::SP, Register::SP, function->frame->frameSize));
            submitText(MIPSInst::JumpReg(Register::RA));
        } else {
            submitText(MIPSInst::AddImm(Register::SP, Register::SP, function->frame->frameSize));
            submitText(MIPSInst::LoadImm(Register::V0, 10));
            submitText(MIPSInst::Syscall());
        }
    }

    void MIPSBuilder::translateZextInst(Function *function, ZextInst *zextInst) {
        loadValue(function, zextInst->v, Register::T0);
        submitText(MIPSInst::AndImm(Register::T0, Register::T0, 0xff));
        submitText(MIPSInst::SaveWord(Register::T0, function->frame->slotOffset[zextInst->slot], Register::SP));
    }

    void MIPSBuilder::translateTruncInst(Function *function, TruncInst *truncInst) {
        loadValue(function, truncInst->v, Register::T0);
        submitText(MIPSInst::AndImm(Register::T0, Register::T0, 0xff));
        submitText(MIPSInst::SaveWord(Register::T0, function->frame->slotOffset[truncInst->slot], Register::SP));
    }

    void MIPSBuilder::submitText(MIPSText *text) {
        text->print(os);
        texts.push_back(text);
    }

    void MIPSBuilder::loadSlot(Function *function, int slot, Register reg) {
        submitText(MIPSInst::LoadWord(reg, function->frame->slotOffset[slot], Register::SP));
    }

    void MIPSBuilder::loadValue(Function *function, Value *value, Register reg) {
        if (value->slot >= 0) {
            loadSlot(function, value->slot, reg);
        } else if (value->type() == LLVMType::NUMERIC_LITERAL) {
            NumericLiteral *num = static_cast<NumericLiteral *>(value);
            submitText(MIPSInst::LoadImm(reg, num->value));
        } else if (value->type() == LLVMType::STRING_LITERAL) {
            StringLiteral *str = static_cast<StringLiteral *>(value);
            std::string label = ".str";
            if (str->refId) {
                label += "." + std::to_string(str->refId);
            }
            submitText(MIPSInst::LoadAddr(reg, label));
        } else if (value->type() == LLVMType::GLOBAL_VARIABLE) {
            GlobalVariable *global = static_cast<GlobalVariable *>(value);
            submitText(MIPSInst::LoadAddr(reg, global->name));
        }
    }

    void MIPSBuilder::debugBreak() {
        static int cnt = 0;

        os << "ble $sp, 1000, force_ret." << cnt << std::endl;
        os << "j cont." << cnt << std::endl;
        os << "force_ret." << cnt << ":" << std::endl;
        os << "li $v0, 10" << std::endl;
        os << "syscall" << std::endl;
        os << "cont." << cnt << ":" << std::endl;
        cnt++;
    }

    void MIPSBuilder::debugTLE() {
        static int cnt = 0;

        os << "tle." << cnt << ":" << std::endl;
        os << "j " << "tle." << cnt << std::endl;
        cnt++;
    }

    void MIPSBuilder::alignCheck(Register reg, int offset) {
        static int cnt = 0;
        os << "move $s0, $" << static_cast<int>(reg) << std::endl;
        os << "add $s0, $s0, " << offset << std::endl;
        os << "rem $s0, $s0, 4" << std::endl;
        os << "bne $s0, $0, failed." << cnt << std::endl;
        os << "j suc." << cnt << std::endl;
        os << "failed." << cnt << ":" << std::endl;
        os << "j failed." << cnt << std::endl;
        os << "suc." << cnt << ":" << std::endl;
        cnt ++;
    }
} // thm