cmake_minimum_required(VERSION 3.28)
project(Compiler)

set(CMAKE_CXX_STANDARD 20)

add_executable(Compiler main.cpp
        lexer/Token.cpp
        lexer/Token.h
        lexer/Lexer.cpp
        lexer/Lexer.h
)

add_definitions(-DPRINT_LEXER)