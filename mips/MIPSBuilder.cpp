//
// Created by slty5 on 24-11-7.
//

#include "MIPSBuilder.h"

#include <complex>
#include <functional>
#include <functional>
#include <functional>
#include <iostream>
#include <ostream>
#include <set>

#include "Frame.h"
#include "../util/util.h"

namespace thm {
    void MIPSBuilder::process() {
        os << ".data" << std::endl;
        for (GlobalVariable *global : module->variables) {
            addGlobalVariable(global);
        }
        for (StringLiteral *str : module->strings) {
            addStringLiteral(str);
        }
        os << ".text" << std::endl;
        processFunction(module->main);
        for (Function *function : module->functions) {
            processFunction(function);
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

    void MIPSBuilder::processFunction(Function *function) {
        int maxCallArgs = function->getMaxArgs();
        function->frame->init(maxCallArgs);
        submitText(new MIPSLabel(function->name));
        submitText(MIPSInst::AddImm(Register::SP, Register::SP, -function->frame->frameSize));
        for (int i = 0; i < function->args.size() && i < maxCallArgs; i++) {
            if (i < 4) {
                submitText(MIPSInst::SaveWord(regParams[i], function->frame->frameSize + 4 * i, Register::SP));
            }
        }
        for (int i = 4; i < function->args.size(); i++) {
            submitText(MIPSInst::LoadWord(function->args[i]->reg, function->frame->frameSize + 4 * i, Register::SP));
        }
        for (auto ent : function->frame->saved) {
            submitText(MIPSInst::SaveWord(ent.first, function->frame->getRegOffset(ent.first), Register::SP));
        }
        for (BasicBlock *block : function->blocks) {
            translateBlock(block);
        }
    }

    void MIPSBuilder::translateBlock(BasicBlock *block) {
        os << block->function->name << "." << block->blockIdx << ":" << std::endl;
        for (Instruction *inst : block->insts) {
            os << "#";
            inst->print(os);
            os << std::endl;
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
                // case LLVMType::STORE_INST:
                //     translateStoreInst(block->function, static_cast<StoreInst *>(inst));
                //     break;
                // case LLVMType::GET_ELEMENT_PTR_INST:
                //     translateGetElementPtrInst(block->function, static_cast<GetElementPtr *>(inst));
                //     break;
                // case LLVMType::BRANCH_INST:
                //     translateBranchInst(block->function, static_cast<BranchInst *>(inst));
                //     break;
                // case LLVMType::RET_INST:
                //     translateRetInst(block->function, static_cast<RetInst *>(inst));
                //     break;
                // case LLVMType::ZEXT_INST:
                //     translateZextInst(block->function, static_cast<ZextInst *>(inst));
                //     break;
                // case LLVMType::TRUNC_INST:
                //     translateTruncInst(block->function, static_cast<TruncInst *>(inst));
                //     break;
                // case LLVMType::MOVE:
                //     translateMoveInst(block->function, static_cast<MoveInst *>(inst));
                //     break;
                default:
                    break;
            }
        }
    }

    void MIPSBuilder::translateBinaryInst(Function *function, BinaryInst *binaryInst) {
        bool lConst = false;
        bool rConst = false;
        int lNum, rNum;
        if (binaryInst->l->type() == LLVMType::NUMERIC_LITERAL) {
            lNum = static_cast<NumericLiteral *>(binaryInst->l)->value;
            lConst = true;
        } else if (binaryInst->l->type() == LLVMType::UNDEF) {
            lNum = 1;
            lConst = true;
        }
        if (binaryInst->r->type() == LLVMType::NUMERIC_LITERAL) {
            rConst = true;
            rNum = static_cast<NumericLiteral *>(binaryInst->r)->value;
        } else if (binaryInst->r->type() == LLVMType::UNDEF) {
            rNum = 1;
            rConst = true;
        }
        switch (binaryInst->op) {
            case BinaryInst::ADD:
                if (lConst) {
                    if (rConst) {
                        submitText(MIPSInst::LoadImm(Register::V1, lNum));
                        submitText(MIPSInst::AddImm(binaryInst->reg, Register::V1, rNum));
                    } else {
                        submitText(MIPSInst::AddImm(binaryInst->reg, binaryInst->r->reg, lNum));
                    }
                } else {
                    if (rConst) {
                        submitText(MIPSInst::AddImm(binaryInst->reg, binaryInst->l->reg, rNum));
                    } else {
                        submitText(MIPSInst::Add(binaryInst->reg, binaryInst->l->reg, binaryInst->r->reg));
                    }
                }
                break;
            case BinaryInst::SUB:
                if (lConst) {
                    submitText(MIPSInst::LoadImm(Register::V1, lNum));
                    if (rConst) {
                        submitText(MIPSInst::AddImm(binaryInst->reg, Register::V1, -rNum));
                    } else {
                        submitText(MIPSInst::Sub(binaryInst->reg, Register::V1, binaryInst->r->reg));
                    }
                } else {
                    if (rConst) {
                        submitText(MIPSInst::AddImm(binaryInst->reg, binaryInst->l->reg, -rNum));
                    } else {
                        submitText(MIPSInst::Sub(binaryInst->reg, binaryInst->l->reg, binaryInst->r->reg));
                    }
                }
                break;
            case BinaryInst::MUL:
                if (lConst) {
                    if (rConst) {
                        submitText(MIPSInst::LoadImm(Register::V1, lNum));
                        submitText(MIPSInst::MulImm(binaryInst->reg, Register::V1, rNum));
                    } else {
                        submitText(MIPSInst::MulImm(binaryInst->reg, binaryInst->r->reg, lNum));
                    }
                } else {
                    if (rConst) {
                        submitText(MIPSInst::MulImm(binaryInst->reg, binaryInst->l->reg, rNum));
                    } else {
                        submitText(MIPSInst::Mul(binaryInst->reg, binaryInst->l->reg, binaryInst->r->reg));
                    }
                }
                break;
            case BinaryInst::SDIV:
                if (lConst) {
                    submitText(MIPSInst::LoadImm(Register::V1, lNum));
                    if (rConst) {
                        submitText(MIPSInst::DivImm(binaryInst->reg, Register::V1, rNum));
                    } else {
                        submitText(MIPSInst::Div(binaryInst->reg, Register::V1, binaryInst->r->reg));
                    }
                } else {
                    if (rConst) {
                        submitText(MIPSInst::DivImm(binaryInst->reg, binaryInst->l->reg, rNum));
                    } else {
                        submitText(MIPSInst::Div(binaryInst->reg, binaryInst->l->reg, binaryInst->r->reg));
                    }
                }
                break;
            case BinaryInst::SREM:
                if (lConst) {
                    submitText(MIPSInst::LoadImm(Register::V1, lNum));
                    if (rConst) {
                        submitText(MIPSInst::RemImm(binaryInst->reg, Register::V1, rNum));
                    } else {
                        submitText(MIPSInst::Rem(binaryInst->reg, Register::V1, binaryInst->r->reg));
                    }
                } else {
                    if (rConst) {
                        submitText(MIPSInst::RemImm(binaryInst->reg, binaryInst->l->reg, rNum));
                    } else {
                        submitText(MIPSInst::Rem(binaryInst->reg, binaryInst->l->reg, binaryInst->r->reg));
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
                    if (rConst) {
                        submitText(MIPSInst::LoadImm(Register::V1, lNum));
                        submitText(MIPSInst::EqImm(binaryInst->reg, Register::V1, rNum));
                    } else {
                        submitText(MIPSInst::EqImm(binaryInst->reg, binaryInst->r->reg, lNum));
                    }
                } else {
                    if (rConst) {
                        submitText(MIPSInst::EqImm(binaryInst->reg, binaryInst->l->reg, rNum));
                    } else {
                        submitText(MIPSInst::Eq(binaryInst->reg, binaryInst->l->reg, binaryInst->r->reg));
                    }
                }
                break;
            case BinaryInst::NE:
                if (lConst) {
                    submitText(MIPSInst::LoadImm(Register::V1, lNum));
                    if (rConst) {
                        submitText(MIPSInst::NeqImm(binaryInst->reg, Register::V1, rNum));
                    } else {
                        submitText(MIPSInst::Neq(binaryInst->reg, Register::V1, binaryInst->r->reg));
                    }
                } else {
                    if (rConst) {
                        submitText(MIPSInst::NeqImm(binaryInst->reg, binaryInst->l->reg, rNum));
                    } else {
                        submitText(MIPSInst::Neq(binaryInst->reg, binaryInst->l->reg, binaryInst->r->reg));
                    }
                }
                break;
            case BinaryInst::SGE:
                if (lConst) {
                    submitText(MIPSInst::LoadImm(Register::V1, lNum));
                    if (rConst) {
                        submitText(MIPSInst::SgeImm(binaryInst->reg, Register::V1, rNum));
                    } else {
                        submitText(MIPSInst::Sge(binaryInst->reg, Register::V1, binaryInst->r->reg));
                    }
                } else {
                    if (rConst) {
                        submitText(MIPSInst::SgeImm(binaryInst->reg, binaryInst->l->reg, rNum));
                    } else {
                        submitText(MIPSInst::Sge(binaryInst->reg, binaryInst->l->reg, binaryInst->r->reg));
                    }
                }
                break;
            case BinaryInst::SGT:
                if (lConst) {
                    submitText(MIPSInst::LoadImm(Register::V1, lNum));
                    if (rConst) {
                        submitText(MIPSInst::SgtImm(binaryInst->reg, Register::V1, rNum));
                    } else {
                        submitText(MIPSInst::Sgt(binaryInst->reg, Register::V1, binaryInst->r->reg));
                    }
                } else {
                    if (rConst) {
                        submitText(MIPSInst::SgtImm(binaryInst->reg, binaryInst->l->reg, rNum));
                    } else {
                        submitText(MIPSInst::Sgt(binaryInst->reg, binaryInst->l->reg, binaryInst->r->reg));
                    }
                }
                break;
            case BinaryInst::SLE:
                if (lConst) {
                    submitText(MIPSInst::LoadImm(Register::V1, lNum));
                    if (rConst) {
                        submitText(MIPSInst::SleImm(binaryInst->reg, Register::V1, rNum));
                    } else {
                        submitText(MIPSInst::Sle(binaryInst->reg, Register::V1, binaryInst->r->reg));
                    }
                } else {
                    if (rConst) {
                        submitText(MIPSInst::SleImm(binaryInst->reg, binaryInst->l->reg, rNum));
                    } else {
                        submitText(MIPSInst::Sle(binaryInst->reg, binaryInst->l->reg, binaryInst->r->reg));
                    }
                }
                break;
            case BinaryInst::SLT:
                if (lConst) {
                    submitText(MIPSInst::LoadImm(Register::V1, lNum));
                    if (rConst) {
                        submitText(MIPSInst::SleImm(binaryInst->reg, Register::V1, rNum));
                    } else {
                        submitText(MIPSInst::Slt(binaryInst->reg, Register::V1, binaryInst->r->reg));
                    }
                } else {
                    if (rConst) {
                        submitText(MIPSInst::SltImm(binaryInst->reg, binaryInst->l->reg, rNum));
                    } else {
                        submitText(MIPSInst::Slt(binaryInst->reg, binaryInst->l->reg, binaryInst->r->reg));
                    }
                }
                break;
        }
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
            loadValue(callInst->args[0], Register::A0);
            submitText(MIPSInst::Syscall());
        } else if (callInst->name == "putch") {
            submitText(MIPSInst::LoadImm(Register::V0, 11));
            loadValue(callInst->args[0], Register::A0);
            submitText(MIPSInst::Syscall());
        } else if (callInst->name == "putstr") {
            submitText(MIPSInst::LoadImm(Register::V0, 4));
            loadValue(callInst->args[0], Register::A0);
            submitText(MIPSInst::Syscall());
        } else {
            for (int i = 4; i < callInst->args.size(); i++) {
                Register reg = loadValue(callInst->args[i]);
                submitText(MIPSInst::SaveWord(reg, function->frame->getCallArgOffset(i), Register::SP));
            }
            std::vector<std::pair<Register, Register>> regMove;
            for (int i = 0; i < callInst->args.size() && i < 4; i++) {
                if (callInst->args[i]->reg == Register::NONE)
                    loadValue(callInst->args[i], regParams[i]);
                else
                    regMove.push_back({regParams[i], callInst->args[i]->reg});
            }
            sequenceMoves(regMove);
            submitText(MIPSInst::JumpAndLink(callInst->name));
        }

        if (callInst->slot >= 0) {
            move(callInst->reg, Register::V0);
        }

        for (int i = 0; i < function->args.size() && i < 4 && i < callInst->args.size(); i++) {
            submitText(MIPSInst::LoadWord(regParams[i], function->frame->frameSize + 4 * i, Register::SP));
        }
    }

    void MIPSBuilder::translateLoadInst(Function *function, LoadInst *loadInst) {
        int offset = 0;
        Register reg = Register::SP;
        if (AllocaInst *alloc = dynamic_cast<AllocaInst *>(loadInst->ptr)) {
            offset = function->frame->getOffset(alloc);
            reg = Register::SP;
        } else if (ArgumentAddress *arg = dynamic_cast<ArgumentAddress *>(loadInst->ptr)) {
            offset = function->frame->getArgAddress(arg);
            reg = Register::SP;
        } else if (loadInst->ptr->reg != Register::NONE) {
            offset = 0;
            reg = loadInst->ptr->reg;
        } else if (Constant *constant = dynamic_cast<Constant *>(loadInst->ptr)) {
            loadConstant(constant, Register::V1);
            offset = 0;
            reg = Register::V1;
        }
        if (loadInst->valueType->type() == ValueType::BASIC) {
            BasicValueType *basicType = static_cast<BasicValueType *>(loadInst->valueType);
            switch (basicType->basicType) {
                case BasicValueType::I8:
                    submitText(MIPSInst::LoadByte(loadInst->reg, offset, reg));
                break;
                case BasicValueType::I32:
                    submitText(MIPSInst::LoadWord(loadInst->reg, offset, reg));
                break;
                default:
                    break;
            }
        } else {
            submitText(MIPSInst::LoadWord(loadInst->reg, offset, reg));
        }
    }

    void MIPSBuilder::translateStoreInst(Function *function, StoreInst *storeInst) {
        int offset = 0;
        bool useLabel = false;
        Register reg = Register::SP;
        std::string label;
        if (AllocaInst *alloc = dynamic_cast<AllocaInst *>(storeInst->ptr)) {
            offset = function->frame->getOffset(alloc);
            reg = Register::SP;
        } else if (ArgumentAddress *arg = dynamic_cast<ArgumentAddress *>(storeInst->ptr)) {
            offset = function->frame->getArgAddress(arg);
            reg = Register::SP;
        } else if (storeInst->ptr->reg != Register::NONE) {
            offset = 0;
            reg = storeInst->ptr->reg;
        } else if (GlobalVariable *global = dynamic_cast<GlobalVariable *>(storeInst->ptr)) {
            useLabel = true;
            label = global->name;
        }
        if (BasicValueType *basicType = dynamic_cast<BasicValueType *>(storeInst->value->valueType)) {
            switch (basicType->basicType) {
                case BasicValueType::I8:
                    if (useLabel) {
                        submitText(MIPSInst::SaveByte(loadValue(storeInst->value), Register::ZERO, label));
                    } else {
                        submitText(MIPSInst::SaveByte(loadValue(storeInst->value), offset, reg));
                    }
                break;
                case BasicValueType::I32:
                    if (useLabel) {
                        submitText(MIPSInst::SaveWord(loadValue(storeInst->value), Register::ZERO, label));
                    } else {
                        submitText(MIPSInst::SaveWord(loadValue(storeInst->value), offset, reg));
                    }
                break;
                default:
                    break;
            }
        } else if (PtrValueType* ptrType = dynamic_cast<PtrValueType *>(storeInst->value->valueType)) {
            if (useLabel) {
                submitText(MIPSInst::SaveWord(loadValue(storeInst->value), Register::ZERO, label));
            } else {
                submitText(MIPSInst::SaveWord(loadValue(storeInst->value), offset, reg));
            }
        }
    }

    void MIPSBuilder::translateGetElementPtrInst(Function *function, GetElementPtr *getElementPtr) {
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
            Register baseReg = getElementPtr->reg;
            if (AllocaInst *alloc = dynamic_cast<AllocaInst *>(getElementPtr->ptr)) {
                submitText(MIPSInst::AddImm(getElementPtr->reg, Register::SP, function->frame->getOffset(alloc)));
            } else if (getElementPtr->ptr->reg != Register::NONE) {
                baseReg = getElementPtr->ptr->reg;
            } else {
                loadConstant(getElementPtr->ptr, Register::V1);
                baseReg = Register::V1;
            }
            submitText(MIPSInst::AddImm(getElementPtr->reg, baseReg, num->value * size));
        } else {
            Register baseReg = getElementPtr->reg;
            Register idxTmp = getElementPtr->reg;
            if (AllocaInst *alloc = dynamic_cast<AllocaInst *>(getElementPtr->ptr)) {
                submitText(MIPSInst::AddImm(getElementPtr->reg, Register::SP, function->frame->getOffset(alloc)));
                idxTmp = Register::V1;
            } else if (getElementPtr->ptr->reg != Register::NONE) {
                baseReg = getElementPtr->ptr->reg;
                idxTmp = Register::V1;
            } else {
                loadConstant(getElementPtr->ptr, Register::V1);
                baseReg = Register::V1;
                idxTmp = getElementPtr->reg;
            }
            if (size != 1) {
                submitText(MIPSInst::MulImm(idxTmp, getElementPtr->idx->reg, size));
            }
            submitText(MIPSInst::Add(getElementPtr->reg, baseReg, idxTmp));
        }
    }

    void MIPSBuilder::translateBranchInst(Function *function, BranchInst *branchInst) {
        if (branchInst->cond != nullptr) {
            Register condReg = branchInst->cond->reg;
            if (Constant *constant = dynamic_cast<Constant *>(branchInst->cond)) {
                loadConstant(constant, Register::V1);
                condReg = Register::V1;
            }
            submitText(MIPSInst::BranchNE(condReg, Register::ZERO, function->name + "." + std::to_string(branchInst->ifTrue->blockIdx)));
            submitText(MIPSInst::Jump(function->name + "." + std::to_string(branchInst->ifFalse->blockIdx)));
        } else {
            submitText(MIPSInst::Jump(function->name + "." + std::to_string(branchInst->ifTrue->blockIdx)));
        }
    }

    void MIPSBuilder::translateRetInst(Function *function, RetInst *retInst) {

        if (function->name != "main") {
            if (retInst->value != nullptr) {
                Register reg = retInst->value->reg;
                if (Constant *constant = dynamic_cast<Constant *>(retInst->value)) {
                    reg = Register::V1;
                    loadConstant(constant, Register::V1);
                }
                move(Register::V0, reg);
            }
            for (auto ent : function->frame->saved) {
                submitText(MIPSInst::LoadWord(ent.first, function->frame->getRegOffset(ent.first), Register::SP));
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
        Register reg = loadValue(zextInst->v);
        submitText(MIPSInst::AndImm(zextInst->reg, reg, 0xff));
    }

    void MIPSBuilder::translateTruncInst(Function *function, TruncInst *truncInst) {
        Register reg = loadValue(truncInst->v);
        submitText(MIPSInst::AndImm(truncInst->reg, reg, 0xff));
    }

    void MIPSBuilder::translateMoveInst(Function *function, MoveInst *moveInst) {
        if (Constant *constant = dynamic_cast<Constant *>(moveInst->src)) {
            loadConstant(constant, moveInst->dst->reg);
        } else {
            move(moveInst->dst->reg, moveInst->src->reg);
        }
    }

    void MIPSBuilder::submitText(MIPSText *text) {
        text->print(os);
        texts.push_back(text);
    }

    Register MIPSBuilder::loadValue(Value *value) {
        if (Constant *constant = dynamic_cast<Constant *>(value)) {
            loadConstant(constant, Register::V1);
            return Register::V1;
        }
        return value->reg;
    }

    void MIPSBuilder::loadValue(Value *value, Register reg) {
        if (Constant *constant = dynamic_cast<Constant *>(value)) {
            loadConstant(constant, reg);
        } else {
            move(reg, value->reg);
        }
    }

    void MIPSBuilder::loadConstant(Value *value, Register reg) {
        if (NumericLiteral *num = dynamic_cast<NumericLiteral *>(value)) {
            submitText(MIPSInst::LoadImm(reg, num->value));
        }
        if (StringLiteral *str = dynamic_cast<StringLiteral *>(value)) {
            std::string label = ".str";
            if (str->refId) {
                label += "." + std::to_string(str->refId);
            }
            submitText(MIPSInst::LoadAddr(reg, label));
        }
        if (GlobalVariable *global = dynamic_cast<GlobalVariable *>(value)) {
            submitText(MIPSInst::LoadAddr(reg, global->name));
        }
    }

    void MIPSBuilder::move(Register dst, Register src) {
        if (dst != src) {
            submitText(MIPSInst::Move(dst, src));
        }
    }

    void MIPSBuilder::sequenceMoves(std::vector<std::pair<Register, Register>> &moves) {
        while (true) {
            bool allTheSame = true;
            std::unordered_set<Register> used;
            for (auto mv : moves) {
                if (mv.first != mv.second) {
                    allTheSame = false;
                }
                used.insert(mv.second);
            }
            if (allTheSame) break;
            bool directRemove = false;
            std::vector<std::pair<Register, Register>>::iterator cp;
            for (auto iter = moves.begin(); iter != moves.end(); ++iter) {
                cp = iter;
                if (used.find(iter->first) == used.end()) {
                    directRemove = true;
                    move(iter->first, iter->second);
                    moves.erase(iter);
                    break;
                }
            }
            if (directRemove) continue;
            move(Register::V1, cp->second);
            cp->second = Register::V1;
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