#include <iostream>
#include <fstream>
#include <vector>

#include "lexer/Lexer.h"
#include "lexer/Token.h"

std::vector<thm::Token> tokens;
std::vector<thm::Token> errorTokens;

int main() {
    std::ifstream testfile;
    testfile.open("testfile.txt");
    thm::Lexer lexer(testfile);

    for (;;) {
        thm::Token token;
        if (lexer.next(token)) {
            errorTokens.push_back(token);
        }
        if (token.type == thm::TK_EOF) {
            break;
        }
        tokens.push_back(token);
    }

    testfile.close();
#ifdef PRINT_LEXER
    if (errorTokens.empty()) {
        std::ofstream lexerfile;
        lexerfile.open("lexer.txt");
        for (auto token : tokens) {
            lexerfile << thm::tokenTypeToString(token.type) << " " << token.content << std::endl;
        }
        lexerfile.close();
    } else {
        std::ofstream errorfile;
        errorfile.open("error.txt");
        for (auto token : errorTokens) {
            errorfile << token.lineno << " a" << std::endl;
        }
        errorfile.close();
    }
#endif
    return 0;
}
