//
// Created by slty5 on 24-10-11.
//

#ifndef SYMBOL_H
#define SYMBOL_H
#include <string>
#include <vector>

#include "../lexer/Token.h"

namespace thm {

class Symbol {
public:
    virtual ~Symbol() = default;

    enum Type {
        VARIABLE,
        FUNCTION
    };
    int id;
    int scopeId;
    Token ident;

    virtual std::string typeString() const {return "";};
    virtual Type symbolType() const { return VARIABLE; };
};

class VariableType {
public:
    enum Type {
        INT,
        CHAR
    } type;
    bool isConst;
    bool isArray;
    int arrayLen;
};

class VariableSymbol : public Symbol {
public:
    VariableType type;
    bool hasInit;
    int initVal;
    std::vector<int> initVals;

    std::string typeString() const override;
    Type symbolType() const override;
};

class FunctionSymbol : public Symbol {
public:
    enum Type {
        INT,
        CHAR,
        VOID
    } type;
    std::vector<VariableType> paramTypes;
    std::string typeString() const override;
    Symbol::Type symbolType() const override;
};

} // thm

#endif //SYMBOL_H
