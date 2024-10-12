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
            ptr->lineno = currentLine_;
            static int cnt = 0;
            if (++cnt > 10000 && ptr->nodeType() == ASTNode::EXP) {
                int *a = 0;
                *a = 1;
            }
            if (logger_) {
                logger_->stream() << *ptr;
            }
        }
        std::unique_ptr<CompUnit> parseCompUnit();
        std::unique_ptr<Decl> parseDecl();
        std::unique_ptr<ConstDecl> parseConstDecl();
        std::unique_ptr<BType> parseBType();
        std::unique_ptr<ConstDef> parseConstDef();
        std::unique_ptr<ConstInitVal> parseConstInitVal();
        std::unique_ptr<VarDecl> parseVarDecl();
        std::unique_ptr<VarDef> parseVarDef();
        std::unique_ptr<InitVal> parseInitVal();
        std::unique_ptr<FuncDef> parseFuncDef();
        std::unique_ptr<MainFuncDef> parseMainFuncDef();
        std::unique_ptr<FuncType> parseFuncType();
        std::unique_ptr<FuncFParams> parseFuncFParams();
        std::unique_ptr<FuncFParam> parseFuncFParam();
        std::unique_ptr<Block> parseBlock();
        std::unique_ptr<BlockItem> parseBlockItem();
        std::unique_ptr<Stmt> parseStmt();
        std::unique_ptr<ForStmt> parseForStmt();
        std::unique_ptr<Exp> parseExp();
        std::unique_ptr<Cond> parseCond();
        std::unique_ptr<LVal> parseLVal();
        std::unique_ptr<PrimaryExp> parsePrimaryExp();
        std::unique_ptr<Number> parseNumber();
        std::unique_ptr<Character> parseCharacter();
        std::unique_ptr<UnaryExp> parseUnaryExp();
        std::unique_ptr<UnaryOp> parseUnaryOp();
        std::unique_ptr<FuncRParams> parseFuncRParams();
        std::unique_ptr<MulExp> parseMulExp();
        std::unique_ptr<AddExp> parseAddExp();
        std::unique_ptr<RelExp> parseRelExp();
        std::unique_ptr<EqExp> parseEqExp();
        std::unique_ptr<LAndExp> parseLAndExp();
        std::unique_ptr<LOrExp> parseLOrExp();
        std::unique_ptr<ConstExp> parseConstExp();

        void setLogger(const std::shared_ptr<Logger> & shared) {logger_ = shared;}
    };

} // thm

#endif //PARSER_H
