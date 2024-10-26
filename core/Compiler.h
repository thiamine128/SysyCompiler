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
        TokenStream tokenStream_;
        ErrorReporter errorReporter_;
        std::unique_ptr<Lexer> lexer_;
        std::unique_ptr<Parser> parser_;
        std::shared_ptr<CompUnit> compUnit_;

    public:
        Compiler(std::string const &source);

        void lexer();

        void parse();

        void buildSymbolTables();

        void printErrors();
    };
} // thm

#endif //COMPILER_H
