//
// Created by slty5 on 24-9-20.
//

#include "Token.h"

#include <unordered_map>

namespace thm {
    std::unordered_map<std::string, TokenType> reserved = {
        {"main", MAINTK},
        {"const", CONSTTK},
        {"int", INTTK},
        {"char", CHARTK},
        {"break", BREAKTK},
        {"continue", CONTINUETK},
        {"if", IFTK},
        {"else", ELSETK},
        {"for", FORTK},
        {"getint", GETINTTK},
        {"getchar", GETCHARTK},
        {"printf", PRINTFTK},
        {"return", RETURNTK},
        {"void", VOIDTK}
    };

    std::string tokenTypeToString(TokenType type) {

        switch (type) {
#define X(a, b) \
    case TokenType::a: \
        return b;
            TOKEN_TYPES
            default:
                return "UNKNOWN";
        }
#undef X
    }

    TokenType reserve(const std::string& content) {
        if (reserved.contains(content)) {
            return reserved[content];
        }
        return IDENFR;
    }

} // thm