//
// Created by slty5 on 24-10-27.
//

#ifndef LLVM_H
#define LLVM_H
#include <string>
#include <unordered_set>
#include <vector>

#include "SlotTracker.h"
#include "../semantic/Symbol.h"
namespace thm {
    class PhiInst;
    class BranchInst;
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
    PHI_INST,
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
    std::vector<Value *> usedBys;
    int slot = -1;
    int color = -1;

    virtual ~Value();
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
    Function *function;
    std::vector<Instruction *> insts;
    std::vector<BasicBlock *> froms;
    std::vector<BasicBlock *> tos;
    std::unordered_set<Value *> def;
    std::unordered_set<Value *> use;
    std::unordered_set<Value *> in;
    std::unordered_set<Value *> out;
    BasicBlock *iDom = nullptr;
    std::unordered_set<BasicBlock *> doms;
    std::unordered_set<BasicBlock *> df;
    std::unordered_map<AllocaInst *, PhiInst *> phis;
    std::unordered_map<AllocaInst *, Value *> allocaTracker;
    int blockIdx;

    BasicBlock(Function* function);
    LLVMType type() const override;
    void print(std::ostream &os) const override;
    void printRef(std::ostream &os) const override;
    bool isReachable() const;
    void calcDefUse();
    void addInstAhead(Instruction *instruction);
    void addInst(Instruction *instruction);
    void setupTransfer();
    bool canMerge();
    void merge(BasicBlock *block);
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
    std::vector<BasicBlock *> blocks;
    std::vector<AllocaInst *> allocas;
    SlotTracker slotTracker;
    Frame *frame;

    Function(std::string const &name, BasicValueType::BasicType retType);

    LLVMType type() const override;
    void print(std::ostream &os) const override;
    void removeUnreachableBlocks();
    void arrangeBlocks();
    void calcDominators();
    void setAllocas();
    void livenessAnalysis();
    void fillSlot();
    int getMaxArgs();
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
    std::vector<Value **> usings;

    LLVMType type() const override;
    void use(Value **value);
};
class Instruction : public User {
public:
    BasicBlock *block;

    LLVMType type() const override;
    virtual void getDefUse(std::unordered_set<Value *> &def, std::unordered_set<Value *> &use);
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
    void getDefUse(std::unordered_set<Value *> &def, std::unordered_set<Value *> &use) override;
};
class CallInst : public Instruction {
public:
    std::string name;
    Function *function;
    std::vector<Value*> args;

    CallInst(bool requireSlot, Function* function, std::vector<Value*> const& args);
    LLVMType type() const override;
    void print(std::ostream &os) const override;
    void getDefUse(std::unordered_set<Value *> &def, std::unordered_set<Value *> &use) override;
};
class AllocaInst : public Instruction {
public:
    ValueType* allocType;
    int argIdx = -1;

    AllocaInst(VariableType& variableType);
    AllocaInst(ValueType* allocType, int argIdx);
    LLVMType type() const override;
    void print(std::ostream &os) const override;
    void getDefUse(std::unordered_set<Value *> &def, std::unordered_set<Value *> &use) override;
    bool isPromotable() const;
};
class LoadInst : public Instruction {
public:
    Value* ptr;

    LoadInst(Value* ptr);
    LLVMType type() const override;
    void print(std::ostream &os) const override;
    void getDefUse(std::unordered_set<Value *> &def, std::unordered_set<Value *> &use) override;
};
class StoreInst : public Instruction {
public:
    Value* value;
    Value* ptr;
    bool isArgInit = false;

    StoreInst(Value* value, Value* ptr);
    StoreInst(Value* value, Value* ptr, bool isArgInit);
    LLVMType type() const override;
    void print(std::ostream &os) const override;
    void getDefUse(std::unordered_set<Value *> &def, std::unordered_set<Value *> &use) override;
};
class GetElementPtr : public Instruction {
public:
    Value* ptr;
    Value* idx;

    GetElementPtr(Value* ptr, Value* idx);
    LLVMType type() const override;
    void print(std::ostream &os) const override;
    void getDefUse(std::unordered_set<Value *> &def, std::unordered_set<Value *> &use) override;
};
class ZextInst : public Instruction {
public:
    Value* v;

    ZextInst(Value* v);
    LLVMType type() const override;
    void print(std::ostream &os) const override;
    void getDefUse(std::unordered_set<Value *> &def, std::unordered_set<Value *> &use) override;
};
class TruncInst : public Instruction {
public:
    Value* v;

    TruncInst(Value* v);
    LLVMType type() const override;
    void print(std::ostream &os) const override;
    void getDefUse(std::unordered_set<Value *> &def, std::unordered_set<Value *> &use) override;
};
class RetInst : public Instruction {
public:
    Value* value;

    RetInst(Value* value);
    LLVMType type() const override;
    void print(std::ostream &os) const override;
    void getDefUse(std::unordered_set<Value *> &def, std::unordered_set<Value *> &use) override;
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
    void getDefUse(std::unordered_set<Value *> &def, std::unordered_set<Value *> &use) override;
};
class PhiInst : public Instruction {
public:
    AllocaInst* alloc;
    std::unordered_map<BasicBlock *, Value *> opt;

    PhiInst(AllocaInst* alloc);
    LLVMType type() const override;
    void print(std::ostream &os) const override;
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
    void preprocess();
};

} // thm

#endif //LLVM_H
