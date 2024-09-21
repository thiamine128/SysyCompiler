//
// Created by slty5 on 24-9-21.
//

#include "Compiler.h"

#include <fstream>

namespace thm {
    Compiler::Compiler(std::string const &source) : source_(source) {

    }

    void Compiler::lexer() {
        std::ifstream file;
        file.open(source_);
        Lexer lexer = Lexer(file);
        for (;;) {
            thm::Token token;
            if (lexer.next(token)) {
                errorTokens_.push_back(token);
            }
            if (token.type == thm::TK_EOF) {
                break;
            }
            tokens_.push_back(token);
        }
        file.close();
    }
} // thm