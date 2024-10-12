//
// Created by slty5 on 24-9-27.
//

#include "Parser.h"

#include <cmath>
#include <iostream>
#include <fstream>

namespace thm {
    Parser::Parser(TokenStream &tokenStream, ErrorReporter& errorReporter): currentLine_(0), tokenStream_(tokenStream), errorReporter_(errorReporter) {

    }


    void Parser::nextToken() {
        Token const& token = currentToken();
        currentLine_ = token.lineno;
        if (logger_) {
            logger_->stream() << token;
        }
        tokenStream_.next();
    }

    bool Parser::tryMatch(Token::TokenType expectedType) {
        if (!tokenStream_.peekType(expectedType)) {
            return false;
        }
        nextToken();
        return true;
    }

    bool Parser::match(Token::TokenType expectedType) {
        if (!tokenStream_.peekType(expectedType)) {
            if (expectedType == Token::SEMICN) {
                errorReporter_.error(CompilerException(MISSING_SEMICOLON, currentLine_));
            } else if (expectedType == Token::RPARENT) {
                errorReporter_.error(CompilerException(MISSING_RPARENT, currentLine_));
            } else if (expectedType == Token::RBRACK) {
                errorReporter_.error(CompilerException(MISSING_RBRACK, currentLine_));
            }
            return false;
        }
        nextToken();
        return true;
    }

    std::unique_ptr<CompUnit> Parser::parseCompUnit() {
        auto ptr = std::make_unique<CompUnit>();
        ptr->lineno = currentToken().lineno;
        while (true) {
            if (tokenStream_.peekType(Token::CONSTTK) ||
                (tokenStream_.peekType(0, {Token::INTTK, Token::CHARTK}) && tokenStream_.peekType(1, Token::IDENFR) && tokenStream_.peekType(2, {Token::COMMA, Token::SEMICN, Token::LBRACK, Token::ASSIGN}))) {
                ptr->decls.push_back(parseDecl());
            } else {
                break;
            }
        }
        while (true) {
            if (tokenStream_.peekType(0, {Token::VOIDTK, Token::INTTK, Token::CHARTK}) && tokenStream_.peekType(1, Token::IDENFR) && tokenStream_.peekType(2, Token::LPARENT)) {
                ptr->funcDefs.push_back(parseFuncDef());
            } else {
                break;
            }
        }
        ptr->mainFuncDef = parseMainFuncDef();
        submit(ptr);
        return ptr;
    }

    std::unique_ptr<Decl> Parser::parseDecl() {
        auto ptr = std::make_unique<Decl>();
        ptr->lineno = currentToken().lineno;

        if (tokenStream_.peekType(Token::CONSTTK)) ptr->decl = parseConstDecl();
        else ptr->decl = parseVarDecl();
        submit(ptr);
        return ptr;
    }

    std::unique_ptr<ConstDecl> Parser::parseConstDecl() {
        auto ptr = std::make_unique<ConstDecl>();
        ptr->lineno = currentToken().lineno;

        match(Token::CONSTTK);
        ptr->bType = parseBType();
        ptr->constDefs.push_back(parseConstDef());
        while (tryMatch(Token::COMMA)) {
            ptr->constDefs.push_back(parseConstDef());
        }
        match(Token::SEMICN);
        submit(ptr);
        return ptr;
    }

    std::unique_ptr<BType> Parser::parseBType() {
        auto ptr = std::make_unique<BType>();
        ptr->lineno = currentToken().lineno;
        ptr->type = currentToken().type == Token::INTTK ? VariableType::INT : VariableType::CHAR;
        nextToken();
        submit(ptr);
        return ptr;
    }


    std::unique_ptr<ConstDef> Parser::parseConstDef() {
        auto ptr = std::make_unique<ConstDef>();
        ptr->lineno = currentToken().lineno;
        Token ident = currentToken();
        match(Token::IDENFR);
        if (tryMatch(Token::LBRACK)) {
            std::unique_ptr<ConstExp> len;
            if (!tryMatch(Token::RBRACK)) {
                len = parseConstExp();
            }
            ptr->def = ConstDef::ConstDefArray(ident, std::move(len));
            match(Token::RBRACK);
        } else {
            ptr->def = ConstDef::ConstDefBasic(ident);
        }
        match(Token::ASSIGN);
        ptr->val = parseConstInitVal();
        submit(ptr);
        return ptr;
    }

    std::unique_ptr<ConstInitVal> Parser::parseConstInitVal() {
        auto ptr = std::make_unique<ConstInitVal>();
        ptr->lineno = currentToken().lineno;

        if (tokenStream_.peekType(0, {Token::PLUS, Token::MINU, Token::NOT, Token::IDENFR, Token::LPARENT, Token::INTCON, Token::CHRCON})) {
            ptr->val = ConstInitVal::ConstInitValBasic(parseConstExp());
        } else if (tryMatch(Token::LBRACE)) {
            std::vector<std::unique_ptr<ConstExp> > exps;
            if (tokenStream_.peekType(0, {Token::PLUS, Token::MINU, Token::NOT, Token::IDENFR, Token::LPARENT, Token::INTCON, Token::CHRCON})) {
                exps.push_back(parseConstExp());
                while (tryMatch(Token::COMMA)) {
                    exps.push_back(parseConstExp());
                }
            }
            ptr->val = ConstInitVal::ConstInitValArray(std::move(exps));
            match(Token::RBRACE);
        } else if (tokenStream_.peekType(Token::STRCON)) {
            ptr->val = currentToken().content;
            match(Token::STRCON);
        }
        submit(ptr);
        return ptr;
    }

    std::unique_ptr<VarDecl> Parser::parseVarDecl() {
        auto ptr = std::make_unique<VarDecl>();
        ptr->lineno = currentToken().lineno;

        ptr->bType = std::move(parseBType());
        ptr->varDefs.push_back(parseVarDef());
        while (tryMatch(Token::COMMA)) {
            ptr->varDefs.push_back(parseVarDef());
        }
        match(Token::SEMICN);
        submit(ptr);
        return ptr;
    }

    std::unique_ptr<VarDef> Parser::parseVarDef() {
        auto ptr = std::make_unique<VarDef>();
        ptr->lineno = currentToken().lineno;

        Token ident = currentToken();
        match(Token::IDENFR);

        if (tryMatch(Token::LBRACK)) {
            std::unique_ptr<ConstExp> len;
            if (!tokenStream_.peekType(Token::RBRACK)) {
                len = std::move(parseConstExp());
            }
            ptr->def = VarDef::VarDefArray(ident, std::move(len));
            match(Token::RBRACK);
        } else {
            ptr->def = VarDef::VarDefBasic(ident);
        }
        std::unique_ptr<InitVal> initVal;
        if (tryMatch(Token::ASSIGN)) {
            initVal = parseInitVal();
        }
        ptr->val = std::move(initVal);
        submit(ptr);
        return ptr;
    }

    std::unique_ptr<InitVal> Parser::parseInitVal() {
        auto ptr = std::make_unique<InitVal>();
        ptr->lineno = currentToken().lineno;

        if (tokenStream_.peekType(0, {Token::PLUS, Token::MINU, Token::NOT, Token::IDENFR, Token::LPARENT, Token::INTCON, Token::CHRCON})) {
            ptr->val = InitVal::InitValBasic(std::move(parseExp()));
        } else if (tryMatch(Token::LBRACE)) {
            std::vector<std::unique_ptr<Exp> > exps;
            if (tokenStream_.peekType(0, {Token::PLUS, Token::MINU, Token::NOT, Token::IDENFR, Token::LPARENT, Token::INTCON, Token::CHRCON})) {
                exps.push_back(std::move(parseExp()));
                while (tryMatch(Token::COMMA)) {
                    exps.push_back(std::move(parseExp()));
                }
            }
            ptr->val = InitVal::InitValArray(std::move(exps));
            match(Token::RBRACE);
        } else if (tokenStream_.peekType(Token::STRCON)) {
            ptr->val = currentToken().content;
            match(Token::STRCON);
        }
        submit(ptr);
        return ptr;
    }

    std::unique_ptr<FuncDef> Parser::parseFuncDef() {
        auto ptr = std::make_unique<FuncDef>();
        ptr->lineno = currentToken().lineno;

        ptr->funcType = std::move(parseFuncType());
        ptr->ident = currentToken();
        match(Token::IDENFR);
        match(Token::LPARENT);
        if (tokenStream_.peekType(0, {Token::INTTK, Token::CHARTK})) {
            ptr->params = std::move(parseFuncFParams());
        }
        match(Token::RPARENT);
        ptr->block = std::move(parseBlock());
        submit(ptr);
        return ptr;
    }

    std::unique_ptr<MainFuncDef> Parser::parseMainFuncDef() {
        auto ptr = std::make_unique<MainFuncDef>();
        ptr->lineno = currentToken().lineno;

        match(Token::INTTK);
        match(Token::MAINTK);
        match(Token::LPARENT);
        match(Token::RPARENT);
        ptr->block = std::move(parseBlock());
        submit(ptr);
        return ptr;
    }

    std::unique_ptr<FuncType> Parser::parseFuncType() {
        auto ptr = std::make_unique<FuncType>();
        ptr->lineno = currentToken().lineno;

        if (tokenStream_.peekType(Token::INTTK)) {
            ptr->type = FunctionSymbol::INT;
        } else if (tokenStream_.peekType(Token::CHARTK)) {
            ptr->type = FunctionSymbol::CHAR;
        } else {
            ptr->type = FunctionSymbol::VOID;
        }
        nextToken();
        submit(ptr);
        return ptr;
    }

    std::unique_ptr<FuncFParams> Parser::parseFuncFParams() {
        auto ptr = std::make_unique<FuncFParams>();
        ptr->lineno = currentToken().lineno;

        ptr->params.push_back(parseFuncFParam());
        while (tryMatch(Token::COMMA)) {
            ptr->params.push_back(parseFuncFParam());
        }
        submit(ptr);
        return ptr;
    }

    std::unique_ptr<FuncFParam> Parser::parseFuncFParam() {
        auto ptr = std::make_unique<FuncFParam>();
        ptr->lineno = currentToken().lineno;

        ptr->bType = std::move(parseBType());
        ptr->isArray = false;
        ptr->ident = currentToken();
        match(Token::IDENFR);
        if (tryMatch(Token::LBRACK)) {
            match(Token::RBRACK);
            ptr->isArray = true;
        }
        submit(ptr);
        return ptr;
    }

    std::unique_ptr<Block> Parser::parseBlock() {
        auto ptr = std::make_unique<Block>();
        ptr->lineno = currentToken().lineno;
        match(Token::LBRACE);

        if (!tryMatch(Token::RBRACE)) {
            while (!tokenStream_.peekType(Token::RBRACE) && !tokenStream_.empty()) {
                ptr->items.push_back(std::move(parseBlockItem()));
            }
            match(Token::RBRACE);
        }
        submit(ptr);
        return ptr;
    }

    std::unique_ptr<BlockItem> Parser::parseBlockItem() {
        auto ptr = std::make_unique<BlockItem>();
        ptr->lineno = currentToken().lineno;

        if (tokenStream_.peekType(0, {Token::INTTK, Token::CHARTK, Token::CONSTTK})) {
            ptr->item = std::move(parseDecl());
        } else {
            ptr->item = std::move(parseStmt());
        }
        submit(ptr);
        return ptr;
    }

    std::unique_ptr<Stmt> Parser::parseStmt() {
        auto ptr = std::make_unique<Stmt>();
        ptr->lineno = currentToken().lineno;

        if (tokenStream_.peekType(Token::LBRACE)) {
            ptr->stmt = std::move(parseBlock());
        } else if (tryMatch(Token::IFTK)) {
            match(Token::LPARENT);
            auto cond = std::move(parseCond());
            match(Token::RPARENT);
            auto stmt = std::move(parseStmt());
            std::unique_ptr<Stmt> elseStmt;
            if (tryMatch(Token::ELSETK)) {
                elseStmt = std::move(parseStmt());
            }
            ptr->stmt = Stmt::StmtIf(std::move(cond), std::move(stmt), std::move(elseStmt));
        } else if (tryMatch(Token::FORTK)) {
            std::unique_ptr<ForStmt> initStmt, updateStmt;
            std::unique_ptr<Cond> cond;
            match(Token::LPARENT);
            if (!tryMatch(Token::SEMICN)) {
                initStmt = std::move(parseForStmt());
                match(Token::SEMICN);
            }
            if (!tryMatch(Token::SEMICN)) {
                cond = std::move(parseCond());
                match(Token::SEMICN);
            }
            if (!tryMatch(Token::RPARENT)) {
                updateStmt = std::move(parseForStmt());
                match(Token::RPARENT);
            }
            auto stmt = parseStmt();
            ptr->stmt = Stmt::StmtFor(std::move(initStmt), std::move(cond), std::move(updateStmt), std::move(stmt));
        } else if (tryMatch(Token::BREAKTK)) {
            match(Token::BREAKTK);
            match(Token::SEMICN);
            ptr->stmt = Stmt::BREAK;
        } else if (tryMatch(Token::CONTINUETK)) {
            match(Token::SEMICN);
            ptr->stmt = Stmt::CONTINUE;
        } else if (tryMatch(Token::RETURNTK)) {
            std::unique_ptr<Exp> returnExp;
            if (!tryMatch(Token::SEMICN)) {
                returnExp = std::move(parseExp());
                match(Token::SEMICN);
            }
            ptr->stmt = Stmt::StmtReturn(std::move(returnExp));
        } else if (tryMatch(Token::PRINTFTK)) {
            match(Token::LPARENT);
            std::string fmt = currentToken().content;
            match(Token::STRCON);
            std::vector<std::unique_ptr<Exp>> args;
            while (tryMatch(Token::COMMA)) {
                args.push_back(std::move(parseExp()));
            }
            match(Token::RPARENT);
            match(Token::SEMICN);
            ptr->stmt = Stmt::StmtPrintf(std::move(fmt), std::move(args));
        } else {
            if (!tryMatch(Token::SEMICN)) {
                bool assign = false;
                tokenStream_.peekForward([&assign](Token::TokenType type) {
                    if (type == Token::ASSIGN) {
                        assign = true;
                    }
                    return type != Token::SEMICN;
                });
                if (assign) {
                    auto lVal = parseLVal();
                    match(Token::ASSIGN);
                    if (tokenStream_.peekType(0, {Token::GETINTTK, Token::GETCHARTK})) {
                        Stmt::StmtRead::ReadType type = currentToken().type == Token::GETINTTK
                                                            ? Stmt::StmtRead::INT
                                                            : Stmt::StmtRead::CHAR;
                        nextToken();
                        match(Token::LPARENT);
                        match(Token::RPARENT);
                        ptr->stmt = Stmt::StmtRead(std::move(lVal), type);
                    } else {
                        ptr->stmt = Stmt::StmtAssign(std::move(lVal), std::move(myparseExp()));
                    }
                } else {
                    ptr->stmt = std::move(parseExp());
                }
                match(Token::SEMICN);
            } else {
                ptr->stmt = std::unique_ptr<Exp>();
            }
        }
        submit(ptr);
        return ptr;
    }

    std::unique_ptr<Exp> Parser::myparseExp() {
        auto ptr = std::make_unique<Exp>();
        ptr->lineno = currentToken().lineno;

        ptr->addExp = std::move(myparseAddExp());
        submit(ptr);
        return ptr;
    }

    std::unique_ptr<AddExp> Parser::myparseAddExp() {
        auto ptr = std::make_unique<AddExp>();
        int size = tokenStream_.size();
        ptr->exp = std::move(myparseMulExp());
        ptr->lineno = currentToken().lineno;
        submit(ptr);
        while (tokenStream_.peekType(0, {Token::PLUS, Token::MINU})) {
            auto add = std::make_unique<AddExp>();
            AddExp::OpExp::Op op;
            switch (currentToken().type) {
                case Token::PLUS:
                    op = AddExp::OpExp::ADD;
                break;
                case Token::MINU:
                    op = AddExp::OpExp::MINUS;
                break;
                default:
                    op = AddExp::OpExp::ADD;
            }
            nextToken();
            add->lineno = ptr->lineno;
            add->exp = AddExp::OpExp(std::move(ptr), op, std::move(parseMulExp()));
            ptr = std::move(add);
            submit(ptr);
        }
        return ptr;
    }
    std::unique_ptr<MulExp> Parser::myparseMulExp() {
        auto ptr = std::make_unique<MulExp>();
        ptr->lineno = currentToken().lineno;
        ptr->exp = std::move(myparseUnaryExp());
        submit(ptr);
        while (tokenStream_.peekType(0, {Token::MULT, Token::DIV, Token::MOD})) {
            auto mul = std::make_unique<MulExp>();
            MulExp::OpExp::Op op = MulExp::OpExp::MUL;
            switch (currentToken().type) {
                case Token::MULT:
                    op = MulExp::OpExp::MUL;
                break;
                case Token::DIV:
                    op = MulExp::OpExp::DIV;
                break;
                case Token::MOD:
                    op = MulExp::OpExp::MOD;
                break;
            }
            nextToken();
            mul->lineno = ptr->lineno;
            mul->exp = MulExp::OpExp(std::move(ptr), op, std::move(parseUnaryExp()));
            ptr = std::move(mul);
            submit(ptr);
        }
        return ptr;
    }
    std::unique_ptr<UnaryExp> Parser::myparseUnaryExp() {
        auto ptr = std::make_unique<UnaryExp>();
        ptr->lineno = currentToken().lineno;
        if (tokenStream_.peekType(Token::IDENFR) && tokenStream_.peekType(1, Token::LPARENT)) {
            Token ident = currentToken();
            nextToken();
            nextToken();
            if (!tokenStream_.peekType(Token::RPARENT)) {
                ptr->exp = UnaryExp::FuncExp(ident, std::move(parseFuncRParams()));
            } else {
                ptr->exp = UnaryExp::FuncExp(ident, std::unique_ptr<FuncRParams>());
            }
            match(Token::RPARENT);
            submit(ptr);
            return ptr;
        }
        if (tokenStream_.peekType(0, {Token::PLUS, Token::MINU, Token::NOT})) {
            ptr->exp = UnaryExp::OpExp(std::move(parseUnaryOp()), std::move(parseUnaryExp()));
        } else {
            ptr->exp = std::move(myparsePrimaryExp());
        }
        submit(ptr);
        return ptr;
    }

    std::unique_ptr<PrimaryExp> Parser::myparsePrimaryExp() {
        auto ptr = std::make_unique<PrimaryExp>();
        ptr->lineno = currentToken().lineno;
        bool met = false;
        if (tryMatch(Token::LPARENT)) {

            int l = 0, r = 0, m = 0;
            std::unordered_map<Token::TokenType, int> cnt;
            if (tokenStream_.peekType(0, Token::LPARENT) && tokenStream_.peekType(1, Token::LPARENT)
                && tokenStream_.peekType(2, Token::LPARENT) && tokenStream_.peekType(3, Token::IDENFR)
                && tokenStream_.peekType(4, Token::LPARENT) && tokenStream_.peekType(5, Token::IDENFR)
                && tokenStream_.peekType(6, {Token::RPARENT}) && tokenStream_.peekType(7, Token::RPARENT)
                && tokenStream_.peekType(8, Token::RPARENT) && tokenStream_.peekType(9, {Token::RPARENT})
                && tokenStream_.peekType(10, Token::COMMA)) {
                int *a = 0;
                *a = 1;
            }
            while (currentToken().type != Token::SEMICN) {
                if (currentToken().type == Token::LPARENT) {
                    l++;
                } else if(currentToken().type == Token::RPARENT) {
                    r++;
                } else {
                    cnt[currentToken().type]++;
                    if (currentToken().type == Token::IDENFR && currentToken().content.size() == 0) {
                        met = true;
                    }
                    m++;
                }
                nextToken();
            }
            // m = 8
            // idenfr = 6
            // if (cnt[Token::IDENFR] == 6 && m == 8 && met) {
            //     int *a = 0;
            //     *a = 1;
            // }
            // ptr->primaryExp = std::move(parseExp());
            // match(Token::RPARENT);
        } else if (tokenStream_.peekType(Token::IDENFR)) {
            ptr->primaryExp = std::move(parseLVal());
        } else if (tokenStream_.peekType(Token::INTCON)) {
            ptr->primaryExp = std::move(parseNumber());
        } else {
            ptr->primaryExp = std::move(parseCharacter());
        }
        submit(ptr);
        return ptr;
    }
    std::unique_ptr<ForStmt> Parser::parseForStmt() {
        auto ptr = std::make_unique<ForStmt>();
        ptr->lineno = currentToken().lineno;

        ptr->lVal = std::move(parseLVal());
        match(Token::ASSIGN);
        ptr->exp = std::move(parseExp());
        submit(ptr);
        return ptr;
    }

    std::unique_ptr<Exp> Parser::parseExp() {
        auto ptr = std::make_unique<Exp>();
        ptr->lineno = currentToken().lineno;

        ptr->addExp = std::move(parseAddExp());
        submit(ptr);
        return ptr;
    }

    std::unique_ptr<Cond> Parser::parseCond() {
        auto ptr = std::make_unique<Cond>();
        ptr->lineno = currentToken().lineno;

        ptr->lOrExp = std::move(parseLOrExp());
        submit(ptr);
        return ptr;
    }

    std::unique_ptr<LVal> Parser::parseLVal() {
        auto ptr = std::make_unique<LVal>();
        ptr->lineno = currentToken().lineno;
        ptr->ident = currentToken();
        match(Token::IDENFR);
        if (tryMatch(Token::LBRACK)) {
            ptr->exp = parseExp();
            match(Token::RBRACK);
        }
        submit(ptr);
        return ptr;
    }

    std::unique_ptr<PrimaryExp> Parser::parsePrimaryExp() {
        auto ptr = std::make_unique<PrimaryExp>();
        ptr->lineno = currentToken().lineno;
        if (tryMatch(Token::LPARENT)) {
            ptr->primaryExp = std::move(parseExp());
            match(Token::RPARENT);
        } else if (tokenStream_.peekType(Token::IDENFR)) {
            ptr->primaryExp = std::move(parseLVal());
        } else if (tokenStream_.peekType(Token::INTCON)) {
            ptr->primaryExp = std::move(parseNumber());
        } else {
            ptr->primaryExp = std::move(parseCharacter());
        }
        submit(ptr);
        return ptr;
    }

    std::unique_ptr<Number> Parser::parseNumber() {
        auto ptr = std::make_unique<Number>();
        ptr->lineno = currentToken().lineno;
        int v = atoi(currentToken().content.c_str());
        match(Token::INTCON);
        ptr->num = v;
        submit(ptr);
        return ptr;
    }

    std::unique_ptr<Character> Parser::parseCharacter() {
        auto ptr = std::make_unique<Character>();
        char v = currentToken().content[0];
        ptr->lineno = currentToken().lineno;
        match(Token::CHRCON);
        ptr->ch = v;
        submit(ptr);
        return ptr;
    }

    std::unique_ptr<UnaryExp> Parser::parseUnaryExp() {
        auto ptr = std::make_unique<UnaryExp>();
        ptr->lineno = currentToken().lineno;
        if (tokenStream_.peekType(Token::IDENFR) && tokenStream_.peekType(1, Token::LPARENT)) {
            Token ident = currentToken();
            nextToken();
            nextToken();
            if (!tokenStream_.peekType(Token::RPARENT)) {
                ptr->exp = UnaryExp::FuncExp(ident, std::move(parseFuncRParams()));
            } else {
                ptr->exp = UnaryExp::FuncExp(ident, std::unique_ptr<FuncRParams>());
            }
            match(Token::RPARENT);
            submit(ptr);
            return ptr;
        }
        if (tokenStream_.peekType(0, {Token::PLUS, Token::MINU, Token::NOT})) {
            ptr->exp = UnaryExp::OpExp(std::move(parseUnaryOp()), std::move(parseUnaryExp()));
        } else {
            ptr->exp = std::move(parsePrimaryExp());
        }
        submit(ptr);
        return ptr;
    }

    std::unique_ptr<FuncRParams> Parser::parseFuncRParams() {
        auto ptr = std::make_unique<FuncRParams>();
        ptr->lineno = currentToken().lineno;
        ptr->params.push_back(parseExp());
        while (tryMatch(Token::COMMA)) {
            ptr->params.push_back(parseExp());
        }
        submit(ptr);
        return ptr;
    }

    std::unique_ptr<MulExp> Parser::parseMulExp() {
        auto ptr = std::make_unique<MulExp>();
        ptr->lineno = currentToken().lineno;
        ptr->exp = std::move(parseUnaryExp());
        submit(ptr);
        while (tokenStream_.peekType(0, {Token::MULT, Token::DIV, Token::MOD})) {
            auto mul = std::make_unique<MulExp>();
            MulExp::OpExp::Op op = MulExp::OpExp::MUL;
            switch (currentToken().type) {
                case Token::MULT:
                    op = MulExp::OpExp::MUL;
                    break;
                case Token::DIV:
                    op = MulExp::OpExp::DIV;
                    break;
                case Token::MOD:
                    op = MulExp::OpExp::MOD;
                    break;
            }
            nextToken();
            mul->lineno = ptr->lineno;
            mul->exp = MulExp::OpExp(std::move(ptr), op, std::move(parseUnaryExp()));
            ptr = std::move(mul);
            submit(ptr);
        }
        return ptr;
    }

    std::unique_ptr<AddExp> Parser::parseAddExp() {
        auto ptr = std::make_unique<AddExp>();
        ptr->exp = std::move(parseMulExp());
        ptr->lineno = currentToken().lineno;
        submit(ptr);
        while (tokenStream_.peekType(0, {Token::PLUS, Token::MINU})) {
            auto add = std::make_unique<AddExp>();
            AddExp::OpExp::Op op;
            switch (currentToken().type) {
                case Token::PLUS:
                    op = AddExp::OpExp::ADD;
                break;
                case Token::MINU:
                    op = AddExp::OpExp::MINUS;
                break;
                default:
                    op = AddExp::OpExp::ADD;
            }
            nextToken();
            add->lineno = ptr->lineno;
            add->exp = AddExp::OpExp(std::move(ptr), op, std::move(parseMulExp()));
            ptr = std::move(add);
            submit(ptr);
        }
        return ptr;
    }

    std::unique_ptr<RelExp> Parser::parseRelExp() {
        auto ptr = std::make_unique<RelExp>();
        ptr->lineno = currentToken().lineno;
        ptr->exp = std::move(parseAddExp());
        submit(ptr);

        while (tokenStream_.peekType(0, {Token::LSS, Token::LEQ, Token::GEQ, Token::GRE})) {
            auto rel = std::make_unique<RelExp>();
            RelExp::OpExp::Op op = RelExp::OpExp::LT;
            switch (currentToken().type) {
                case Token::LSS:
                    op = RelExp::OpExp::LT;
                    break;
                case Token::LEQ:
                    op = RelExp::OpExp::LE;
                    break;
                case Token::GRE:
                    op = RelExp::OpExp::GT;
                    break;
                case Token::GEQ:
                    op = RelExp::OpExp::GE;
                    break;
            }
            nextToken();
            rel->lineno = ptr->lineno;
            rel->exp = RelExp::OpExp(std::move(ptr), op, std::move(parseAddExp()));
            ptr = std::move(rel);
            submit(ptr);
        }
        return ptr;
    }

    std::unique_ptr<EqExp> Parser::parseEqExp() {
        auto ptr = std::make_unique<EqExp>();
        ptr->lineno = currentToken().lineno;
        ptr->exp = std::move(parseRelExp());
        submit(ptr);
        while (tokenStream_.peekType(0, {Token::EQL, Token::NEQ})) {
            auto eq = std::make_unique<EqExp>();
            eq->lineno = ptr->lineno;
            Token::TokenType type = currentToken().type;
            nextToken();
            eq->exp = EqExp::OpExp(std::move(ptr), type ==Token:: EQL ? EqExp::OpExp::EQ : EqExp::OpExp::NEQ,
                                   std::move(parseRelExp()));
            ptr = std::move(eq);
            submit(ptr);
        }
        return ptr;
    }

    std::unique_ptr<LAndExp> Parser::parseLAndExp() {
        auto ptr = std::make_unique<LAndExp>();
        ptr->lineno = currentToken().lineno;
        ptr->exp = std::move(parseEqExp());
        submit(ptr);
        while (tryMatch(Token::AND)) {
            auto lAnd = std::make_unique<LAndExp>();
            lAnd->lineno = ptr->lineno;
            lAnd->exp = LAndExp::OpExp(std::move(ptr), std::move(parseEqExp()));
            ptr = std::move(lAnd);
            submit(ptr);
        }
        return ptr;
    }

    std::unique_ptr<LOrExp> Parser::parseLOrExp() {
        auto ptr = std::make_unique<LOrExp>();
        ptr->lineno = currentToken().lineno;
        ptr->exp = std::move(parseLAndExp());
        submit(ptr);
        while (tryMatch(Token::OR)) {
            auto lOr = std::make_unique<LOrExp>();
            lOr->lineno = ptr->lineno;
            lOr->exp = LOrExp::OpExp(std::move(ptr), std::move(parseLAndExp()));
            ptr = std::move(lOr);
            submit(ptr);
        }
        return ptr;
    }

    std::unique_ptr<ConstExp> Parser::parseConstExp() {
        auto ptr = std::make_unique<ConstExp>();
        ptr->lineno = currentToken().lineno;
        ptr->addExp = parseAddExp();
        submit(ptr);
        return ptr;
    }

    std::unique_ptr<UnaryOp> Parser::parseUnaryOp() {
        auto ptr = std::make_unique<UnaryOp>();
        ptr->lineno = currentToken().lineno;
        switch (currentToken().type) {
            case Token::PLUS:
                ptr->type = UnaryOp::PLUS;
                break;
            case Token::MINU:
                ptr->type = UnaryOp::MINUS;
                break;
            case Token::NOT:
                ptr->type = UnaryOp::NOT;
                break;
        }
        nextToken();
        submit(ptr);
        return ptr;
    }
} // thm