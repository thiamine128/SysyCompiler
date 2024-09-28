cmake_minimum_required(VERSION 3.28)
project(Compiler)

set(CMAKE_CXX_STANDARD 20)

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
)

add_definitions(-DPRINT_LEXER)
add_definitions(-DPRINT_PARSER)