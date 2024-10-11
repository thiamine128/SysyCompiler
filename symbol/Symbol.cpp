//
// Created by slty5 on 24-10-11.
//

#include "Symbol.h"

namespace thm {
    std::string VariableSymbol::typeString() const {
        std::string str = "";
        if (type.isConst) str += "Const";
        switch (type.type) {
            case VariableType::INT:
                str += "Int";
            break;
            case VariableType::CHAR:
                str += "Char";
            break;
        }
        if (type.isArray) str += "Array";
        return str;
    }

    Symbol::Type VariableSymbol::symbolType() const {
        return Symbol::VARIABLE;
    }

    std::string FunctionSymbol::typeString() const {
        std::string str = "";
        switch (type) {
            case INT:
                str += "Int";
            break;
            case CHAR:
                str += "Char";
            break;
            default:
                str += "Void";
            break;
        }
        str += "Func";
        return str;
    }

    Symbol::Type FunctionSymbol::symbolType() const {
        return Symbol::FUNCTION;
    }
} // thm