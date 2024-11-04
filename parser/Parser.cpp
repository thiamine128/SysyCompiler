//
// Created by slty5 on 24-9-27.
//

#include "Parser.h"

#include <cmath>
#include <iostream>
#include <fstream>


namespace thm {
    Parser::Parser(TokenStream &tokenStream, ErrorReporter& errorReporter) : tokenStream_(tokenStream), errorReporter_(errorReporter), currentLine_(0) {

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

    CompUnit* Parser::parseCompUnit() {
        auto ptr = new CompUnit();
        ptr->lineno = currentToken().lineno;



        while (true) {
            if (tokenStream_.peekType(Token::CONSTTK) ||
                (tokenStream_.peekType(0, {Token::INTTK, Token::CHARTK}) && tokenStream_.peekType(1, Token::IDENFR) && !tokenStream_.peekType(2, Token::LPARENT))) {
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

    Decl* Parser::parseDecl() {
        auto ptr = new Decl();
        ptr->lineno = currentToken().lineno;

        if (tokenStream_.peekType(Token::CONSTTK)) ptr->decl = parseConstDecl();
        else ptr->decl = parseVarDecl();
        submit(ptr);
        return ptr;
    }

    ConstDecl* Parser::parseConstDecl() {
        auto ptr = new ConstDecl();
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

    BType* Parser::parseBType() {
        auto ptr = new BType();
        ptr->lineno = currentToken().lineno;
        ptr->type = currentToken().type == Token::INTTK ? VariableType::INT : VariableType::CHAR;
        nextToken();
        submit(ptr);
        return ptr;
    }


    ConstDef* Parser::parseConstDef() {
        auto ptr = new ConstDef();
        ptr->lineno = currentToken().lineno;
        ptr->ident = currentToken();
        match(Token::IDENFR);
        if (tryMatch(Token::LBRACK)) {
            ConstExp* len = nullptr;
            if (!tryMatch(Token::RBRACK)) {
                len = parseConstExp();
            }
            ptr->def = ConstDef::ConstDefArray(len);
            match(Token::RBRACK);
        } else {
            ptr->def = ConstDef::ConstDefBasic();
        }
        match(Token::ASSIGN);
        ptr->val = parseConstInitVal();
        submit(ptr);
        return ptr;
    }

    ConstInitVal* Parser::parseConstInitVal() {
        auto ptr = new ConstInitVal();
        ptr->lineno = currentToken().lineno;

        if (tokenStream_.peekType(0, {Token::PLUS, Token::MINU, Token::NOT, Token::IDENFR, Token::LPARENT, Token::INTCON, Token::CHRCON})) {
            ptr->val = ConstInitVal::ConstInitValBasic(parseConstExp());
        } else if (tryMatch(Token::LBRACE)) {
            std::vector<ConstExp* > exps;
            if (tokenStream_.peekType(0, {Token::PLUS, Token::MINU, Token::NOT, Token::IDENFR, Token::LPARENT, Token::INTCON, Token::CHRCON})) {
                exps.push_back(parseConstExp());
                while (tryMatch(Token::COMMA)) {
                    exps.push_back(parseConstExp());
                }
            }
            ptr->val = ConstInitVal::ConstInitValArray(exps);
            match(Token::RBRACE);
        } else if (tokenStream_.peekType(Token::STRCON)) {
            ptr->val = currentToken().content;
            match(Token::STRCON);
        }
        submit(ptr);
        return ptr;
    }

    VarDecl* Parser::parseVarDecl() {
        auto ptr = new VarDecl();
        ptr->lineno = currentToken().lineno;

        ptr->bType = parseBType();
        ptr->varDefs.push_back(parseVarDef());
        while (tryMatch(Token::COMMA)) {
            ptr->varDefs.push_back(parseVarDef());
        }
        match(Token::SEMICN);
        submit(ptr);
        return ptr;
    }

    VarDef* Parser::parseVarDef() {
        auto ptr = new VarDef();
        ptr->lineno = currentToken().lineno;

        ptr->ident = currentToken();
        match(Token::IDENFR);

        if (tryMatch(Token::LBRACK)) {
            ConstExp* len = nullptr;
            if (!tokenStream_.peekType(Token::RBRACK)) {
                len = parseConstExp();
            }
            ptr->def = VarDef::VarDefArray(len);
            match(Token::RBRACK);
        } else {
            ptr->def = VarDef::VarDefBasic();
        }
        InitVal* initVal = nullptr;
        if (tryMatch(Token::ASSIGN)) {
            initVal = parseInitVal();
        }
        ptr->val = initVal;
        submit(ptr);
        return ptr;
    }

    InitVal* Parser::parseInitVal() {
        auto ptr = new InitVal();
        ptr->lineno = currentToken().lineno;

        if (tokenStream_.peekType(0, {Token::PLUS, Token::MINU, Token::NOT, Token::IDENFR, Token::LPARENT, Token::INTCON, Token::CHRCON})) {
            ptr->val = InitVal::InitValBasic(parseExp());
        } else if (tryMatch(Token::LBRACE)) {
            std::vector<Exp* > exps;
            if (tokenStream_.peekType(0, {Token::PLUS, Token::MINU, Token::NOT, Token::IDENFR, Token::LPARENT, Token::INTCON, Token::CHRCON})) {
                exps.push_back(parseExp());
                while (tryMatch(Token::COMMA)) {
                    exps.push_back(parseExp());
                }
            }
            ptr->val = InitVal::InitValArray(exps);
            match(Token::RBRACE);
        } else if (tokenStream_.peekType(Token::STRCON)) {
            ptr->val = currentToken().content;
            match(Token::STRCON);
        }
        submit(ptr);
        return ptr;
    }

    FuncDef* Parser::parseFuncDef() {
        auto ptr = new FuncDef();
        ptr->lineno = currentToken().lineno;

        ptr->funcType = parseFuncType();
        ptr->ident = currentToken();
        match(Token::IDENFR);
        match(Token::LPARENT);
        if (tokenStream_.peekType(0, {Token::INTTK, Token::CHARTK})) {
            ptr->params = parseFuncFParams();
        }
        match(Token::RPARENT);
        ptr->block = parseBlock();
        submit(ptr);
        return ptr;
    }

    MainFuncDef* Parser::parseMainFuncDef() {
        auto ptr = new MainFuncDef();
        ptr->lineno = currentToken().lineno;

        match(Token::INTTK);
        match(Token::MAINTK);
        match(Token::LPARENT);
        match(Token::RPARENT);
        ptr->block = parseBlock();
        submit(ptr);
        return ptr;
    }

    FuncType* Parser::parseFuncType() {
        auto ptr = new FuncType();
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

    FuncFParams* Parser::parseFuncFParams() {
        auto ptr = new FuncFParams();
        ptr->lineno = currentToken().lineno;

        ptr->params.push_back(parseFuncFParam());
        while (tryMatch(Token::COMMA)) {
            ptr->params.push_back(parseFuncFParam());
        }
        submit(ptr);
        return ptr;
    }

    FuncFParam* Parser::parseFuncFParam() {
        auto ptr = new FuncFParam();
        ptr->lineno = currentToken().lineno;

        ptr->bType = parseBType();
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

    Block* Parser::parseBlock() {
        auto ptr = new Block();
        ptr->lineno = currentToken().lineno;
        match(Token::LBRACE);

        if (!tokenStream_.peekType(Token::RBRACE)) {
            while (!tokenStream_.peekType(Token::RBRACE) && !tokenStream_.empty()) {
                ptr->items.push_back(parseBlockItem());
            }
            ptr->rBrace = currentToken();
            match(Token::RBRACE);
        } else {
            ptr->rBrace = currentToken();
            nextToken();
        }
        submit(ptr);
        return ptr;
    }

    BlockItem* Parser::parseBlockItem() {
        auto ptr = new BlockItem();
        ptr->lineno = currentToken().lineno;

        if (tokenStream_.peekType(0, {Token::INTTK, Token::CHARTK, Token::CONSTTK})) {
            ptr->item = parseDecl();
        } else {
            ptr->item = parseStmt();
        }
        submit(ptr);
        return ptr;
    }

    Stmt* Parser::parseStmt() {
        auto ptr = new Stmt();
        ptr->lineno = currentToken().lineno;

        if (tokenStream_.peekType(Token::LBRACE)) {
            ptr->stmt = parseBlock();
        } else if (tryMatch(Token::IFTK)) {
            match(Token::LPARENT);
            auto cond = parseCond();
            match(Token::RPARENT);
            auto stmt = parseStmt();
            Stmt* elseStmt = nullptr;
            if (tryMatch(Token::ELSETK)) {
                elseStmt = parseStmt();
            }
            ptr->stmt = Stmt::StmtIf(cond, stmt, elseStmt);
        } else if (tryMatch(Token::FORTK)) {
            ForStmt *initStmt = nullptr, *updateStmt = nullptr;
            Cond* cond = nullptr;
            match(Token::LPARENT);
            if (!tryMatch(Token::SEMICN)) {
                initStmt = parseForStmt();
                match(Token::SEMICN);
            }
            if (!tryMatch(Token::SEMICN)) {
                cond = parseCond();
                match(Token::SEMICN);
            }
            if (!tryMatch(Token::RPARENT)) {
                updateStmt = parseForStmt();
                match(Token::RPARENT);
            }
            auto stmt = parseStmt();
            ptr->stmt = Stmt::StmtFor(initStmt, cond, updateStmt, stmt);
        } else if (tryMatch(Token::BREAKTK)) {
            match(Token::BREAKTK);
            match(Token::SEMICN);
            ptr->stmt = Stmt::BREAK;
        } else if (tryMatch(Token::CONTINUETK)) {
            match(Token::SEMICN);
            ptr->stmt = Stmt::CONTINUE;
        } else if (tryMatch(Token::RETURNTK)) {
            Exp* returnExp = nullptr;
            if (stepExp(0)) {
                returnExp = parseExp();
                match(Token::SEMICN);
            } else {
                match(Token::SEMICN);
            }
            ptr->stmt = Stmt::StmtReturn(returnExp);
        } else if (tokenStream_.peekType(Token::PRINTFTK)) {
            Token printfTk = currentToken();
            nextToken();
            match(Token::LPARENT);
            std::string fmt = currentToken().content;
            match(Token::STRCON);
            std::vector<Exp*> args;
            while (tryMatch(Token::COMMA)) {
                args.push_back(parseExp());
            }
            match(Token::RPARENT);
            match(Token::SEMICN);
            ptr->stmt = Stmt::StmtPrintf(fmt, args, printfTk);
        } else {
            if (!tryMatch(Token::SEMICN)) {
                int offset = stepExp(0);
                if (tokenStream_.peekType(offset, Token::ASSIGN)) {
                    int size = tokenStream_.size();
                    auto lVal = parseLVal();
                    if (size == tokenStream_.size()) {
                        while (currentToken().type != Token::SEMICN) nextToken();
                        match(Token::SEMICN);
                        return ptr;
                    }
                    match(Token::ASSIGN);
                    if (tokenStream_.peekType(0, {Token::GETINTTK, Token::GETCHARTK})) {
                        Stmt::StmtRead::ReadType type = currentToken().type == Token::GETINTTK
                                                            ? Stmt::StmtRead::INT
                                                            : Stmt::StmtRead::CHAR;
                        nextToken();
                        match(Token::LPARENT);
                        match(Token::RPARENT);
                        ptr->stmt = Stmt::StmtRead(lVal, type);
                    } else {
                        ptr->stmt = Stmt::StmtAssign(lVal, parseExp());
                    }
                } else {
                    ptr->stmt = parseExp();
                }
                match(Token::SEMICN);
            } else {
                ptr->stmt = static_cast<Exp*>(nullptr);
            }
        }
        submit(ptr);
        return ptr;
    }

    ForStmt* Parser::parseForStmt() {
        auto ptr = new ForStmt();
        ptr->lineno = currentToken().lineno;

        ptr->lVal = parseLVal();
        match(Token::ASSIGN);
        ptr->exp = parseExp();
        submit(ptr);
        return ptr;
    }

    Exp* Parser::parseExp() {
        auto ptr = new Exp();
        ptr->len = stepExp(0);
        ptr->lineno = currentToken().lineno;

        ptr->addExp = parseAddExp();
        submit(ptr);
        return ptr;
    }

    int Parser::stepExp(int offset) {
        return stepAddExp(offset);
    }

    Cond* Parser::parseCond() {
        auto ptr = new Cond();
        ptr->lineno = currentToken().lineno;

        ptr->lOrExp = parseLOrExp();
        submit(ptr);
        return ptr;
    }

    LVal* Parser::parseLVal() {
        auto ptr = new LVal();
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

    int Parser::stepLVal(int offset) {
        if (tokenStream_.peekType(offset, Token::IDENFR))
            offset += 1;
        if (tokenStream_.peekType(offset, Token::LBRACK)) {
            offset += 1;
            offset = stepExp(offset);
            if (tokenStream_.peekType(offset, Token::RBRACK))
                offset += 1;
        }
        return offset;
    }

    PrimaryExp* Parser::parsePrimaryExp() {
        auto ptr = new PrimaryExp();
        ptr->lineno = currentToken().lineno;
        if (tryMatch(Token::LPARENT)) {
            ptr->primaryExp = parseExp();
            match(Token::RPARENT);
        } else if (tokenStream_.peekType(Token::IDENFR)) {
            ptr->primaryExp = parseLVal();
        } else if (tokenStream_.peekType(Token::INTCON)) {
            ptr->primaryExp = parseNumber();
        } else {
            ptr->primaryExp = parseCharacter();
        }
        submit(ptr);
        return ptr;
    }

    int Parser::stepPrimaryExp(int offset) {
        if (tokenStream_.peekType(offset, Token::LPARENT)) {
            offset += 1;
            offset = stepExp(offset);
            if (tokenStream_.peekType(offset, Token::RPARENT))
                offset += 1;
        } else if (tokenStream_.peekType(offset, Token::IDENFR)) {
            offset = stepLVal(offset);
        } else if (tokenStream_.peekType(offset, Token::INTCON)) {
            offset = stepNumber(offset);
        } else {
            offset = stepCharacter(offset);
        }
        return offset;
    }

    Number* Parser::parseNumber() {
        auto ptr = new Number();
        ptr->lineno = currentToken().lineno;
        int v = atoi(currentToken().content.c_str());
        match(Token::INTCON);
        ptr->num = v;
        submit(ptr);
        return ptr;
    }

    int Parser::stepNumber(int offset) const {
        if (tokenStream_.peekType(offset, Token::INTCON))
            offset += 1;
        return offset;
    }

    Character* Parser::parseCharacter() {
        auto ptr = new Character();
        char v;
        if (currentToken().content[1] != '\\')
            v = currentToken().content[1];
        else {
            int *a = 0;
            *a = 1;
            v = currentToken().content[2];
        }
        ptr->lineno = currentToken().lineno;
        match(Token::CHRCON);
        ptr->ch = v;
        submit(ptr);
        return ptr;
    }

    int Parser::stepCharacter(int offset) const {
        if (tokenStream_.peekType(offset, Token::CHRCON))
            offset += 1;
        return offset;
    }

    UnaryExp* Parser::parseUnaryExp() {
        auto ptr = new UnaryExp();
        ptr->lineno = currentToken().lineno;
        if (tokenStream_.peekType(Token::IDENFR) && tokenStream_.peekType(1, Token::LPARENT)) {
            Token ident = currentToken();
            nextToken();
            nextToken();
            if (stepExp(0)) {
                ptr->exp = UnaryExp::FuncExp(ident, parseFuncRParams());
            } else {
                ptr->exp = UnaryExp::FuncExp(ident, nullptr);
            }
            match(Token::RPARENT);
            submit(ptr);
            return ptr;
        }
        if (tokenStream_.peekType(0, {Token::PLUS, Token::MINU, Token::NOT})) {
            auto op = parseUnaryOp();
            ptr->exp = UnaryExp::OpExp(op, parseUnaryExp());
        } else {
            ptr->exp = parsePrimaryExp();
        }
        submit(ptr);
        return ptr;
    }

    int Parser::stepUnaryExp(int offset) {
        if (tokenStream_.peekType(offset, Token::IDENFR) && tokenStream_.peekType(offset + 1, Token::LPARENT)) {
            offset += 2;
            if (!tokenStream_.peekType(offset, Token::RPARENT)) {
                offset = stepFuncRParams(offset);
            }
            if (tokenStream_.peekType(offset, Token::RPARENT))
                offset += 1;
            return offset;
        }
        if (tokenStream_.peekType(offset, {Token::PLUS, Token::MINU, Token::NOT})) {
            offset = stepUnaryOp(offset);
            offset = stepUnaryExp(offset);
        } else {
            offset = stepPrimaryExp(offset);
        }
        return offset;
    }

    FuncRParams* Parser::parseFuncRParams() {
        auto ptr = new FuncRParams();
        ptr->lineno = currentToken().lineno;
        ptr->params.push_back(parseExp());
        while (tryMatch(Token::COMMA)) {
            ptr->params.push_back(parseExp());
        }
        submit(ptr);
        return ptr;
    }

    int Parser::stepFuncRParams(int offset) {
        offset = stepExp(offset);
        while (tokenStream_.peekType(offset, Token::COMMA)) {
            offset += 1;
            offset = stepExp(offset);
        }
        return offset;
    }

    MulExp* Parser::parseMulExp() {
        auto ptr = new MulExp();
        ptr->lineno = currentToken().lineno;
        ptr->exp = parseUnaryExp();
        submit(ptr);
        while (tokenStream_.peekType(0, {Token::MULT, Token::DIV, Token::MOD})) {
            MulExp* mul = new MulExp();
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
                default: ;
            }
            nextToken();
            mul->lineno = ptr->lineno;
            mul->exp = MulExp::OpExp(ptr, op, parseUnaryExp());
            ptr = mul;
            submit(ptr);
        }
        return ptr;
    }

    int Parser::stepMulExp(int offset) {
        offset = stepUnaryExp(offset);
        while (tokenStream_.peekType(offset, {Token::MULT, Token::DIV, Token::MOD})) {
            offset += 1;
            offset = stepUnaryExp(offset);
        }
        return offset;
    }

    AddExp* Parser::parseAddExp() {
        auto ptr = new AddExp();
        ptr->exp = parseMulExp();
        ptr->lineno = currentToken().lineno;
        submit(ptr);
        while (tokenStream_.peekType(0, {Token::PLUS, Token::MINU})) {
            auto add = new AddExp();
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
            add->exp = AddExp::OpExp(ptr, op, parseMulExp());
            ptr = add;
            submit(ptr);
        }
        return ptr;
    }

    int Parser::stepAddExp(int offset) {
        offset = stepMulExp(offset);
        while (tokenStream_.peekType(offset, {Token::PLUS, Token::MINU})) {
            offset += 1;
            offset = stepMulExp(offset);
        }
        return offset;
    }

    RelExp* Parser::parseRelExp() {
        auto ptr = new RelExp();
        ptr->lineno = currentToken().lineno;
        ptr->exp = parseAddExp();
        submit(ptr);

        while (tokenStream_.peekType(0, {Token::LSS, Token::LEQ, Token::GEQ, Token::GRE})) {
            auto rel = new RelExp();
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
                default: ;
            }
            nextToken();
            rel->lineno = ptr->lineno;
            rel->exp = RelExp::OpExp(ptr, op, parseAddExp());
            ptr = rel;
            submit(ptr);
        }
        return ptr;
    }

    EqExp* Parser::parseEqExp() {
        auto ptr = new EqExp();
        ptr->lineno = currentToken().lineno;
        ptr->exp = parseRelExp();
        submit(ptr);
        while (tokenStream_.peekType(0, {Token::EQL, Token::NEQ})) {
            auto eq = new EqExp();
            eq->lineno = ptr->lineno;
            Token::TokenType type = currentToken().type;
            nextToken();
            eq->exp = EqExp::OpExp(ptr, type ==Token:: EQL ? EqExp::OpExp::EQ : EqExp::OpExp::NEQ,
                                   parseRelExp());
            ptr = eq;
            submit(ptr);
        }
        return ptr;
    }

    LAndExp* Parser::parseLAndExp() {
        auto ptr = new LAndExp();
        ptr->lineno = currentToken().lineno;
        ptr->exp = parseEqExp();
        submit(ptr);
        while (tryMatch(Token::AND)) {
            auto lAnd = new LAndExp();
            lAnd->lineno = ptr->lineno;
            lAnd->exp = LAndExp::OpExp(ptr, parseEqExp());
            ptr = lAnd;
            submit(ptr);
        }
        return ptr;
    }

    LOrExp* Parser::parseLOrExp() {
        auto ptr = new LOrExp();
        ptr->lineno = currentToken().lineno;
        ptr->exp = parseLAndExp();
        submit(ptr);
        while (tryMatch(Token::OR)) {
            auto lOr = new LOrExp();
            lOr->lineno = ptr->lineno;
            lOr->exp = LOrExp::OpExp(ptr, parseLAndExp());
            ptr = lOr;
            submit(ptr);
        }
        return ptr;
    }

    ConstExp* Parser::parseConstExp() {
        auto ptr = new ConstExp();
        ptr->lineno = currentToken().lineno;
        ptr->addExp = parseAddExp();
        submit(ptr);
        return ptr;
    }

    UnaryOp* Parser::parseUnaryOp() {
        auto ptr = new UnaryOp();
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
            default: ;
        }
        nextToken();
        submit(ptr);
        return ptr;
    }

    int Parser::stepUnaryOp(int offset) const {
        if (tokenStream_.peekType(offset, {Token::PLUS, Token::MINU, Token::NOT})) {
            offset += 1;
        }
        return offset;
    }
} // thm