//
// Created by slty5 on 24-9-21.
//

#include "Compiler.h"

#include <algorithm>
#include <fstream>
#include <iostream>

#include "Scope.h"
#include "../error/CompilerException.h"
#include "../mips/MIPSBuilder.h"
#include "../semantic/SemanticVisitor.h"

namespace thm {
    Compiler::Compiler(std::string const &source) : source_(source) {

    }

    void Compiler::lexer() {
        std::ifstream file;
        file.open(source_);
        lexer_ = std::make_unique<Lexer>(file, errorReporter_);
#ifdef PRINT_LEXER
        std::shared_ptr<Logger> logger = std::make_shared<Logger>("lexer.txt");
        lexer_->setLogger(logger);
#endif
        lexer_->tokenize(tokenStream_);
        file.close();
    }

    void Compiler::parse() {
        parser_ = std::make_unique<Parser>(tokenStream_, errorReporter_);
#ifdef PRINT_PARSER
        std::shared_ptr<Logger> logger = std::make_shared<Logger>("parser.txt");
        parser_->setLogger(logger);
#endif
        compUnit_ = parser_->parseCompUnit();
    }

    void Compiler::buildSymbolTables() {
        semanticVisitor = new SemanticVisitor(errorReporter_);
        compUnit_->visit(semanticVisitor);
#ifdef PRINT_SYMBOL
        std::shared_ptr<Logger> logger = std::make_shared<Logger>("symbol.txt");
        for (auto& scope : semanticVisitor->scopes) {
            scope->symbolTable->print(logger->stream());
        }
    }

    void Compiler::buildIR() {
        irBuilder = new IRBuilder();
        compUnit_->visit(irBuilder);
        std::shared_ptr<Logger> logger = std::make_shared<Logger>("llvm_ir.txt");
        irBuilder->module->preprocess();
        irBuilder->module->print(logger->stream());
    }

    void Compiler::buildMIPS() {
        MIPSBuilder mipsBuilder(irBuilder->module, std::cout);
        mipsBuilder.build();
    }
#endif
    void Compiler::printErrors() {
#ifdef PRINT_ERROR
        std::shared_ptr<Logger> logger = std::make_shared<Logger>("error.txt");
        errorReporter_.printErrors(logger);
#endif
    }
} // thm