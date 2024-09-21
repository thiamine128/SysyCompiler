#include <iostream>
#include <fstream>
#include <vector>

#include "core/Compiler.h"

int main() {
    thm::Compiler compiler("testfile.txt");

    compiler.lexer();

#ifdef PRINT_LEXER
    compiler.printTokens();
#endif
    return 0;
}
