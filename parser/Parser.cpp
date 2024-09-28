//
// Created by slty5 on 24-9-27.
//

#include "Parser.h"

#include <cmath>
#include <iostream>
#include <fstream>

namespace thm {
    Parser::Parser(const TokenStream &tokenStream) : tokenStream_(tokenStream) {
        os.open("parser.txt");
    }

    void Parser::nextToken() {
        if (token_.type != Token::DEFAULT) {
            tokens_.push_back(token_);
        }
        currentLine_ = token_.lineno;
        token_ = tokenStream_.next();
    }

    void Parser::ungetToken() {
        token_ = tokenStream_.unget();
        currentLine_ = token_.lineno;
        if (!tokens_.empty())
            tokens_.pop_back();
    }

    bool Parser::tryMatch(Token::TokenType expectedType) {
        if (token_.type == expectedType) {
            nextToken();
            return true;
        }
        return false;
    }

    void Parser::matchToken(Token::TokenType expectedType) {
        if (token_.type != expectedType) {
            if (expectedType == Token::SEMICN) {
                errorReporter_.error(CompilerException(MISSING_SEMICOLON, currentLine_));
            } else if (expectedType == Token::RPARENT) {
                errorReporter_.error(CompilerException(MISSING_RPARENT, currentLine_));
            } else if (expectedType == Token::RBRACK) {
                errorReporter_.error(CompilerException(MISSING_RBRACK, currentLine_));
            } else {

            }
        } else {
            nextToken();
        }
    }

    std::unique_ptr<CompUnit> Parser::parseCompUnit() {
        auto ptr = std::make_unique<CompUnit>();
        ptr->lineno = token_.lineno;
        bool hasDecl = true, hasFuncDef = true;
        while (hasDecl) {
            if (token_.type == Token::CONSTTK) {
                ptr->decls.push_back(parseDecl());
                continue;
            }
            if (token_.type == Token::INTTK || token_.type == Token::CHARTK) {
                nextToken();
                if (tryMatch(Token::IDENFR)) {
                    if (token_.type == Token::COMMA || token_.type == Token::SEMICN || token_.type == Token::LBRACK || token_.type ==
                        Token::ASSIGN) {
                        ungetToken();
                        ungetToken();
                        ptr->decls.push_back(parseDecl());
                        continue;;
                    }
                    ungetToken();
                }
                ungetToken();
            }
            hasDecl = false;
        }
        while (hasFuncDef) {
            if (token_.type == Token::VOIDTK || token_.type == Token::INTTK || token_.type == Token::CHARTK) {
                nextToken();
                if (tryMatch(Token::IDENFR)) {
                    matchToken(Token::LPARENT);
                    ungetToken();
                    ungetToken();
                    ungetToken();
                    ptr->funcDefs.push_back(parseFuncDef());
                    continue;
                }
                ungetToken();
            }
            hasFuncDef = false;
        }
        ptr->mainFuncDef = parseMainFuncDef();
        submit(ptr);
        return ptr;
    }

    std::unique_ptr<Decl> Parser::parseDecl() {
        auto ptr = std::make_unique<Decl>();
        ptr->lineno = token_.lineno;

        if (token_.type == Token::CONSTTK) ptr->decl = parseConstDecl();
        else if (token_.type == Token::INTTK || token_.type == Token::CHARTK) ptr->decl = parseVarDecl();
        submit(ptr);
        return ptr;
    }

    std::unique_ptr<ConstDecl> Parser::parseConstDecl() {
        auto ptr = std::make_unique<ConstDecl>();
        ptr->lineno = token_.lineno;

        matchToken(Token::CONSTTK);
        ptr->bType = parseBType();
        ptr->constDefs.push_back(parseConstDef());
        while (tryMatch(Token::COMMA)) {
            ptr->constDefs.push_back(parseConstDef());
        }
        matchToken(Token::SEMICN);
        submit(ptr);
        return ptr;
    }

    std::unique_ptr<BType> Parser::parseBType() {
        auto ptr = std::make_unique<BType>();
        ptr->lineno = token_.lineno;

        if (token_.type == Token::INTTK || token_.type == Token::CHARTK) {
            ptr->type = token_.type == Token::INTTK ? BType::INT : BType::CHAR;
        } else {
            errorReporter_.error(CompilerException(UNEXPECTED_TOKEN, token_.lineno));
        }
        nextToken();
        submit(ptr);
        return ptr;
    }


    std::unique_ptr<ConstDef> Parser::parseConstDef() {
        auto ptr = std::make_unique<ConstDef>();
        ptr->lineno = token_.lineno;

        std::string ident = token_.content;
        matchToken(Token::IDENFR);
        if (token_.type == Token::LBRACK) {
            nextToken();
            std::unique_ptr<ConstExp> len;
            if (token_.type != Token::RBRACK) {
                len = parseConstExp();
            }
            ptr->def = ConstDef::ConstDefArray(ident, std::move(len));
            matchToken(Token::RBRACK);
        } else {
            ptr->def = ConstDef::ConstDefBasic(ident);
        }
        matchToken(Token::ASSIGN);
        ptr->val = parseConstInitVal();
        submit(ptr);
        return ptr;
    }

    std::unique_ptr<ConstInitVal> Parser::parseConstInitVal() {
        auto ptr = std::make_unique<ConstInitVal>();
        ptr->lineno = token_.lineno;

        if (token_.type == Token::PLUS || token_.type == Token::MINU || token_.type == Token::NOT || token_.type == Token::IDENFR || token_.type ==
            Token::LPARENT || token_.type == Token::INTCON || token_.type == Token::CHRCON) {
            ptr->val = ConstInitVal::ConstInitValBasic(parseConstExp());
        } else if (token_.type == Token::LBRACE) {
            nextToken();
            std::vector<std::unique_ptr<ConstExp> > exps;
            if (token_.type == Token::PLUS || token_.type == Token::MINU || token_.type == Token::NOT || token_.type == Token::IDENFR || token_.type
                == Token::LPARENT || token_.type == Token::INTCON || token_.type == Token::CHRCON) {
                exps.push_back(parseConstExp());
                while (token_.type == Token::COMMA) {
                    nextToken();
                    exps.push_back(parseConstExp());
                }
            }
            ptr->val = ConstInitVal::ConstInitValArray(std::move(exps));
            matchToken(Token::RBRACE);
        } else if (token_.type == Token::STRCON) {
            ptr->val = token_.content;
            matchToken(Token::STRCON);
        } else {
            errorReporter_.error(CompilerException(UNEXPECTED_TOKEN, token_.lineno));
        }
        submit(ptr);
        return ptr;
    }

    std::unique_ptr<VarDecl> Parser::parseVarDecl() {
        auto ptr = std::make_unique<VarDecl>();
        ptr->lineno = token_.lineno;

        ptr->bType = std::move(parseBType());
        ptr->varDefs.push_back(parseVarDef());
        while (token_.type == Token::COMMA) {
            nextToken();
            ptr->varDefs.push_back(parseVarDef());
        }
        matchToken(Token::SEMICN);
        submit(ptr);
        return ptr;
    }

    std::unique_ptr<VarDef> Parser::parseVarDef() {
        auto ptr = std::make_unique<VarDef>();
        ptr->lineno = token_.lineno;

        std::string ident = token_.content;
        matchToken(Token::IDENFR);

        if (token_.type == Token::LBRACK) {
            nextToken();
            std::unique_ptr<ConstExp> len;
            if (token_.type != Token::RBRACK) {
                len = std::move(parseConstExp());
            }
            ptr->def = VarDef::VarDefArray(token_.content, std::move(len));
            matchToken(Token::RBRACK);
        } else {
            ptr->def = VarDef::VarDefBasic(token_.content);
        }
        std::unique_ptr<InitVal> initVal;
        if (token_.type == Token::ASSIGN) {
            nextToken();
            initVal = parseInitVal();
        }
        ptr->val = std::move(initVal);
        submit(ptr);
        return ptr;
    }

    std::unique_ptr<InitVal> Parser::parseInitVal() {
        auto ptr = std::make_unique<InitVal>();
        ptr->lineno = token_.lineno;

        if (token_.type == Token::PLUS || token_.type == Token::MINU || token_.type == Token::NOT || token_.type == Token::IDENFR || token_.type ==
            Token::LPARENT || token_.type == Token::INTCON || token_.type == Token::CHRCON) {
            ptr->val = InitVal::InitValBasic(std::move(parseExp()));
        } else if (token_.type == Token::LBRACE) {
            nextToken();
            std::vector<std::unique_ptr<Exp> > exps;
            if (token_.type == Token::PLUS || token_.type == Token::MINU || token_.type == Token::NOT || token_.type == Token::IDENFR || token_.type
                == Token::LPARENT || token_.type == Token::INTCON || token_.type == Token::CHRCON) {
                exps.push_back(std::move(parseExp()));
                while (token_.type == Token::COMMA) {
                    nextToken();
                    exps.push_back(std::move(parseExp()));
                }
            }
            ptr->val = InitVal::InitValArray(std::move(exps));
            matchToken(Token::RBRACE);
        } else {
            ptr->val = token_.content;
            matchToken(Token::STRCON);
        }
        submit(ptr);
        return ptr;
    }

    std::unique_ptr<FuncDef> Parser::parseFuncDef() {
        auto ptr = std::make_unique<FuncDef>();
        ptr->lineno = token_.lineno;

        ptr->funcType = std::move(parseFuncType());
        ptr->ident = token_.content;
        matchToken(Token::IDENFR);
        matchToken(Token::LPARENT);
        if (token_.type == Token::INTTK || token_.type == Token::CHARTK) {
            ptr->params = std::move(parseFuncFParams());
        }
        matchToken(Token::RPARENT);
        ptr->block = std::move(parseBlock());
        submit(ptr);
        return ptr;
    }

    std::unique_ptr<MainFuncDef> Parser::parseMainFuncDef() {
        auto ptr = std::make_unique<MainFuncDef>();
        ptr->lineno = token_.lineno;

        matchToken(Token::INTTK);
        matchToken(Token::MAINTK);
        matchToken(Token::LPARENT);
        matchToken(Token::RPARENT);
        ptr->block = std::move(parseBlock());
        submit(ptr);
        return ptr;
    }

    std::unique_ptr<FuncType> Parser::parseFuncType() {
        auto ptr = std::make_unique<FuncType>();
        ptr->lineno = token_.lineno;

        if (token_.type == Token::INTTK) {
            ptr->type = FuncType::INT;
        } else if (token_.type == Token::CHARTK) {
            ptr->type = FuncType::CHAR;
        } else {
            ptr->type = FuncType::VOID;
        }
        nextToken();
        submit(ptr);
        return ptr;
    }

    std::unique_ptr<FuncFParams> Parser::parseFuncFParams() {
        auto ptr = std::make_unique<FuncFParams>();
        ptr->lineno = token_.lineno;

        ptr->params.push_back(parseFuncFParam());
        while (tryMatch(Token::COMMA)) {
            ptr->params.push_back(parseFuncFParam());
        }
        submit(ptr);
        return ptr;
    }

    std::unique_ptr<FuncFParam> Parser::parseFuncFParam() {
        auto ptr = std::make_unique<FuncFParam>();
        ptr->lineno = token_.lineno;

        ptr->bType = std::move(parseBType());
        ptr->isArray = false;
        matchToken(Token::IDENFR);
        if (token_.type == Token::LBRACK) {
            nextToken();
            matchToken(Token::RBRACK);
            ptr->isArray = true;
        }
        submit(ptr);
        return ptr;
    }

    std::unique_ptr<Block> Parser::parseBlock() {
        auto ptr = std::make_unique<Block>();
        ptr->lineno = token_.lineno;
        matchToken(Token::LBRACE);

        while (token_.type != Token::RBRACE) {
            ptr->items.push_back(std::move(parseBlockItem()));
        }
        matchToken(Token::RBRACE);
        submit(ptr);
        return ptr;
    }

    std::unique_ptr<BlockItem> Parser::parseBlockItem() {
        auto ptr = std::make_unique<BlockItem>();
        ptr->lineno = token_.lineno;

        if (token_.type == Token::INTTK || token_.type == Token::CHARTK || token_.type == Token::CONSTTK) {
            ptr->item = std::move(parseDecl());
        } else {
            ptr->item = std::move(parseStmt());
        }
        submit(ptr);
        return ptr;
    }

    std::unique_ptr<Stmt> Parser::parseStmt() {
        auto ptr = std::make_unique<Stmt>();
        ptr->lineno = token_.lineno;

        if (token_.type == Token::LBRACE) {
            ptr->stmt = std::move(parseBlock());
        } else if (token_.type == Token::IFTK) {
            matchToken(Token::IFTK);
            matchToken(Token::LPARENT);
            auto cond = std::move(parseCond());
            matchToken(Token::RPARENT);
            auto stmt = std::move(parseStmt());
            std::unique_ptr<Stmt> elseStmt;
            if (token_.type == Token::ELSETK) {
                matchToken(Token::ELSETK);
                elseStmt = std::move(parseStmt());
            }
            ptr->stmt = Stmt::StmtIf(std::move(cond), std::move(stmt), std::move(elseStmt));
        } else if (token_.type == Token::FORTK) {
            std::unique_ptr<ForStmt> initStmt, updateStmt;
            std::unique_ptr<Cond> cond;
            matchToken(Token::FORTK);
            matchToken(Token::LPARENT);
            if (token_.type == Token::SEMICN) {
                matchToken(Token::SEMICN);
            } else {
                initStmt = std::move(parseForStmt());
                matchToken(Token::SEMICN);
            }
            if (token_.type == Token::SEMICN) {
                matchToken(Token::SEMICN);
            } else {
                cond = std::move(parseCond());
                matchToken(Token::SEMICN);
            }
            if (token_.type == Token::RPARENT) {
                matchToken(Token::RPARENT);
            } else {
                updateStmt = std::move(parseForStmt());
                matchToken(Token::RPARENT);
            }
            auto stmt = parseStmt();
            ptr->stmt = Stmt::StmtFor(std::move(initStmt), std::move(cond), std::move(updateStmt), std::move(stmt));
        } else if (token_.type == Token::BREAKTK) {
            matchToken(Token::BREAKTK);
            matchToken(Token::SEMICN);
            ptr->stmt = Stmt::BREAK;
        } else if (token_.type == Token::CONTINUETK) {
            matchToken(Token::CONTINUETK);
            matchToken(Token::SEMICN);
            ptr->stmt = Stmt::CONTINUE;
        } else if (token_.type == Token::RETURNTK) {
            nextToken();
            std::unique_ptr<Exp> returnExp;
            if (token_.type == Token::SEMICN) {
                matchToken(Token::SEMICN);
            } else {
                returnExp = std::move(parseExp());
                matchToken(Token::SEMICN);
            }
            ptr->stmt = Stmt::StmtReturn(std::move(returnExp));
        } else if (token_.type == Token::PRINTFTK) {
            matchToken(Token::PRINTFTK);
            matchToken(Token::LPARENT);
            std::string fmt = token_.content;
            matchToken(Token::STRCON);
            std::vector<std::unique_ptr<Exp> > args;
            while (tryMatch(Token::COMMA)) {
                args.push_back(std::move(parseExp()));
            }
            matchToken(Token::RPARENT);
            matchToken(Token::SEMICN);
            ptr->stmt = Stmt::StmtPrintf(std::move(fmt), std::move(args));
        } else {
            if (token_.type == Token::SEMICN) {
                matchToken(Token::SEMICN);
            } else {
                int step = 0;
                bool assign = false;
                while (token_.type != Token::SEMICN && token_.type != Token::TK_EOF) {
                    nextToken();
                    step++;
                    if (token_.type == Token::ASSIGN) {
                        assign = true;
                        break;
                    }
                }
                for (int i = 0; i < step; ++i) {
                    ungetToken();
                }
                if (assign) {
                    auto lVal = std::move(parseLVal());
                    matchToken(Token::ASSIGN);
                    Token::TokenType tokenType = token_.type;
                    if (tryMatch(Token::GETINTTK) || tryMatch(Token::GETCHARTK)) {
                        Stmt::StmtRead::ReadType type = tokenType == Token::GETINTTK
                                                            ? Stmt::StmtRead::INT
                                                            : Stmt::StmtRead::CHAR;
                        matchToken(Token::LPARENT);
                        matchToken(Token::RPARENT);
                        ptr->stmt = Stmt::StmtRead(std::move(lVal), type);
                    } else {
                        ptr->stmt = Stmt::StmtAssign(std::move(lVal), std::move(parseExp()));
                    }
                } else {
                    ptr->stmt = std::move(parseExp());
                }
                matchToken(Token::SEMICN);
            }
        }
        submit(ptr);
        return ptr;
    }

    std::unique_ptr<ForStmt> Parser::parseForStmt() {
        auto ptr = std::make_unique<ForStmt>();
        ptr->lineno = token_.lineno;

        ptr->lVal = std::move(parseLVal());
        matchToken(Token::ASSIGN);
        ptr->exp = std::move(parseExp());
        submit(ptr);
        return ptr;
    }

    std::unique_ptr<Exp> Parser::parseExp() {
        auto ptr = std::make_unique<Exp>();
        ptr->lineno = token_.lineno;

        ptr->addExp = std::move(parseAddExp());
        submit(ptr);
        return ptr;
    }

    std::unique_ptr<Cond> Parser::parseCond() {
        auto ptr = std::make_unique<Cond>();
        ptr->lineno = token_.lineno;

        ptr->lOrExp = std::move(parseLOrExp());
        submit(ptr);
        return ptr;
    }

    std::unique_ptr<LVal> Parser::parseLVal() {
        auto ptr = std::make_unique<LVal>();
        ptr->lineno = token_.lineno;

        ptr->ident = token_.content;
        matchToken(Token::IDENFR);
        if (token_.type == Token::LBRACK) {
            matchToken(Token::LBRACK);
            ptr->exp = parseExp();
            matchToken(Token::RBRACK);
        }
        submit(ptr);
        return ptr;
    }

    std::unique_ptr<PrimaryExp> Parser::parsePrimaryExp() {
        auto ptr = std::make_unique<PrimaryExp>();
        ptr->lineno = token_.lineno;
        if (token_.type == Token::LPARENT) {
            matchToken(Token::LPARENT);
            ptr->primaryExp = std::move(parseExp());
            matchToken(Token::RPARENT);
        } else if (token_.type == Token::IDENFR) {
            ptr->primaryExp = std::move(parseLVal());
        } else if (token_.type == Token::INTCON) {
            ptr->primaryExp = std::move(parseNumber());
        } else {
            ptr->primaryExp = std::move(parseCharacter());
        }
        submit(ptr);
        return ptr;
    }

    std::unique_ptr<Number> Parser::parseNumber() {
        auto ptr = std::make_unique<Number>();
        ptr->lineno = token_.lineno;
        int v = atoi(token_.content.c_str());
        matchToken(Token::INTCON);
        ptr->num = v;
        submit(ptr);
        return ptr;
    }

    std::unique_ptr<Character> Parser::parseCharacter() {
        auto ptr = std::make_unique<Character>();
        char v = token_.content[0];
        ptr->lineno = token_.lineno;
        matchToken(Token::CHRCON);
        ptr->ch = v;
        submit(ptr);
        return ptr;
    }

    std::unique_ptr<UnaryExp> Parser::parseUnaryExp() {
        auto ptr = std::make_unique<UnaryExp>();
        ptr->lineno = token_.lineno;
        if (token_.type == Token::IDENFR) {
            std::string ident = token_.content;
            nextToken();
            if (token_.type == Token::LPARENT) {
                nextToken();
                if (token_.type != Token::RPARENT) {
                    ptr->exp = UnaryExp::FuncExp(ident, std::move(parseFuncRParams()));
                }
                matchToken(Token::RPARENT);
                submit(ptr);
                return ptr;
            }
            ungetToken();
        }
        if (token_.type == Token::PLUS || token_.type == Token::MINU || token_.type == Token::NOT) {
            ptr->exp = UnaryExp::OpExp(std::move(parseUnaryOp()), std::move(parseUnaryExp()));
        } else {
            ptr->exp = std::move(parsePrimaryExp());
        }
        submit(ptr);
        return ptr;
    }

    std::unique_ptr<FuncRParams> Parser::parseFuncRParams() {
        auto ptr = std::make_unique<FuncRParams>();
        ptr->lineno = token_.lineno;
        ptr->params.push_back(parseExp());
        while (token_.type == Token::COMMA) {
            matchToken(Token::COMMA);
            ptr->params.push_back(parseExp());
        }
        submit(ptr);
        return ptr;
    }

    std::unique_ptr<MulExp> Parser::parseMulExp() {
        auto ptr = std::make_unique<MulExp>();
        ptr->lineno = token_.lineno;
        ptr->exp = std::move(parseUnaryExp());
        submit(ptr);
        while (token_.type == Token::MULT || token_.type == Token::DIV || token_.type == Token::MOD) {
            nextToken();
            auto mul = std::make_unique<MulExp>();
            MulExp::OpExp::Op op = MulExp::OpExp::MUL;
            switch (token_.type) {
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
        ptr->lineno = token_.lineno;
        submit(ptr);
        while (token_.type == Token::PLUS || token_.type == Token::MINU) {
            nextToken();
            auto add = std::make_unique<AddExp>();
            AddExp::OpExp::Op op;
            switch (token_.type) {
                case Token::PLUS:
                    op = AddExp::OpExp::ADD;
                break;
                case Token::MINU:
                    op = AddExp::OpExp::MINUS;
                break;
                default:
                    op = AddExp::OpExp::ADD;
            }
            add->lineno = ptr->lineno;
            add->exp = AddExp::OpExp(std::move(ptr), op, std::move(parseMulExp()));
            ptr = std::move(add);
            submit(ptr);
        }
        return ptr;
    }

    std::unique_ptr<RelExp> Parser::parseRelExp() {
        auto ptr = std::make_unique<RelExp>();
        ptr->lineno = token_.lineno;
        ptr->exp = std::move(parseAddExp());
        submit(ptr);

        while (token_.type == Token::LSS || token_.type == Token::LEQ || token_.type == Token::GRE || token_.type == Token::GEQ) {
            nextToken();
            auto rel = std::make_unique<RelExp>();
            RelExp::OpExp::Op op = RelExp::OpExp::LT;
            switch (token_.type) {
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
            rel->lineno = ptr->lineno;
            rel->exp = RelExp::OpExp(std::move(ptr), op, std::move(parseAddExp()));
            ptr = std::move(rel);
            submit(ptr);
        }
        return ptr;
    }

    std::unique_ptr<EqExp> Parser::parseEqExp() {
        auto ptr = std::make_unique<EqExp>();
        ptr->lineno = token_.lineno;
        ptr->exp = std::move(parseRelExp());
        submit(ptr);
        while (token_.type == Token::EQL || token_.type == Token::NEQ) {
            nextToken();
            auto eq = std::make_unique<EqExp>();
            eq->lineno = ptr->lineno;
            eq->exp = EqExp::OpExp(std::move(ptr), token_.type ==Token:: EQL ? EqExp::OpExp::EQ : EqExp::OpExp::NEQ,
                                   std::move(parseRelExp()));
            ptr = std::move(eq);
            submit(ptr);
        }
        return ptr;
    }

    std::unique_ptr<LAndExp> Parser::parseLAndExp() {
        auto ptr = std::make_unique<LAndExp>();
        ptr->lineno = token_.lineno;
        ptr->exp = std::move(parseEqExp());
        submit(ptr);
        while (token_.type == Token::AND) {
            nextToken();
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
        ptr->lineno = token_.lineno;
        ptr->exp = std::move(parseLAndExp());
        submit(ptr);
        while (token_.type == Token::OR) {
            nextToken();
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
        ptr->lineno = token_.lineno;
        ptr->addExp = parseAddExp();
        submit(ptr);
        return ptr;
    }

    std::unique_ptr<UnaryOp> Parser::parseUnaryOp() {
        auto ptr = std::make_unique<UnaryOp>();
        ptr->lineno = token_.lineno;
        switch (token_.type) {
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
