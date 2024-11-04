//
// Created by slty5 on 24-10-11.
//

#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H
#include <string>
#include <unordered_map>

#include "Symbol.h"

namespace thm {

class SymbolTable {
public:
    int scopeId;
    std::unordered_map<std::string, Symbol*> symbols;
    SymbolTable* parent;

    SymbolTable(int scopeId, SymbolTable* parent);

    bool hasSymbolInScope(const std::string& ident) const;
    Symbol* findSymbol(const std::string& ident) const;
    bool addSymbol(Symbol* symbol);
    int getScopeId() const { return scopeId; }
    void print(std::ostream& out) const;
};

} // thm

#endif //SYMBOLTABLE_H
