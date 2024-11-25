//
// Created by slty5 on 24-11-25.
//

#include "GVN.h"

namespace thm {
    void GVN::process() {
        for (auto func : module->functions) {
            processFunction(func);
        }
        processFunction(module->main);
    }

    void GVN::processFunction(Function *func) {
        std::vector<BasicBlock *> rpo = func->calcRPO();
        bool changed = true;
        while (changed) {
            changed = false;

            for (auto bb : rpo) {
                for (auto it = bb->insts.begin(); it != bb->insts.end(); ) {
                    // try fold
                    if (Value *folded = tryFold(*it)) {
                        changed = true;
                        for (auto bb1 : rpo) {
                            for (auto it1 : bb1->insts) {
                                (*it1).replaceUse(*it, folded);
                            }
                        }
                        it = bb->insts.erase(it);
                    } else {
                        ++it;
                    }
                }
            }
        }
    }

    Value * GVN::tryFold(Value *val) {
        if (BinaryInst *bin = dynamic_cast<BinaryInst *>(val)) {
            if (bin->l->type() == LLVMType::NUMERIC_LITERAL && bin->r->type() == LLVMType::NUMERIC_LITERAL) {
                NumericLiteral *lNum = static_cast<NumericLiteral *>(bin->l);
                NumericLiteral *rNum = static_cast<NumericLiteral *>(bin->r);
                switch (bin->op) {
                    case BinaryInst::ADD:
                        return new NumericLiteral(lNum->value + rNum->value, BasicValueType::I32);
                        break;
                    case BinaryInst::SUB:
                        return new NumericLiteral(lNum->value - rNum->value, BasicValueType::I32);
                        break;
                    case BinaryInst::MUL:
                        return new NumericLiteral(lNum->value * rNum->value, BasicValueType::I32);
                        break;
                    case BinaryInst::SDIV:
                        return new NumericLiteral(lNum->value / rNum->value, BasicValueType::I32);
                        break;
                    case BinaryInst::SREM:
                        return new NumericLiteral(lNum->value % rNum->value, BasicValueType::I32);
                        break;
                    case BinaryInst::AND:
                        return new NumericLiteral(lNum->value & rNum->value, BasicValueType::I32);
                        break;
                    case BinaryInst::OR:
                        return new NumericLiteral(lNum->value | rNum->value, BasicValueType::I32);
                        break;
                    case BinaryInst::EQ:
                        return new NumericLiteral(lNum->value == rNum->value, BasicValueType::I32);
                        break;
                    case BinaryInst::NE:
                        return new NumericLiteral(lNum->value != rNum->value, BasicValueType::I32);
                        break;
                    case BinaryInst::SGE:
                        return new NumericLiteral(lNum->value >= rNum->value, BasicValueType::I32);
                        break;
                    case BinaryInst::SGT:
                        return new NumericLiteral(lNum->value > rNum->value, BasicValueType::I32);
                        break;
                    case BinaryInst::SLE:
                        return new NumericLiteral(lNum->value <= rNum->value, BasicValueType::I32);
                        break;
                    case BinaryInst::SLT:
                        return new NumericLiteral(lNum->value < rNum->value, BasicValueType::I32);
                        break;
                }
            }
        }
        if (TruncInst *trunc = dynamic_cast<TruncInst *>(val)) {
            if (NumericLiteral *num = dynamic_cast<NumericLiteral *>(trunc->v)) {
                return new NumericLiteral(static_cast<unsigned int>(num->value) & 0xff, BasicValueType::I8);
            }
        }
        if (ZextInst *zext = dynamic_cast<ZextInst *>(val)) {
            if (NumericLiteral *num = dynamic_cast<NumericLiteral *>(zext->v)) {
                return new NumericLiteral(static_cast<unsigned int>(num->value) & 0xff, BasicValueType::I32);
            }
        }
        return nullptr;
    }
} // thm