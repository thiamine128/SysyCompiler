#include <iostream>
#include <fstream>
#include <vector>

#include "core/Compiler.h"

int main() {
    thm::Compiler compiler("testfile.txt");

    compiler.lexer();
    compiler.parse();
    //compiler.buildSymbolTables();
    compiler.printErrors();
    int* a = 0;
    *a = 1;
    return 0;
}
