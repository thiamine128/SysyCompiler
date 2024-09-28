//
// Created by slty5 on 24-9-20.
//

#include "Token.h"

#include <ostream>
#include <unordered_map>

namespace thm {
    std::unordered_map<std::string, Token::TokenType> reserved = {
        {"main", Token::MAINTK},
        {"const", Token::CONSTTK},
        {"int", Token::INTTK},
        {"char", Token::CHARTK},
        {"break", Token::BREAKTK},
        {"continue", Token::CONTINUETK},
        {"if", Token::IFTK},
        {"else", Token::ELSETK},
        {"for", Token::FORTK},
        {"getint", Token::GETINTTK},
        {"getchar", Token::GETCHARTK},
        {"printf", Token::PRINTFTK},
        {"return", Token::RETURNTK},
        {"void", Token::VOIDTK}
    };

    std::string tokenTypeToString(Token::TokenType type) {

        switch (type) {
#define X(a, b) \
    case Token::a: \
        return b;
            TOKEN_TYPES
            default:
                return "UNKNOWN";
        }
#undef X
    }

    Token::TokenType reserve(const std::string& content) {
        if (reserved.contains(content)) {
            return reserved[content];
        }
        return Token::IDENFR;
    }

    std::ostream & operator<<(std::ostream &os, Token const &t) {
        return os << tokenTypeToString(t.type) << " " << t.content << std::endl;
    }
} // thm