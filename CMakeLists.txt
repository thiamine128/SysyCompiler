cmake_minimum_required(VERSION 3.15)
project(Compiler)

set(CMAKE_CXX_STANDARD 17)

add_executable(Compiler main.cpp
        lexer/Token.cpp
        lexer/Token.h
        lexer/Lexer.cpp
        lexer/Lexer.h
        core/Compiler.cpp
        core/Compiler.h
        error/CompilerException.cpp
        error/CompilerException.h
        error/ErrorReporter.cpp
        error/ErrorReporter.h
        parser/Parser.cpp
        parser/Parser.h
        lexer/TokenStream.cpp
        lexer/TokenStream.h
        parser/AbstractSyntaxTree.cpp
        parser/AbstractSyntaxTree.h
        core/Logger.cpp
        core/Logger.h
        parser/ASTVisitor.cpp
        parser/ASTVisitor.h
        util/util.h
        symbol/Symbol.cpp
        symbol/Symbol.h
        symbol/SymbolTable.cpp
        symbol/SymbolTable.h
        symbol/SymbolTableBuilder.cpp
        symbol/SymbolTableBuilder.h
        core/Scope.cpp
        core/Scope.h
        util/util.cpp
)

add_definitions(-DPRINT_LEXER)
add_definitions(-DPRINT_PARSER)
add_definitions(-DPRINT_SYMBOL)
add_definitions(-DPRINT_ERROR)