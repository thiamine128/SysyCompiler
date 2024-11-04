//
// Created by slty5 on 24-10-27.
//

#ifndef LLVM_H
#define LLVM_H
#include <string>
#include <vector>

#include "SlotTracker.h"
#include "../semantic/Symbol.h"
namespace thm {
    class Instruction;
    class Function;
    class Use;

enum class LLVMType {
    DEFAULT,
    ARGUMENT,
    BASIC_BLOCK,
    CONSTANT,
    NUMERIC_LITERAL,
    STRING_LITERAL,
    GLOBAL_VALUE,
    FUNCTION,
    GLOBAL_VARIABLE,
    USER,
    INST,
    BINARY_INST,
    CALL_INST,
    ALLOCA_INST,
    LOAD_INST,
    STORE_INST,
    GET_ELEMENT_PTR_INST,
    BRANCH_INST,
    RET_INST,
    ZEXT_INST,
    TRUNC_INST,
};

class ValueType {
public:
    enum Type {
        BASIC,
        PTR,
        ARRAY,
    };
    virtual ~ValueType() = default;

    virtual Type type() const;
    virtual void print(std::ostream& os) const;
    virtual ValueType* clone() const;

    static ValueType* fromVariableType(VariableType& variableType);
};
class BasicValueType final : public ValueType {
public:
    enum BasicType {
        I32,
        I8,
        I1,
        VOID,
        LABEL
    } basicType;

    BasicValueType(BasicType t) : basicType(t) {}
    Type type() const override;
    void print(std::ostream& os) const override;
    ValueType* clone() const override;
    static BasicType fromFunctionType(FunctionSymbol::Type functionType);
};
class PtrValueType final : public ValueType {
public:
    ValueType* value = nullptr;

    PtrValueType(ValueType* value) : value(value) {}
    Type type() const override;
    void print(std::ostream& os) const override;
    ValueType *clone() const override;
};
class ArrayValueType final : public ValueType {
public:
    ValueType* value = nullptr;
    int arrayLen = 0;

    ArrayValueType(ValueType* value, int arrayLen) : value(value), arrayLen(arrayLen) {}
    Type type() const override;
    void print(std::ostream& os) const override;
    ValueType *clone() const override;
};

class Value {
public:
    ValueType* valueType = nullptr;
    std::vector<Use> usedBys;
    int slot = -1;

    virtual ~Value() = default;
    virtual LLVMType type() const;
    virtual void print(std::ostream& os) const;
    virtual void printRef(std::ostream& os) const;
};
class Argument : public Value {
public:
    Argument();
    LLVMType type() const override;
    void print(std::ostream &os) const override;
};
class BasicBlock : public Value {
public:
    Function* function;
    std::vector<Instruction*> insts;
    std::vector<BasicBlock*> froms;
    std::vector<BasicBlock*> tos;

    BasicBlock(Function* function);
    LLVMType type() const override;
    void print(std::ostream &os) const override;
    void printRef(std::ostream &os) const override;
    void addInst(Instruction* instruction);
    void removeDeadInst();
    void fillSlot();
};
class Constant : public Value {
public:
    LLVMType type() const override;
};
class NumericLiteral : public Constant {
public:
    int value;

    NumericLiteral(int value, BasicValueType::BasicType basicType);
    LLVMType type() const override;
    void print(std::ostream &os) const override;
    void printRef(std::ostream &os) const override;
};
class StringLiteral : public Constant {
public:
    std::string value;
    int refId;

    StringLiteral(const std::string &value);
    LLVMType type() const override;
    void print(std::ostream &os) const override;
    void printRef(std::ostream &os) const override;
};
class GlobalValue : public Constant {
public:
    std::string name;

    LLVMType type() const override;
    void printRef(std::ostream &os) const override;
};
class Function : public GlobalValue {
public:
    std::vector<Argument*> args;
    std::vector<BasicBlock*> blocks;
    SlotTracker slotTracker;

    Function(std::string const &name, BasicValueType::BasicType retType);

    LLVMType type() const override;
    void print(std::ostream &os) const override;
    void fillSlot();
};
class GlobalVariable : public GlobalValue {
public:
    bool zeroInit;
    int initVal;
    std::vector<int> initVals;

    LLVMType type() const override;
    void print(std::ostream& os) const override;
};
class User : public Value {
public:
    std::vector<Use> usings;

    LLVMType type() const override;
};
class Instruction : public User {
public:
    LLVMType type() const override;
};
class BinaryInst : public Instruction {
public:
    enum Op {
        ADD,
        SUB,
        MUL,
        SDIV,
        SREM,
        AND,
        OR,
        EQ,
        NE,
        SGE,
        SGT,
        SLE,
        SLT
    } op;
    Value* l;
    Value* r;

    BinaryInst(Op op, Value* l, Value* r);
    LLVMType type() const override;
    void print(std::ostream &os) const override;
};
class CallInst : public Instruction {
public:
    std::string name;
    std::vector<Value*> args;

    CallInst(bool requireSlot, Function* function, std::vector<Value*> const& args);
    LLVMType type() const override;
    void print(std::ostream &os) const override;
};
class AllocaInst : public Instruction {
public:
    ValueType* allocType;

    AllocaInst(VariableType& variableType);
    AllocaInst(ValueType* allocType);
    LLVMType type() const override;
    void print(std::ostream &os) const override;
};
class LoadInst : public Instruction {
public:
    Value* ptr;

    LoadInst(Value* ptr);
    LLVMType type() const override;
    void print(std::ostream &os) const override;
};
class StoreInst : public Instruction {
public:
    Value* value;
    Value* ptr;

    StoreInst(Value* value, Value* ptr);
    LLVMType type() const override;
    void print(std::ostream &os) const override;
};
class GetElementPtr : public Instruction {
public:
    Value* ptr;
    Value* idx;

    GetElementPtr(Value* ptr, Value* idx);
    LLVMType type() const override;
    void print(std::ostream &os) const override;
};
class ZextInst : public Instruction {
public:
    Value* v;

    ZextInst(Value* v);
    LLVMType type() const override;
    void print(std::ostream &os) const override;
};
class TruncInst : public Instruction {
public:
    Value* v;

    TruncInst(Value* v);
    LLVMType type() const override;
    void print(std::ostream &os) const override;
};
class RetInst : public Instruction {
public:
    Value* value;

    RetInst(Value* value);
    LLVMType type() const override;
    void print(std::ostream &os) const override;
};
class BranchInst : public Instruction {
public:
    Value* cond;
    BasicBlock* ifTrue;
    BasicBlock* ifFalse;

    BranchInst(BasicBlock* uncond);
    BranchInst(Value* cond, BasicBlock* ifTrue, BasicBlock* ifFalse);
    LLVMType type() const override;
    void print(std::ostream &os) const override;
    void printRef(std::ostream &os) const override;
};

class Use {
public:
    User* user;
    Value* value;

    Use(User* user, Value* value);
};

class Module {
public:
    Function* getInt;
    Function* getChar;
    Function* putInt;
    Function* putChar;
    Function* putStr;
    std::vector<GlobalVariable*> variables;
    std::vector<StringLiteral*> strings;
    std::vector<Function*> functions;

    Function* main;

    Module();
    void print(std::ostream& os) const;
    StringLiteral* addStringLiteral(std::string const& string);
    void fillSlot();
};

} // thm

#endif //LLVM_H
