//
// Created by slty5 on 24-9-21.
//

#ifndef COMPILER_H
#define COMPILER_H
#include <memory>
#include <string>
#include <vector>

#include "../lexer/Lexer.h"
#include "../error/CompilerException.h"
#include "../error/ErrorReporter.h"
#include "../parser/Parser.h"

namespace thm {

    class Compiler {
    protected:
        std::string source_;
        std::vector<Token> tokens_;
        Parser parser_;
        ErrorReporter lexerErrors;
    public:
        Compiler(std::string const& source);
        void lexer();
        void parse();
        void printInfo();
    };

} // thm

#endif //COMPILER_H
