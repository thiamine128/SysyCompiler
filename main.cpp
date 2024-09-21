#include <iostream>
#include <fstream>
#include <vector>

#include "Compiler.h"
#include "lexer/Lexer.h"
#include "lexer/Token.h"

int main() {
    thm::Compiler compiler("testfile.txt");

    compiler.lexer();

#ifdef PRINT_LEXER
    if (compiler.getErrorTokens().empty()) {
        std::ofstream lexerfile;
        lexerfile.open("lexer.txt");
        for (auto token : compiler.getTokens()) {
            lexerfile << thm::tokenTypeToString(token.type) << " " << token.content << std::endl;
        }
        lexerfile.close();
    } else {
        std::ofstream errorfile;
        errorfile.open("error.txt");
        for (auto token : compiler.getErrorTokens()) {
            errorfile << token.lineno << " a" << std::endl;
        }
        errorfile.close();
    }
#endif
    return 0;
}
