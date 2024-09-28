//
// Created by slty5 on 24-9-20.
//

#ifndef LEXER_H
#define LEXER_H
#include <istream>
#include <optional>

#include "Token.h"
#include "TokenStream.h"
#include "../core/Logger.h"
#include "../error/ErrorReporter.h"

namespace thm {

    class Lexer {
    protected:
        std::istream& input_;
        int currentLine = 1;
        ErrorReporter& errorReporter_;
        std::shared_ptr<Logger> logger_;
    public:
        explicit Lexer(std::istream& in, ErrorReporter& errorReporter) : input_(in), errorReporter_(errorReporter) {};

        void next(Token& token);
        void tokenize(TokenStream& tokenStream);
        void setLogger(std::shared_ptr<Logger> logger) { logger_ = logger; };
    };

} // thm

#endif //LEXER_H
