//
// Created by slty5 on 24-9-20.
//

#ifndef LEXER_H
#define LEXER_H
#include <istream>

#include "Token.h"

namespace thm {

class Lexer {
protected:
    std::istream& input_;
    int currentLine = 1;
public:
    explicit Lexer(std::istream& in) : input_(in) {};

    int next(Token& token);
};

} // thm

#endif //LEXER_H
