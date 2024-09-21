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
    compiler.printTokens();
#endif
    return 0;
}
