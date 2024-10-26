//
// Created by slty5 on 24-9-27.
//

#ifndef PARSER_H
#define PARSER_H

#include <fstream>
#include <optional>

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
        std::shared_ptr<CompUnit> parseCompUnit();
        std::shared_ptr<Decl> parseDecl();
        std::shared_ptr<ConstDecl> parseConstDecl();
        std::shared_ptr<BType> parseBType();
        std::shared_ptr<ConstDef> parseConstDef();
        std::shared_ptr<ConstInitVal> parseConstInitVal();
        std::shared_ptr<VarDecl> parseVarDecl();
        std::shared_ptr<VarDef> parseVarDef();
        std::shared_ptr<InitVal> parseInitVal();
        std::shared_ptr<FuncDef> parseFuncDef();
        std::shared_ptr<MainFuncDef> parseMainFuncDef();
        std::shared_ptr<FuncType> parseFuncType();
        std::shared_ptr<FuncFParams> parseFuncFParams();
        std::shared_ptr<FuncFParam> parseFuncFParam();
        std::shared_ptr<Block> parseBlock();
        std::shared_ptr<BlockItem> parseBlockItem();
        std::shared_ptr<Stmt> parseStmt();
        std::shared_ptr<ForStmt> parseForStmt();
        std::shared_ptr<Exp> parseExp();
        int stepExp(int offset);
        std::shared_ptr<Cond> parseCond();
        std::shared_ptr<LVal> parseLVal();
        int stepLVal(int offset);
        std::shared_ptr<PrimaryExp> parsePrimaryExp();
        int stepPrimaryExp(int offset);
        std::shared_ptr<Number> parseNumber();
        int stepNumber(int offset);
        std::shared_ptr<Character> parseCharacter();
        int stepCharacter(int offset);
        std::shared_ptr<UnaryExp> parseUnaryExp();
        int stepUnaryExp(int offset);
        std::shared_ptr<UnaryOp> parseUnaryOp();
        int stepUnaryOp(int offset);
        std::shared_ptr<FuncRParams> parseFuncRParams();
        int stepFuncRParams(int offset);
        std::shared_ptr<MulExp> parseMulExp();
        int stepMulExp(int offset);
        std::shared_ptr<AddExp> parseAddExp();
        int stepAddExp(int offset);
        std::shared_ptr<RelExp> parseRelExp();
        std::shared_ptr<EqExp> parseEqExp();
        std::shared_ptr<LAndExp> parseLAndExp();
        std::shared_ptr<LOrExp> parseLOrExp();
        std::shared_ptr<ConstExp> parseConstExp();

        void setLogger(const std::shared_ptr<Logger> & shared) {logger_ = shared;}
    };

} // thm

#endif //PARSER_H
