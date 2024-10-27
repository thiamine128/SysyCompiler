//
// Created by slty5 on 24-10-27.
//

#ifndef LLVM_H
#define LLVM_H
#include <memory>
#include <string>
#include <vector>

#include "../semantic/Symbol.h"

namespace thm {

class Value {

};

class GlobalValue : public Value {
public:
    std::string name;
};

class GlobalVariable : public GlobalValue {
public:
    std::shared_ptr<Symbol> symbol;

    GlobalVariable(std::shared_ptr<Symbol> symbol) : symbol(symbol) {}
};

class User : public Value {

};

class Instruction : public User {
public:
    enum Ty {
        I32,
        I8
    };
    enum CmpCond {

    };
};

class BasicBlock : public Value {
public:
    std::vector<std::shared_ptr<Instruction>> instructions;
};

class Function : public GlobalValue {
public:
    std::vector<std::shared_ptr<BasicBlock>> blocks;
};

class Module {
public:
    std::vector<std::shared_ptr<GlobalVariable>> variables;
    std::vector<std::shared_ptr<Function>> functions;
};

class Add : public Instruction {
public:
    Ty ty;
    std::shared_ptr<Value> op1, op2;
};

class Sub : public Instruction {
public:
    Ty ty;
    std::shared_ptr<Value> op1, op2;
};

class Mul : public Instruction {
public:
    Ty ty;
    std::shared_ptr<Value> op1, op2;
};

class SDiv : public Instruction {
public:
    Ty ty;
    std::shared_ptr<Value> op1, op2;
};

class SRem : public Instruction {
public:
    Ty ty;
    std::shared_ptr<Value> op1, op2;
};

class ICmp : public Instruction {
public:
    Ty ty;
    CmpCond cond;
    std::shared_ptr<Value> op1, op2;
};

class And : public Instruction {
public:
    Ty ty;
    std::shared_ptr<Value> op1, op2;
};

class Or : public Instruction {
public:
    Ty ty;
    std::shared_ptr<Value> op1, op2;
};

class Call : public Instruction {
public:
    Ty ty;
    std::string name;
    std::vector<std::shared_ptr<Value>> args;
};

class Alloca : public Instruction {
public:
    Ty ty;
};

class Load : public Instruction {
public:
    Ty ty1;
    Ty ty2;
    std::shared_ptr<Value> ptr;
};

class Store : public Instruction {
public:
    Ty ty1;
    std::shared_ptr<Value> value;
    Ty ty2;
    std::shared_ptr<Value> ptr;
};

class GetElementPtr : public Instruction {
public:
    Ty ty1;
    Ty ty2;
    int idx;
};

class Zext : public Instruction {
public:
    Ty ty;
    std::shared_ptr<Value> value;
    Ty ty2;
};

class Trunc : public Instruction {
public:
    Ty ty;
    std::shared_ptr<Value> value;
    Ty ty2;
};

class Br : public Instruction {
public:
    CmpCond cond;
    std::shared_ptr<Value> dest1;
    std::shared_ptr<Value> dest2;
};

class Ret : public Instruction {
public:
    Ty ty;
    std::shared_ptr<Value> value;
};

class LLVM {

};

} // thm

#endif //LLVM_H
