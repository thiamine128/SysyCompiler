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
#include "../llvm/IRBuilder.h"
#include "../parser/Parser.h"

namespace thm {
    class SemanticVisitor;

    class Compiler {
    protected:
        std::string source_;
        TokenStream tokenStream_;
        ErrorReporter errorReporter_;
        std::unique_ptr<Lexer> lexer_;
        std::unique_ptr<Parser> parser_;
        CompUnit* compUnit_;
        SemanticVisitor* semanticVisitor;
        IRBuilder* irBuilder;

    public:
        Compiler(std::string const &source);

        void lexer();

        void parse();

        void buildSymbolTables();

        void buildIR();

        void printErrors();
    };
} // thm

#endif //COMPILER_H
