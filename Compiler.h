//
// Created by slty5 on 24-9-21.
//

#ifndef COMPILER_H
#define COMPILER_H
#include <string>
#include <vector>

#include "lexer/Lexer.h"

namespace thm {

class Compiler {
protected:
    std::string source_;
    std::vector<Token> tokens_;
    std::vector<Token> errorTokens_;
public:
    Compiler(std::string const& source);
    void lexer();
    void printTokens();
};

} // thm

#endif //COMPILER_H
