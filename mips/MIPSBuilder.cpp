//
// Created by slty5 on 24-11-7.
//

#include "MIPSBuilder.h"

#include <functional>
#include <ostream>

#include "../util/util.h"

namespace thm {
    void MIPSBuilder::build() {
        os << ".data" << std::endl;
        for (GlobalVariable *global : module->variables) {
            addGlobalVariable(global);
        }
        for (StringLiteral *str : module->strings) {
            addStringLiteral(str);
        }
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
        os << ".str";
        if (str->refId) {
            os << "." << str->refId;
        }
        os << ": ascii \"";
        for (int i = 0; i < str->value.length(); i++) {
            os << unescape(str->value[i]);
        }
        os << "\"" << std::endl;
    }

    void MIPSBuilder::translateFunction(Function *function) {
        for (BasicBlock *block : function->blocks) {
            translateBlock(block);
        }
    }

    void MIPSBuilder::translateBlock(BasicBlock *block) {
        os << block->function->name << "." << block->blockIdx << ":" << std::endl;
        for (Instruction *inst : block->insts) {
            switch (inst->type()) {
                case LLVMType::BINARY_INST:
                    translateBinaryInst(static_cast<BinaryInst *>(inst));
                    break;
                case LLVMType::CALL_INST:
                    break;
                case LLVMType::ALLOCA_INST:
                    break;
                case LLVMType::LOAD_INST:
                    break;
                case LLVMType::STORE_INST:
                    break;
                case LLVMType::GET_ELEMENT_PTR_INST:
                    break;
                case LLVMType::BRANCH_INST:
                    break;
                case LLVMType::RET_INST:
                    break;
                case LLVMType::ZEXT_INST:
                    break;
                case LLVMType::TRUNC_INST:
                    break;
                default:
                    break;
            }
        }
    }

    void MIPSBuilder::translateBinaryInst(BinaryInst *binaryInst) {
        bool lConst = true;
        bool rConst = true;
        if (binaryInst->l->type() != LLVMType::NUMERIC_LITERAL) {
            loadSlot(binaryInst->l->slot, Register::T0);
            lConst = false;
        }
        if (binaryInst->r->type() != LLVMType::NUMERIC_LITERAL) {
            loadSlot(binaryInst->r->slot, Register::T1);
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

                break;
            case BinaryInst::OR:
                break;
            case BinaryInst::EQ:
                break;
            case BinaryInst::NE:
                break;
            case BinaryInst::SGE:
                break;
            case BinaryInst::SGT:
                break;
            case BinaryInst::SLE:
                break;
            case BinaryInst::SLT:
                break;
        }
    }

    void MIPSBuilder::submitText(MIPSText *text) {
        text->print(os);
        texts.push_back(text);
    }

    void MIPSBuilder::loadSlot(int slot, Register reg) {
        submitText(MIPSInst::LoadWord(reg, stackTracker.offset[slot], Register::FP));
    }
} // thm