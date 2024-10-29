//
// Created by slty5 on 24-10-12.
//

#ifndef SCOPE_H
#define SCOPE_H
#include <memory>

#include "../semantic/SymbolTable.h"

namespace thm {

class Scope {
public:
    int scopeId;
    Scope* parent;
    Scope* returnScope;
    SymbolTable* symbolTable;
    SymbolTable* loopScope;
    bool isReturnScope;
    bool requireReturnValue;

    Scope(int scopeId, Scope* parent, SymbolTable* symbolTable, bool isReturnScope, bool requireReturnValue);

    bool canReturnWithValue();
};

} // thm

#endif //SCOPE_H
