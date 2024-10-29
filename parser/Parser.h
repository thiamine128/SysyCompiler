//
// Created by slty5 on 24-9-27.
//

#ifndef PARSER_H
#define PARSER_H

#include "AbstractSyntaxTree.h"
#include "../core/Logger.h"
#include "../error/ErrorReporter.h"
#include "../lexer/TokenStream.h"

namespace thm {

    class Parser {
    private:
        TokenStream& tokenStream_;
        ErrorReporter& errorReporter_;
        std::vector<Token> tokens_;
        std::shared_ptr<Logger> logger_;
        int currentLine_;
    public:
        Parser(TokenStream& tokenStream, ErrorReporter& errorReporter);

        Token const& currentToken() const { return tokenStream_.peek(); }
        void nextToken();
        bool tryMatch(Token::TokenType expectedType);
        bool match(Token::TokenType expectedType);
        template <typename T> void submit(T& ptr) {
            if (logger_) {
                logger_->stream() << *ptr;
            }
        }
        CompUnit* parseCompUnit();
        Decl* parseDecl();
        ConstDecl* parseConstDecl();
        BType* parseBType();
        ConstDef* parseConstDef();
        ConstInitVal* parseConstInitVal();
        VarDecl* parseVarDecl();
        VarDef* parseVarDef();
        InitVal* parseInitVal();
        FuncDef* parseFuncDef();
        MainFuncDef* parseMainFuncDef();
        FuncType* parseFuncType();
        FuncFParams* parseFuncFParams();
        FuncFParam* parseFuncFParam();
        Block* parseBlock();
        BlockItem* parseBlockItem();
        Stmt* parseStmt();
        ForStmt* parseForStmt();
        Exp* parseExp();
        int stepExp(int offset);
        Cond* parseCond();
        LVal* parseLVal();
        int stepLVal(int offset);
        PrimaryExp* parsePrimaryExp();
        int stepPrimaryExp(int offset);
        Number* parseNumber();
        int stepNumber(int offset) const;
        Character* parseCharacter();
        int stepCharacter(int offset) const;
        UnaryExp* parseUnaryExp();
        int stepUnaryExp(int offset);
        UnaryOp* parseUnaryOp();
        int stepUnaryOp(int offset) const;
        FuncRParams* parseFuncRParams();
        int stepFuncRParams(int offset);
        MulExp* parseMulExp();
        int stepMulExp(int offset);
        AddExp* parseAddExp();
        int stepAddExp(int offset);
        RelExp* parseRelExp();
        EqExp* parseEqExp();
        LAndExp* parseLAndExp();
        LOrExp* parseLOrExp();
        ConstExp* parseConstExp();

        void setLogger(const std::shared_ptr<Logger> & shared) {logger_ = shared;}
    };

} // thm

#endif //PARSER_H
