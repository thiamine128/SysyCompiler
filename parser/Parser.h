//
// Created by slty5 on 24-9-27.
//

#ifndef PARSER_H
#define PARSER_H
#include "../error/ErrorReporter.h"
#include "../lexer/TokenStream.h"

namespace thm {

    class Parser {
    private:
        TokenStream tokenStream_;
        Token token_;
        ErrorReporter errorReporter_;
        std::vector<Token> tokens_;
    public:
        Parser(const TokenStream& tokenStream);

        ErrorReporter& errorReporter() { return errorReporter_; }
        void nextToken();
        void ungetToken();
        bool tryMatch(TokenType expectedType);
        void matchToken(TokenType expectedType);
        void finish(std::string const&);
        void parseCompUnit();
        void parseDecl();
        void parseConstDecl();
        void parseBType();
        void parseConstDef();
        void parseConstInitVal();
        void parseVarDecl();
        void parseVarDef();
        void parseInitVal();
        void parseFuncDef();
        void parseMainFuncDef();
        void parseFuncType();
        void parseFuncFParams();
        void parseFuncFParam();
        void parseBlock();
        void parseBlockItem();
        void parseStmt();
        void parseForStmt();
        void parseExp();
        void parseCond();
        void parseLVal();
        void parsePrimaryExp();
        void parseNumber();
        void parseCharacter();
        void parseUnaryExp();
        void parseUnaryOp();
        void parseFuncRParams();
        void parseMulExp();
        void parseAddExp();
        void parseRelExp();
        void parseEqExp();
        void parseLAndExp();
        void parseLOrExp();
        void parseConstExp();
    };

} // thm

#endif //PARSER_H
