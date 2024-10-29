//
// Created by slty5 on 24-10-12.
//

#include "Scope.h"

#include "../parser/AbstractSyntaxTree.h"
#include "../util/util.h"

namespace thm {
    Scope::Scope(int scopeId, Scope* parent, SymbolTable* symbolTable, bool isReturnScope, bool requireReturnValue) : scopeId(scopeId), parent(parent), symbolTable(symbolTable), requireReturnValue(requireReturnValue), isReturnScope(isReturnScope) {
        if (!isReturnScope && parent != nullptr) {
            if (parent->isReturnScope) {
                returnScope = parent;
            } else {
                returnScope = parent->returnScope;
            }
        }
    }

    bool Scope::canReturnWithValue() {
        if (isReturnScope) {
            return requireReturnValue;
        }
        if (returnScope != nullptr) {
            return returnScope->canReturnWithValue();
        }
        return false;
    }
} // thm