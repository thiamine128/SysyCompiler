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
    std::shared_ptr<Scope> parent;
    std::shared_ptr<Scope> returnScope;
    std::shared_ptr<SymbolTable> symbolTable;
    std::shared_ptr<SymbolTable> loopScope;
    bool isReturnScope;
    bool requireReturnValue;

    Scope(int scopeId, std::shared_ptr<Scope> parent, std::shared_ptr<SymbolTable> symbolTable, bool isReturnScope, bool requireReturnValue);

    bool canReturnWithValue();
};

} // thm

#endif //SCOPE_H
