//
// Created by slty5 on 24-10-27.
//

#ifndef LLVM_H
#define LLVM_H
#include <string>
#include <vector>

#include "../semantic/Symbol.h"
namespace thm {
    class Use;

    enum class LLVMType {
    DEFAULT,
    ARGUMENT,
    BASIC_BLOCK,
    CONSTANT,
    CONSTANT_DATA,
    FUNCTION,
    GLOBAL_VARIABLE,
    BINARY_OPERATION,
    COMPARE,
    BRANCH,
    RETURN,
    STORE,
    CALL,
    ALLOCA,
    LOAD,
    UNARY_OPERATOR,
};
class ValueType {
public:
    enum Type {
        I8,
        I32,
        VOID,
        LABEL
    } type;
    bool isPtr;

    ValueType(Type type, bool isPtr);
};
class Value {
public:
    ValueType valueType;
    std::vector<Use> usees;
    std::vector<Use> users;
    virtual ~Value() = default;
    Value();

    virtual void print(std::ostream& os) const;
    virtual LLVMType type() const;
};
class Argument : public Value {
public:

};

class Use {
public:
    Value* user;
    Value* usee;

    Use(Value* user, Value* usee);
};

class Module {

};

} // thm

#endif //LLVM_H
