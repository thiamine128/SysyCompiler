//
// Created by slty5 on 24-9-21.
//

#include "Compiler.h"

#include <algorithm>
#include <fstream>

#include "../error/CompilerException.h"

namespace thm {
    Compiler::Compiler(std::string const &source) : source_(source), parser_(TokenStream(tokens_)) {

    }

    void Compiler::lexer() {
        std::ifstream file;
        file.open(source_);
        Lexer lexer = Lexer(file);
        for (;;) {
            Token token;
            lexer.next(token);
            tokens_.push_back(token);
            if (token.type == Token::TK_EOF) {
                break;
            }
        }
        file.close();
        lexerErrors = lexer.errorReporter();
    }

    void Compiler::parse() {
        parser_.nextToken();
        auto ptr = parser_.parseCompUnit();
    }

    void Compiler::printInfo() {
        std::ofstream errorfile;
        errorfile.open("error.txt");
        if (!lexerErrors.hasErrors()) {
            std::ofstream lexerfile;
            lexerfile.open("lexer.txt");
            for (auto token : tokens_) {
                if (token.type == Token::TK_EOF) break;
                lexerfile << thm::tokenTypeToString(token.type) << " " << token.content << std::endl;
            }
            lexerfile.close();
        }
        std::vector<CompilerException> exceptions = lexerErrors.getErrors();
        exceptions.insert(exceptions.end(), parser_.errorReporter().getErrors().begin(), parser_.errorReporter().getErrors().end());
        std::sort(exceptions.begin(), exceptions.end(), [](CompilerException const &a, CompilerException const &b) {
            return a.line < b.line;
        });
        for (auto exception : exceptions) {
            errorfile << exception.line << " " << getErrorCode(exception.errorType) << std::endl;
        }
        errorfile.close();

    }
} // thm