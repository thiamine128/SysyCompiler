//
// Created by slty5 on 24-10-11.
//

#include "SymbolTable.h"

#include <algorithm>
#include <ostream>

namespace thm {
    SymbolTable::SymbolTable(int scopeId, SymbolTable *parent) : scopeId(scopeId), parent(parent) {

    }

    bool SymbolTable::hasSymbolInScope(const std::string &ident) const {
        return symbols.find(ident) != symbols.end();
    }

    Symbol* SymbolTable::findSymbol(const std::string &ident) const {
        auto it = symbols.find(ident);
        if (it != symbols.end()) {
            return it->second;
        }
        if (parent != nullptr) {
            return parent->findSymbol(ident);
        }
        return nullptr;
    }

    bool SymbolTable::addSymbol(Symbol* symbol) {
        if (hasSymbolInScope(symbol->ident.content)) return false;
        symbols[symbol->ident.content] = symbol;
        return true;
    }

    void SymbolTable::print(std::ostream &out) const {
        std::vector<Symbol*> symbolList;
        for (auto it = symbols.begin(); it != symbols.end(); ++it) {
            symbolList.push_back(it->second);
        }
        std::sort(symbolList.begin(), symbolList.end(), [](const Symbol* a, const Symbol* b) {
            return a->id < b->id;
        });
        for (auto it = symbolList.begin(); it != symbolList.end(); ++it) {
            out << (*it)->scopeId << " " << (*it)->ident.content << " " << (*it)->typeString() << std::endl;
        }
    }
} // thm