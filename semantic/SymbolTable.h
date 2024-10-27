//
// Created by slty5 on 24-10-11.
//

#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H
#include <memory>
#include <string>
#include <unordered_map>

#include "Symbol.h"

namespace thm {

class SymbolTable {
private:
    int scopeId_;
    std::unordered_map<std::string, std::shared_ptr<Symbol>> symbols_;
    std::shared_ptr<SymbolTable> parent_;
public:
    SymbolTable(int scopeId, std::shared_ptr<SymbolTable> parent) : scopeId_(scopeId), parent_(parent) {}

    bool hasSymbolInScope(const std::string& ident) const;
    std::shared_ptr<Symbol> findSymbol(const std::string& ident) const;
    bool addSymbol(std::shared_ptr<Symbol> symbol);
    int getScopeId() const { return scopeId_; }
    void print(std::ostream& out) const;
};

} // thm

#endif //SYMBOLTABLE_H
