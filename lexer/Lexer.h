//
// Created by slty5 on 24-9-20.
//

#ifndef LEXER_H
#define LEXER_H
#include <istream>

#include "Token.h"
#include "../error/ErrorReporter.h"

namespace thm {

    class Lexer {
    protected:
        std::istream& input_;
        int currentLine = 1;
        ErrorReporter errorReporter_;
    public:
        explicit Lexer(std::istream& in) : input_(in) {};

        void next(Token& token);
        ErrorReporter& errorReporter() { return errorReporter_; }
    };

} // thm

#endif //LEXER_H
