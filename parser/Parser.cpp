//
// Created by slty5 on 24-9-27.
//

#include "Parser.h"

#include <cmath>
#include <iostream>
#include <fstream>

namespace thm {
    Parser::Parser(const TokenStream &tokenStream) : tokenStream_(tokenStream) {
    }

    void Parser::nextToken() {
        if (token_.type != DEFAULT) {
            tokens_.push_back(token_);
        }
        token_ = tokenStream_.next();
    }

    void Parser::ungetToken() {
        token_ = tokenStream_.unget();
        if (!tokens_.empty())
            tokens_.pop_back();
    }

    bool Parser::tryMatch(TokenType expectedType) {
        if (token_.type == expectedType) {
            nextToken();
            return true;
        }
        return false;
    }

    void Parser::matchToken(TokenType expectedType) {
        if (token_.type != expectedType) {
            if (expectedType == SEMICN) {
                ungetToken();
                errorReporter_.error(CompilerException(MISSING_SEMICOLON, token_.lineno));
                nextToken();
            } else if (expectedType == RPARENT) {
                ungetToken();
                errorReporter_.error(CompilerException(MISSING_RPARENT, token_.lineno));
                nextToken();
            } else if (expectedType == RBRACK) {
                ungetToken();
                errorReporter_.error(CompilerException(MISSING_RBRACK, token_.lineno));
                nextToken();
            } else {
            }
        } else {
            nextToken();
        }
    }


    std::unique_ptr<CompUnit> Parser::parseCompUnit() {
        auto ptr = std::make_unique<CompUnit>();
        bool hasDecl = true, hasFuncDef = true;
        while (hasDecl) {
            if (token_.type == CONSTTK) {
                ptr->decls.push_back(parseDecl());
                continue;
            }
            if (token_.type == INTTK || token_.type == CHARTK) {
                nextToken();
                if (tryMatch(IDENFR)) {
                    if (token_.type == COMMA || token_.type == SEMICN || token_.type == LBRACK || token_.type ==
                        ASSIGN) {
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
            if (token_.type == VOIDTK || token_.type == INTTK || token_.type == CHARTK) {
                nextToken();
                if (tryMatch(IDENFR)) {
                    matchToken(LPARENT);
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
        ptr->consume(tokens_);
        return ptr;
    }

    std::unique_ptr<Decl> Parser::parseDecl() {
        auto ptr = std::make_unique<Decl>();
        if (token_.type == CONSTTK) ptr->decl = parseConstDecl();
        else if (token_.type == INTTK || token_.type == CHARTK) ptr->decl = parseVarDecl();
        ptr->consume(tokens_);
        return ptr;
    }

    std::unique_ptr<ConstDecl> Parser::parseConstDecl() {
        auto ptr = std::make_unique<ConstDecl>();
        matchToken(CONSTTK);
        ptr->bType = parseBType();
        ptr->constDefs.push_back(parseConstDef());
        while (tryMatch(COMMA)) {
            ptr->constDefs.push_back(parseConstDef());
        }
        matchToken(SEMICN);
        ptr->consume(tokens_);
        return ptr;
    }

    std::unique_ptr<BType> Parser::parseBType() {
        auto ptr = std::make_unique<BType>();
        if (token_.type == INTTK || token_.type == CHARTK) {
            ptr->type = token_.type == INTTK ? BType::INT : BType::CHAR;
        } else {
            errorReporter_.error(CompilerException(UNEXPECTED_TOKEN, token_.lineno));
        }
        nextToken();
        ptr->consume(tokens_);
        return ptr;
    }


    std::unique_ptr<ConstDef> Parser::parseConstDef() {
        auto ptr = std::make_unique<ConstDef>();
        std::string ident = token_.content;
        matchToken(IDENFR);
        if (token_.type == LBRACK) {
            nextToken();
            std::unique_ptr<ConstExp> len;
            if (token_.type != RBRACK) {
                len = parseConstExp();
            }
            ptr->def = ConstDef::ConstDefArray(ident, std::move(len));
            matchToken(RBRACK);
        } else {
            ptr->def = ConstDef::ConstDefBasic(ident);
        }
        matchToken(ASSIGN);
        ptr->val = parseConstInitVal();
        ptr->consume(tokens_);
        return ptr;
    }

    std::unique_ptr<ConstInitVal> Parser::parseConstInitVal() {
        auto ptr = std::make_unique<ConstInitVal>();
        if (token_.type == PLUS || token_.type == MINU || token_.type == NOT || token_.type == IDENFR || token_.type ==
            LPARENT || token_.type == INTCON || token_.type == CHRCON) {
            ptr->val = ConstInitVal::ConstInitValBasic(parseConstExp());
        } else if (token_.type == LBRACE) {
            nextToken();
            std::vector<std::unique_ptr<ConstExp> > exps;
            if (token_.type == PLUS || token_.type == MINU || token_.type == NOT || token_.type == IDENFR || token_.type
                == LPARENT || token_.type == INTCON || token_.type == CHRCON) {
                exps.push_back(parseConstExp());
                while (token_.type == COMMA) {
                    nextToken();
                    exps.push_back(parseConstExp());
                }
            }
            ptr->val = ConstInitVal::ConstInitValArray(std::move(exps));
            matchToken(RBRACE);
        } else if (token_.type == STRCON) {
            ptr->val = token_.content;
            matchToken(STRCON);
        } else {
            errorReporter_.error(CompilerException(UNEXPECTED_TOKEN, token_.lineno));
        }
        ptr->consume(tokens_);
        return ptr;
    }

    std::unique_ptr<VarDecl> Parser::parseVarDecl() {
        auto ptr = std::make_unique<VarDecl>();
        ptr->bType = std::move(parseBType());
        ptr->varDefs.push_back(parseVarDef());
        while (token_.type == COMMA) {
            nextToken();
            ptr->varDefs.push_back(parseVarDef());
        }
        matchToken(SEMICN);
        ptr->consume(tokens_);
        return ptr;
    }

    std::unique_ptr<VarDef> Parser::parseVarDef() {
        auto ptr = std::make_unique<VarDef>();
        std::string ident = token_.content;
        matchToken(IDENFR);

        if (token_.type == LBRACK) {
            nextToken();
            std::unique_ptr<ConstExp> len;
            if (token_.type != RBRACK) {
                len = std::move(parseConstExp());
            }
            ptr->def = VarDef::VarDefArray(token_.content, std::move(len));
            matchToken(RBRACK);
        } else {
            ptr->def = VarDef::VarDefBasic(token_.content);
        }
        std::unique_ptr<InitVal> initVal;
        if (token_.type == ASSIGN) {
            nextToken();
            initVal = parseInitVal();
        }
        ptr->val = std::move(initVal);
        ptr->consume(tokens_);
        return ptr;
    }

    std::unique_ptr<InitVal> Parser::parseInitVal() {
        auto ptr = std::make_unique<InitVal>();
        if (token_.type == PLUS || token_.type == MINU || token_.type == NOT || token_.type == IDENFR || token_.type ==
            LPARENT || token_.type == INTCON || token_.type == CHRCON) {
            ptr->val = InitVal::InitValBasic(std::move(parseExp()));
        } else if (token_.type == LBRACE) {
            nextToken();
            std::vector<std::unique_ptr<Exp> > exps;
            if (token_.type == PLUS || token_.type == MINU || token_.type == NOT || token_.type == IDENFR || token_.type
                == LPARENT || token_.type == INTCON || token_.type == CHRCON) {
                exps.push_back(std::move(parseExp()));
                while (token_.type == COMMA) {
                    nextToken();
                    exps.push_back(std::move(parseExp()));
                }
            }
            ptr->val = InitVal::InitValArray(std::move(exps));
            matchToken(RBRACE);
        } else {
            ptr->val = token_.content;
            matchToken(STRCON);
        }
        ptr->consume(tokens_);
        return ptr;
    }

    std::unique_ptr<FuncDef> Parser::parseFuncDef() {
        auto ptr = std::make_unique<FuncDef>();
        ptr->funcType = std::move(parseFuncType());
        ptr->ident = token_.content;
        matchToken(IDENFR);
        matchToken(LPARENT);
        if (token_.type == INTTK || token_.type == CHARTK) {
            ptr->params = std::move(parseFuncFParams());
        }
        matchToken(RPARENT);
        ptr->block = std::move(parseBlock());
        ptr->consume(tokens_);
        return ptr;
    }

    std::unique_ptr<MainFuncDef> Parser::parseMainFuncDef() {
        auto ptr = std::make_unique<MainFuncDef>();
        matchToken(INTTK);
        matchToken(MAINTK);
        matchToken(LPARENT);
        matchToken(RPARENT);
        ptr->block = std::move(parseBlock());
        ptr->consume(tokens_);
        return ptr;
    }

    std::unique_ptr<FuncType> Parser::parseFuncType() {
        auto ptr = std::make_unique<FuncType>();
        if (token_.type == INTTK) {
            ptr->type = FuncType::INT;
        } else if (token_.type == CHARTK) {
            ptr->type = FuncType::CHAR;
        } else {
            ptr->type = FuncType::VOID;
        }
        nextToken();
        ptr->consume(tokens_);
        return ptr;
    }

    std::unique_ptr<FuncFParams> Parser::parseFuncFParams() {
        auto ptr = std::make_unique<FuncFParams>();
        ptr->params.push_back(std::move(parseFuncFParam()));
        while (tryMatch(COMMA)) {
            ptr->params.push_back(std::move(parseFuncFParam()));
        }
        ptr->consume(tokens_);
        return ptr;
    }

    std::unique_ptr<FuncFParam> Parser::parseFuncFParam() {
        auto ptr = std::make_unique<FuncFParam>();
        ptr->bType = std::move(parseBType());
        ptr->isArray = false;
        matchToken(IDENFR);
        if (token_.type == LBRACK) {
            nextToken();
            matchToken(RBRACK);
            ptr->isArray = true;
        }
        return ptr;
    }

    std::unique_ptr<Block> Parser::parseBlock() {
        matchToken(LBRACE);
        auto ptr = std::make_unique<Block>();
        while (token_.type != RBRACE) {
            ptr->items.push_back(std::move(parseBlockItem()));
        }
        matchToken(RBRACE);
        ptr->consume(tokens_);
        return ptr;
    }

    std::unique_ptr<BlockItem> Parser::parseBlockItem() {
        auto ptr = std::make_unique<BlockItem>();
        if (token_.type == INTTK || token_.type == CHARTK || token_.type == CONSTTK) {
            ptr->item = std::move(parseDecl());
        } else {
            ptr->item = std::move(parseStmt());
        }
        ptr->consume(tokens_);
        return ptr;
    }

    std::unique_ptr<Stmt> Parser::parseStmt() {
        auto ptr = std::make_unique<Stmt>();
        if (token_.type == LBRACE) {
            ptr->stmt = std::move(parseBlock());
        } else if (token_.type == IFTK) {
            matchToken(IFTK);
            matchToken(LPARENT);
            auto cond = std::move(parseCond());
            matchToken(RPARENT);
            auto stmt = std::move(parseStmt());
            std::unique_ptr<Stmt> elseStmt;
            if (token_.type == ELSETK) {
                matchToken(ELSETK);
                elseStmt = std::move(parseStmt());
            }
            ptr->stmt = Stmt::StmtIf(std::move(cond), std::move(stmt), std::move(elseStmt));
        } else if (token_.type == FORTK) {
            std::unique_ptr<ForStmt> initStmt, updateStmt;
            std::unique_ptr<Cond> cond;
            matchToken(FORTK);
            matchToken(LPARENT);
            if (token_.type == SEMICN) {
                matchToken(SEMICN);
            } else {
                initStmt = std::move(parseForStmt());
                matchToken(SEMICN);
            }
            if (token_.type == SEMICN) {
                matchToken(SEMICN);
            } else {
                cond = std::move(parseCond());
                matchToken(SEMICN);
            }
            if (token_.type == RPARENT) {
                matchToken(RPARENT);
            } else {
                updateStmt = std::move(parseForStmt());
                matchToken(RPARENT);
            }
            auto stmt = parseStmt();
            ptr->stmt = Stmt::StmtFor(std::move(initStmt), std::move(cond), std::move(updateStmt), std::move(stmt));
        } else if (token_.type == BREAKTK) {
            matchToken(BREAKTK);
            matchToken(SEMICN);
            ptr->stmt = Stmt::BREAK;
        } else if (token_.type == CONTINUETK) {
            matchToken(CONTINUETK);
            matchToken(SEMICN);
            ptr->stmt = Stmt::CONTINUE;
        } else if (token_.type == RETURNTK) {
            nextToken();
            std::unique_ptr<Exp> returnExp;
            if (token_.type == SEMICN) {
                matchToken(SEMICN);
            } else {
                returnExp = std::move(parseExp());
                matchToken(SEMICN);
            }
            ptr->stmt = Stmt::StmtReturn(std::move(returnExp));
        } else if (token_.type == PRINTFTK) {
            matchToken(PRINTFTK);
            matchToken(LPARENT);
            std::string fmt = token_.content;
            matchToken(STRCON);
            std::vector<std::unique_ptr<Exp> > args;
            while (tryMatch(COMMA)) {
                args.push_back(std::move(parseExp()));
            }
            matchToken(RPARENT);
            matchToken(SEMICN);
            ptr->stmt = Stmt::StmtPrintf(std::move(fmt), std::move(args));
        } else {
            if (token_.type == SEMICN) {
                matchToken(SEMICN);
            } else {
                int step = 0;
                bool assign = false;
                while (token_.type != SEMICN) {
                    nextToken();
                    step++;
                    if (token_.type == ASSIGN) {
                        assign = true;
                        break;
                    }
                }
                for (int i = 0; i < step; ++i) {
                    ungetToken();
                }
                if (assign) {
                    auto lVal = std::move(parseLVal());
                    matchToken(ASSIGN);
                    TokenType tokenType = token_.type;
                    if (tryMatch(GETINTTK) || tryMatch(GETCHARTK)) {
                        Stmt::StmtRead::ReadType type = tokenType == GETINTTK
                                                            ? Stmt::StmtRead::INT
                                                            : Stmt::StmtRead::CHAR;
                        matchToken(LPARENT);
                        matchToken(RPARENT);
                        ptr->stmt = Stmt::StmtRead(std::move(lVal), type);
                    } else {
                        ptr->stmt = Stmt::StmtAssign(std::move(lVal), std::move(parseExp()));
                    }
                } else {
                    ptr->stmt = std::move(parseExp());
                }
                matchToken(SEMICN);
            }
        }
        ptr->consume(tokens_);
        return ptr;
    }

    std::unique_ptr<ForStmt> Parser::parseForStmt() {
        auto ptr = std::make_unique<ForStmt>();
        ptr->lVal = std::move(parseLVal());
        matchToken(ASSIGN);
        ptr->exp = std::move(parseExp());
        ptr->consume(tokens_);
        return ptr;
    }

    std::unique_ptr<Exp> Parser::parseExp() {
        auto ptr = std::make_unique<Exp>();
        ptr->addExp = std::move(parseAddExp());
        ptr->consume(tokens_);
        return ptr;
    }

    std::unique_ptr<Cond> Parser::parseCond() {
        auto ptr = std::make_unique<Cond>();
        ptr->lOrExp = std::move(parseLOrExp());
        ptr->consume(tokens_);
        return ptr;
    }

    std::unique_ptr<LVal> Parser::parseLVal() {
        auto ptr = std::make_unique<LVal>();
        ptr->ident = token_.content;
        matchToken(IDENFR);
        if (token_.type == LBRACK) {
            matchToken(LBRACK);
            ptr->exp = parseExp();
            matchToken(RBRACK);
        }
        ptr->consume(tokens_);
        return ptr;
    }

    std::unique_ptr<PrimaryExp> Parser::parsePrimaryExp() {
        auto ptr = std::make_unique<PrimaryExp>();
        if (token_.type == LPARENT) {
            matchToken(LPARENT);
            ptr->primaryExp = std::move(parseExp());
            matchToken(RPARENT);
        } else if (token_.type == IDENFR) {
            ptr->primaryExp = std::move(parseLVal());
        } else if (token_.type == INTCON) {
            ptr->primaryExp = std::move(parseNumber());
        } else {
            ptr->primaryExp = std::move(parseCharacter());
        }
        ptr->consume(tokens_);
        return ptr;
    }

    std::unique_ptr<Number> Parser::parseNumber() {
        int v = atoi(token_.content.c_str());
        matchToken(INTCON);
        auto ptr = std::make_unique<Number>();
        ptr->num = v;
        ptr->consume(tokens_);
        return ptr;
    }

    std::unique_ptr<Character> Parser::parseCharacter() {
        char v = token_.content[0];
        matchToken(CHRCON);
        auto ptr = std::make_unique<Character>();
        ptr->ch = v;
        ptr->consume(tokens_);
        return ptr;
    }

    std::unique_ptr<UnaryExp> Parser::parseUnaryExp() {
        auto ptr = std::make_unique<UnaryExp>();
        if (token_.type == IDENFR) {
            std::string ident = token_.content;
            nextToken();
            if (token_.type == LPARENT) {
                nextToken();
                if (token_.type != RPARENT) {
                    ptr->exp = UnaryExp::FuncExp(ident, std::move(parseFuncRParams()));
                }
                matchToken(RPARENT);
                ptr->consume(tokens_);
                return ptr;
            }
            ungetToken();
        }
        if (token_.type == PLUS || token_.type == MINU || token_.type == NOT) {
            ptr->exp = UnaryExp::OpExp(std::move(parseUnaryOp()), std::move(parseUnaryExp()));
        } else {
            ptr->exp = std::move(parsePrimaryExp());
        }
        ptr->consume(tokens_);
        return ptr;
    }

    std::unique_ptr<FuncRParams> Parser::parseFuncRParams() {
        auto ptr = std::make_unique<FuncRParams>();
        ptr->params.push_back(parseExp());
        while (token_.type == COMMA) {
            matchToken(COMMA);
            ptr->params.push_back(parseExp());
        }
        ptr->consume(tokens_);
        return ptr;
    }

    std::unique_ptr<MulExp> Parser::parseMulExp() {
        auto ptr = std::make_unique<MulExp>();
        ptr->exp = std::move(parseUnaryExp());
        while (token_.type == MULT || token_.type == DIV || token_.type == MOD) {
            nextToken();
            auto mul = std::make_unique<MulExp>();
            MulExp::OpExp::Op op = MulExp::OpExp::MUL;
            switch (token_.type) {
                case MULT:
                    op = MulExp::OpExp::MUL;
                    break;
                case DIV:
                    op = MulExp::OpExp::DIV;
                    break;
                case MOD:
                    op = MulExp::OpExp::MOD;
                    break;
            }
            mul->exp = MulExp::OpExp(std::move(ptr), op, std::move(parseUnaryExp()));
            ptr = std::move(mul);
        }
        ptr->consume(tokens_);
        return ptr;
    }

    std::unique_ptr<AddExp> Parser::parseAddExp() {
        auto ptr = std::make_unique<AddExp>();
        ptr->exp = std::move(parseMulExp());
        while (token_.type == PLUS || token_.type == MINU) {
            nextToken();
            auto add = std::make_unique<AddExp>();
            AddExp::OpExp::Op op;
            switch (token_.type) {
                case PLUS:
                    op = AddExp::OpExp::ADD;
                    break;
                case MINU:
                    op = AddExp::OpExp::MINUS;
                    break;
                default:
                    op = AddExp::OpExp::ADD;
            }
            add->exp = AddExp::OpExp(std::move(ptr), op, std::move(parseMulExp()));
            ptr = std::move(add);
        }
        ptr->consume(tokens_);
        return ptr;
    }

    std::unique_ptr<RelExp> Parser::parseRelExp() {
        auto ptr = std::make_unique<RelExp>();
        ptr->exp = std::move(parseAddExp());
        while (token_.type == LSS || token_.type == LEQ || token_.type == GRE || token_.type == GEQ) {
            nextToken();
            auto rel = std::make_unique<RelExp>();
            RelExp::OpExp::Op op = RelExp::OpExp::LT;
            switch (token_.type) {
                case LSS:
                    op = RelExp::OpExp::LT;
                    break;
                case LEQ:
                    op = RelExp::OpExp::LE;
                    break;
                case GRE:
                    op = RelExp::OpExp::GT;
                    break;
                case GEQ:
                    op = RelExp::OpExp::GE;
                    break;
            }
            rel->exp = RelExp::OpExp(std::move(ptr), op, std::move(parseAddExp()));
            ptr = std::move(rel);
        }
        ptr->consume(tokens_);
        return ptr;
    }

    std::unique_ptr<EqExp> Parser::parseEqExp() {
        auto ptr = std::make_unique<EqExp>();
        ptr->exp = std::move(parseRelExp());
        while (token_.type == EQL || token_.type == NEQ) {
            nextToken();
            auto eq = std::make_unique<EqExp>();
            eq->exp = EqExp::OpExp(std::move(ptr), token_.type == EQL ? EqExp::OpExp::EQ : EqExp::OpExp::NEQ,
                                   std::move(parseRelExp()));
            ptr = std::move(eq);
        }
        ptr->consume(tokens_);
        return ptr;
    }

    std::unique_ptr<LAndExp> Parser::parseLAndExp() {
        auto ptr = std::make_unique<LAndExp>();
        ptr->exp = std::move(parseEqExp());
        while (token_.type == AND) {
            nextToken();
            auto lAnd = std::make_unique<LAndExp>();
            lAnd->exp = LAndExp::OpExp(std::move(ptr), std::move(parseEqExp()));
            ptr = std::move(lAnd);
        }
        ptr->consume(tokens_);
        return ptr;
    }

    std::unique_ptr<LOrExp> Parser::parseLOrExp() {
        auto ptr = std::make_unique<LOrExp>();
        ptr->exp = std::move(parseLAndExp());
        while (token_.type == OR) {
            nextToken();
            auto lOr = std::make_unique<LOrExp>();
            lOr->exp = LOrExp::OpExp(std::move(ptr), std::move(parseLAndExp()));
            ptr = std::move(lOr);
        }
        ptr->consume(tokens_);
        return ptr;
    }

    std::unique_ptr<ConstExp> Parser::parseConstExp() {
        auto ptr = std::make_unique<ConstExp>();
        ptr->addExp = parseAddExp();
        ptr->consume(tokens_);
        return ptr;
    }

    std::unique_ptr<UnaryOp> Parser::parseUnaryOp() {
        auto ptr = std::make_unique<UnaryOp>();
        switch (token_.type) {
            case PLUS:
                ptr->type = UnaryOp::PLUS;
                break;
            case MINU:
                ptr->type = UnaryOp::MINUS;
                break;
            case NOT:
                ptr->type = UnaryOp::NOT;
                break;
        }
        nextToken();
        ptr->consume(tokens_);
        return ptr;
    }
} // thm
