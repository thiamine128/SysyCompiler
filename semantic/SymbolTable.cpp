//
// Created by slty5 on 24-10-11.
//

#include "SymbolTable.h"

#include <algorithm>
#include <ostream>

namespace thm {
    bool SymbolTable::hasSymbolInScope(const std::string &ident) const {
        return symbols_.find(ident) != symbols_.end();
    }

    std::shared_ptr<Symbol> SymbolTable::findSymbol(const std::string &ident) const {
        auto it = symbols_.find(ident);
        if (it != symbols_.end()) {
            return it->second;
        }
        if (parent_ != nullptr) {
            return parent_->findSymbol(ident);
        }
        return nullptr;
    }

    bool SymbolTable::addSymbol(std::shared_ptr<Symbol> symbol) {
        if (hasSymbolInScope(symbol->ident.content)) return false;
        symbols_[symbol->ident.content] = symbol;
        return true;
    }

    void SymbolTable::print(std::ostream &out) const {
        std::vector<std::shared_ptr<Symbol>> symbols;
        for (auto it = symbols_.begin(); it != symbols_.end(); ++it) {
            symbols.push_back(it->second);
        }
        std::sort(symbols.begin(), symbols.end(), [](std::shared_ptr<Symbol> a, std::shared_ptr<Symbol> b) {
            return a->id < b->id;
        });
        for (auto it = symbols.begin(); it != symbols.end(); ++it) {
            out << (*it)->scopeId << " " << (*it)->ident.content << " " << (*it)->typeString() << std::endl;
        }
    }
} // thm