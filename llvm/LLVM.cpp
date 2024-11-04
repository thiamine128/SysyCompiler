//
// Created by slty5 on 24-10-27.
//

#include "LLVM.h"

#include <complex>
#include <functional>
#include <iostream>
#include <ostream>
#include <bits/locale_facets_nonio.h>

namespace thm {
    ValueType::Type ValueType::type() const {
        return BASIC;
    }

    void ValueType::print(std::ostream &os) const {

    }

    ValueType* ValueType::clone() const {
        return new ValueType();
    }

    ValueType * ValueType::fromVariableType(VariableType &variableType) {
        ValueType* t = new BasicValueType(variableType.type == VariableType::INT ? BasicValueType::I32 : BasicValueType::I8);
        if (variableType.isArray) {
            t = new ArrayValueType(t, variableType.arrayLen);
        }
        return t;
    }

    ValueType::Type BasicValueType::type() const {
        return BASIC;
    }

    void BasicValueType::print(std::ostream &os) const {
        switch (basicType) {
            case I32: os << "i32"; break;
            case I8: os << "i8"; break;
            case VOID: os << "void"; break;
            case I1: os << "i1"; break;
            case LABEL: os << "label"; break;
        }
    }

    ValueType * BasicValueType::clone() const {
        return new BasicValueType(basicType);
    }

    BasicValueType::BasicType BasicValueType::fromFunctionType(FunctionSymbol::Type functionType) {
        switch (functionType) {
            case FunctionSymbol::INT:
                return BasicValueType::I32;
            break;
            case FunctionSymbol::CHAR:
                return BasicValueType::I8;
            break;
            case FunctionSymbol::VOID:
                return BasicValueType::VOID;
            break;
        }
        return VOID;
    }

    ValueType::Type PtrValueType::type() const {
        return PTR;
    }

    void PtrValueType::print(std::ostream &os) const {
        value->print(os);
        os << "*";
    }

    ValueType * PtrValueType::clone() const {
        return new PtrValueType(value->clone());
    }

    ValueType::Type ArrayValueType::type() const {
        return ARRAY;
    }

    void ArrayValueType::print(std::ostream &os) const {
        os << "[" << arrayLen << " x ";
        value->print(os);
        os << "]";
    }

    ValueType * ArrayValueType::clone() const {
        return new ArrayValueType(value->clone(), arrayLen);
    }

    LLVMType Value::type() const {
        return LLVMType::DEFAULT;
    }

    void Value::print(std::ostream &os) const {

    }

    void Value::printRef(std::ostream &os) const {
        os << "%" << slot;
    }

    Argument::Argument() {
        slot = 0;
    }

    LLVMType Argument::type() const {
        return LLVMType::ARGUMENT;
    }

    void Argument::print(std::ostream &os) const {
        valueType->print(os);
        os << " %" << slot;
    }

    BasicBlock::BasicBlock(Function *function) : function(function) {
        valueType = new BasicValueType(BasicValueType::LABEL);
    }

    LLVMType BasicBlock::type() const {
        return LLVMType::BASIC_BLOCK;
    }

    void BasicBlock::print(std::ostream &os) const {
        os << slot << ":" << std::endl;
        for (Instruction* instruction : insts) {
            os << "\t";
            instruction->print(os);
        }
    }

    void BasicBlock::printRef(std::ostream &os) const {
        os << "%" << slot;
    }

    void BasicBlock::addInst(Instruction *instruction) {
        insts.push_back(instruction);
    }

    void BasicBlock::fillSlot() {
        for (Instruction *inst : insts) {
            if (inst->slot == 0) {
                inst->slot = function->slotTracker.allocSlot();
            }
        }
    }

    LLVMType Constant::type() const {
        return LLVMType::CONSTANT;
    }

    NumericLiteral::NumericLiteral(int value, BasicValueType::BasicType basic) : value(value) {
        valueType = new BasicValueType(basic);
    }

    LLVMType NumericLiteral::type() const {
        return LLVMType::NUMERIC_LITERAL;
    }

    void NumericLiteral::print(std::ostream &os) const {
        os << value;
    }

    void NumericLiteral::printRef(std::ostream &os) const {
        os << value;
    }

    StringLiteral::StringLiteral(const std::string &value) : value(value) {
        valueType = new PtrValueType(new ArrayValueType(new BasicValueType(BasicValueType::I8), value.length() + 1));
    }

    LLVMType StringLiteral::type() const {
        return LLVMType::STRING_LITERAL;
    }

    void StringLiteral::print(std::ostream &os) const {
        os << "@.str";
        if (refId) {
            os << "." << refId;
        }
        os << " = private unnamed_addr constant ";
        PtrValueType *ptr = static_cast<PtrValueType *>(valueType);
        ptr->value->print(os);
        os << " c\"";
        for (int i = 0; i < value.length(); i++) {
            int f = value[i] / 16;
            int s = value[i] % 16;
            os << "\\";
            if (f >= 10) {
                os << static_cast<char>('A' + f - 10);
            } else {
                os << f;
            }
            if (s >= 10) {
                os << static_cast<char>('A' + s - 10);
            } else {
                os << s;
            }
        }
        os << "\\00\", align 1" << std::endl;
    }

    void StringLiteral::printRef(std::ostream &os) const {
        os << "@.str";
        if (refId) {
            os << "." << refId;
        }
    }

    LLVMType GlobalValue::type() const {
        return LLVMType::GLOBAL_VALUE;
    }

    void GlobalValue::printRef(std::ostream &os) const {
        os << "@" << name;
    }

    Function::Function(std::string const &name, BasicValueType::BasicType retType) {
        this->name = name;
        valueType = new BasicValueType(retType);
    }

    LLVMType Function::type() const {
        return LLVMType::FUNCTION;
    }

    void Function::print(std::ostream &os) const {
        os << "define dso_local ";
        valueType->print(os);
        os << " @" << name << "(";
        for (int i = 0; i < args.size(); i++) {
            args[i]->print(os);
            if (i + 1 < args.size()) os << ", ";
        }
        os << ") {" << std::endl;
        for (BasicBlock* block : blocks) {
            block->print(os);
        }
        os << "}" << std::endl;
    }

    void Function::fillSlot() {
        for (Argument* arg : args) {
            arg->slot = slotTracker.allocSlot();
        }
        for (BasicBlock* block : blocks) {
            block->slot = slotTracker.allocSlot();
            block->fillSlot();
        }
    }

    LLVMType GlobalVariable::type() const {
        return LLVMType::GLOBAL_VARIABLE;
    }

    void GlobalVariable::print(std::ostream &os) const {
        PtrValueType* ptr = static_cast<PtrValueType*>(valueType);
        os << "@" << name << " = " << "dso_local" << " " << "global ";
        ptr->value->print(os);
        os << " ";
        if (ptr->value->type() == ValueType::ARRAY) {
            ArrayValueType* arrayType = static_cast<ArrayValueType*>(ptr->value);
            if (zeroInit) {
                os << "zeroinitializer";
            } else {
                os << "[";
                for (int i = 0; i < arrayType->arrayLen; i++) {
                    arrayType->value->print(os);
                    os << " " << initVals[i];
                    if (i < arrayType->arrayLen - 1) {
                        os << ", ";
                    }
                }
                os << "]";
            }
            if (arrayType->value->type() == ValueType::BASIC) {
                BasicValueType* basicType = static_cast<BasicValueType*>(arrayType->value);
                if (basicType->basicType == BasicValueType::I8) {
                    os << ", align 1";
                }
            }
        } else {
            os << initVal;
        }
        os << std::endl;
    }

    LLVMType User::type() const {
        return LLVMType::USER;
    }

    LLVMType Instruction::type() const {
        return LLVMType::INST;
    }

    BinaryInst::BinaryInst(Op op, Value *l, Value *r) : op(op), l(l), r(r) {
        switch (op) {
            case EQ:
            case NE:
            case SLE:
            case SLT:
            case SGE:
            case SGT:
                valueType = new BasicValueType(BasicValueType::I1);
            break;
            default:
                valueType = l->valueType->clone();
            break;
        }
        slot = 0;
    }

    LLVMType BinaryInst::type() const {
        return LLVMType::BINARY_INST;
    }

    void BinaryInst::print(std::ostream &os) const {
        os << "%" << slot << " = ";
        switch (op) {
            case ADD:
                os << "add";
                break;
            case SUB:
                os << "sub";
                break;
            case MUL:
                os << "mul";
                break;
            case SDIV:
                os << "sdiv";
                break;
            case SREM:
                os << "srem";
                break;
            case AND:
                os << "and";
                break;
            case OR:
                os << "or";
                break;
            case EQ:
                os << "icmp eq";
                break;
            case NE:
                os << "icmp ne";
                break;
            case SGE:
                os << "icmp sge";
                break;
            case SGT:
                os << "icmp sgt";
                break;
            case SLE:
                os << "icmp sle";
                break;
            case SLT:
                os << "icmp slt";
                break;
        }
        os << " ";
        l->valueType->print(os);
        os << " ";
        l->printRef(os);
        os << ", ";
        r->printRef(os);
        os << std::endl;
    }

    CallInst::CallInst(bool requireSlot, Function* function, std::vector<Value *> const &args) : name(function->name), args(args) {
        valueType = function->valueType->clone();
        if (requireSlot) slot = 0;
    }

    LLVMType CallInst::type() const {
        return LLVMType::CALL_INST;
    }

    void CallInst::print(std::ostream &os) const {
        if (slot != -1) {
            os << "%" << slot << " = ";
        }
        os << "call ";
        valueType->print(os);
        os << " @" << name;
        os << "(";
        for (int i = 0; i < args.size(); i++) {
            args[i]->valueType->print(os);
            os << " ";
            args[i]->printRef(os);
            if (i + 1 < args.size()) {
                os << ", ";
            }
        }
        os << ")" << std::endl;
    }

    AllocaInst::AllocaInst(VariableType& variableType) {
        slot = 0;
        allocType = ValueType::fromVariableType(variableType);
        valueType = allocType->clone();
        valueType = new PtrValueType(valueType);
    }

    AllocaInst::AllocaInst(ValueType* allocType) : allocType(allocType->clone()) {
        slot = 0;
        valueType = new PtrValueType(allocType->clone());
    }

    LLVMType AllocaInst::type() const {
        return LLVMType::ALLOCA_INST;
    }

    void AllocaInst::print(std::ostream &os) const {
        os << "%" << slot << " = alloca ";
        allocType->print(os);
        BasicValueType* basicType = nullptr;
        if (allocType->type() == ValueType::BASIC) {
            basicType = static_cast<BasicValueType*>(allocType);
        } else if (allocType->type() == ValueType::ARRAY) {
            ArrayValueType* arrayType = static_cast<ArrayValueType*>(allocType);
            if (arrayType->value->type() == ValueType::BASIC) {
                basicType = static_cast<BasicValueType*>(arrayType->value);
            }
        }
        os << std::endl;
    }

    LoadInst::LoadInst(Value *ptr) : ptr(ptr) {
        slot = 0;
        PtrValueType* v = static_cast<PtrValueType*>(ptr->valueType);
        valueType = v->value->clone();
    }

    LLVMType LoadInst::type() const {
        return LLVMType::LOAD_INST;
    }

    void LoadInst::print(std::ostream &os) const {
        os << "%" << slot << " = load ";
        valueType->print(os);
        os << ", ";
        ptr->valueType->print(os);
        os << " ";
        ptr->printRef(os);
        os << std::endl;
    }

    StoreInst::StoreInst(Value *value, Value *ptr) : value(value), ptr(ptr) {
        valueType = new BasicValueType(BasicValueType::VOID);
    }

    LLVMType StoreInst::type() const {
        return LLVMType::STORE_INST;
    }

    void StoreInst::print(std::ostream &os) const {
        os << "store ";
        value->valueType->print(os);
        os << " ";
        value->printRef(os);
        os << ", ";
        ptr->valueType->print(os);
        os << " ";
        ptr->printRef(os);
        os << std::endl;
    }

    GetElementPtr::GetElementPtr(Value *ptr, Value* idx) : ptr(ptr), idx(idx) {
        slot = 0;
        ValueType* inner = static_cast<PtrValueType*>(ptr->valueType)->value;
        if (inner->type() == ValueType::ARRAY) {
            valueType = new PtrValueType(static_cast<ArrayValueType*>(inner)->value->clone());
        } else {
            valueType = ptr->valueType->clone();
        }
    }

    LLVMType GetElementPtr::type() const {
        return LLVMType::GET_ELEMENT_PTR_INST;
    }

    void GetElementPtr::print(std::ostream &os) const {
        os << "%" << slot << " = getelementptr ";
        ValueType* inner = static_cast<PtrValueType*>(ptr->valueType)->value;
        inner->print(os);
        os << ", ";
        ptr->valueType->print(os);
        os << " ";
        ptr->printRef(os);
        os << ", ";
        if (inner->type() == ValueType::ARRAY) {
            os << "i32 0, ";
            idx->valueType->print(os);
            os << " ";
            idx->print(os);
        } else {
            idx->valueType->print(os);
            os << " ";
            idx->print(os);
        }
        os << std::endl;
    }

    ZextInst::ZextInst(Value *v) : v(v) {
        slot = 0;
        valueType = new BasicValueType(BasicValueType::I32);
    }

    LLVMType ZextInst::type() const {
        return LLVMType::ZEXT_INST;
    }

    void ZextInst::print(std::ostream &os) const {
        os << "%" << slot << " = zext ";
        v->valueType->print(os);
        os << " ";
        v->printRef(os);
        os << " to ";
        valueType->print(os);
        os << std::endl;
    }

    TruncInst::TruncInst(Value *v) : v(v) {
        slot = 0;
        valueType = new BasicValueType(BasicValueType::I8);
    }

    LLVMType TruncInst::type() const {
        return LLVMType::TRUNC_INST;
    }

    void TruncInst::print(std::ostream &os) const {
        os << "%" << slot << " = trunc ";
        v->valueType->print(os);
        os << " ";
        v->printRef(os);
        os << " to ";
        valueType->print(os);
        os << std::endl;
    }

    RetInst::RetInst(Value *value) : value(value) {
    }

    LLVMType RetInst::type() const {
        return LLVMType::RET_INST;
    }

    void RetInst::print(std::ostream &os) const {
        os << "ret ";
        if (value != nullptr) {
            value->valueType->print(os);
            os << " ";
            value->printRef(os);
        } else {
            os << "void";
        }
        os << std::endl;
    }

    BranchInst::BranchInst(BasicBlock *uncond) : cond(nullptr), ifTrue(uncond), ifFalse(nullptr) {
    }

    BranchInst::BranchInst(Value *cond, BasicBlock *ifTrue, BasicBlock *ifFalse) : cond(cond), ifTrue(ifTrue), ifFalse(ifFalse) {

    }

    LLVMType BranchInst::type() const {
        return LLVMType::BRANCH_INST;
    }

    void BranchInst::print(std::ostream &os) const {
        os << "br ";
        if (cond != nullptr) {
            cond->valueType->print(os);
            os << " ";
            cond->printRef(os);
            os << ", ";
            ifTrue->valueType->print(os);
            os << " ";
            ifTrue->printRef(os);
            os << ", ";
            ifFalse->valueType->print(os);
            os << " ";
            ifFalse->printRef(os);
        } else {
            ifTrue->valueType->print(os);
            os << " ";
            ifTrue->printRef(os);
        }
        os << std::endl;
    }

    void BranchInst::printRef(std::ostream &os) const {
        Instruction::printRef(os);
    }

    Use::Use(User *user, Value *value) : user(user), value(value) {

    }

    Module::Module() {
        getInt = new Function("getint", BasicValueType::I32);
        getChar = new Function("getchar", BasicValueType::I8);
        putInt = new Function("putint", BasicValueType::VOID);
        Argument* putIntArg = new Argument();
        putIntArg->valueType = new BasicValueType(BasicValueType::I32);
        putInt->args.push_back(putIntArg);
        putChar = new Function("putch", BasicValueType::VOID);
        Argument* putCharArg = new Argument();
        putCharArg->valueType = new BasicValueType(BasicValueType::I8);
        putChar->args.push_back(putCharArg);
        putStr = new Function("putstr", BasicValueType::VOID);
        Argument* putStrArg = new Argument();
        putStrArg->valueType = new PtrValueType(new BasicValueType(BasicValueType::I8));
        putStr->args.push_back(putStrArg);
    }

    void Module::print(std::ostream &os) const {
        os << "declare i32 @getint()" << std::endl
            << "declare i32 @getchar()" << std::endl
            << "declare void @putint(i32)"<< std::endl
            << "declare void @putch(i8)" << std::endl
            << "declare void @putstr(i8*)" << std::endl;

        os << std::endl;
        for (auto var : variables) {
            var->print(os);
        }
        os << std::endl;
        for (auto stringLiteral : strings) {
            stringLiteral->print(os);
        }
        os << std::endl;
        for (auto func : functions) {
            func->print(os);
            os << std::endl;
        }

        main->print(os);
    }

    StringLiteral * Module::addStringLiteral(std::string const &string) {
        StringLiteral* stringLiteral = new StringLiteral(string);
        stringLiteral->refId = strings.size();
        strings.push_back(stringLiteral);
        return stringLiteral;
    }

    void Module::fillSlot() {
        for (Function* function : functions) {
            function->fillSlot();
        }
        main->fillSlot();
    }
} // thm
