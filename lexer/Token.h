//
// Created by slty5 on 24-9-20.
//

#ifndef TOKEN_H
#define TOKEN_H


#include <string>

#define TOKEN_TYPES \
    X(IDENFR, "IDENFR") \
    X(INTCON, "INTCON") \
    X(STRCON, "STRCON") \
    X(CHRCON, "CHRCON") \
    X(MAINTK, "MAINTK") \
    X(CONSTTK, "CONSTTK") \
    X(INTTK, "INTTK") \
    X(CHARTK, "CHARTK") \
    X(BREAKTK, "BREAKTK") \
    X(CONTINUETK, "CONTINUETK") \
    X(IFTK, "IFTK") \
    X(ELSETK, "ELSETK") \
    X(NOT, "NOT") \
    X(AND, "AND") \
    X(OR, "OR") \
    X(FORTK, "FORTK") \
    X(GETINTTK, "GETINTTK") \
    X(GETCHARTK, "GETCHARTK") \
    X(PRINTFTK, "PRINTFTK") \
    X(RETURNTK, "RETURNTK") \
    X(PLUS, "PLUS") \
    X(MINU, "MINU") \
    X(VOIDTK, "VOIDTK") \
    X(MULT, "MULT") \
    X(DIV, "DIV") \
    X(MOD, "MOD") \
    X(LSS, "LSS") \
    X(LEQ, "LEQ") \
    X(GRE, "GRE") \
    X(GEQ, "GEQ") \
    X(EQL, "EQL") \
    X(NEQ, "NEQ") \
    X(ASSIGN, "ASSIGN") \
    X(SEMICN, "SEMICN") \
    X(COMMA, "COMMA") \
    X(LPARENT, "LPARENT") \
    X(RPARENT, "RPARENT") \
    X(LBRACK, "LBRACK") \
    X(RBRACK, "RBRACK") \
    X(LBRACE, "LBRACE") \
    X(RBRACE, "RBRACE") \
    X(TK_EOF, "TK_EOF")


namespace thm {
    enum TokenType {
    #define X(a, b) a,
        TOKEN_TYPES
    #undef X
    };

    struct Token {
        TokenType type;
        std::string content;
        int lineno{};

        Token() : type(TokenType::TK_EOF) {};
        Token(TokenType type, std::string const& content, int lineno) : type(type), content(content), lineno(lineno) {}
    };
    std::string tokenTypeToString(TokenType t);
    TokenType reserve(std::string const& content);
} // thm

#endif //TOKEN_H
