//
// Created by slty5 on 24-10-12.
//

#ifndef SCOPE_H
#define SCOPE_H
#include <memory>

#include "../symbol/SymbolTable.h"

namespace thm {

class Scope {
public:
    int scopeId;
    std::shared_ptr<Scope> parent;
    std::shared_ptr<SymbolTable> symbolTable;
};

} // thm

#endif //SCOPE_H
