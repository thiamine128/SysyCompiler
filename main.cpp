#include <iostream>
#include <fstream>
#include <vector>

#include "core/Compiler.h"

int main() {
    thm::Compiler compiler("testfile.txt");

    compiler.lexer();
    //compiler.parse();
#ifdef PRINT_LEXER
    compiler.printInfo();
#endif
    return 0;
}
