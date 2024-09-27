//
// Created by slty5 on 24-9-27.
//

#include "Parser.h"

#include <iostream>
#include <fstream>
std::ofstream outfile;

namespace thm {
    Parser::Parser(const TokenStream &tokenStream) : tokenStream_(tokenStream) {
        outfile.open("parser.txt");
    }

    void Parser::nextToken() {
        if (token_.type != TK_EOF)
            tokens_.push_back(token_);
        token_ = tokenStream_.next();
    }

    void Parser::ungetToken() {
        token_ = tokenStream_.unget();
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

    void Parser::finish(std::string const & str) {
        for (auto token : tokens_) {
            outfile << tokenTypeToString(token.type) << " " << token.content << std::endl;
        }
        tokens_.clear();
        outfile << "<" << str << ">" << std::endl;
    }


    void Parser::parseCompUnit() {
        bool hasDecl = true, hasFuncDef = true;
        while (hasDecl) {
            if (token_.type == CONSTTK) {
                parseDecl();
                continue;
            }
            if (token_.type == INTTK || token_.type == CHARTK) {
                nextToken();
                if (tryMatch(IDENFR)) {
                    if (token_.type == COMMA || token_.type == SEMICN || token_.type == LBRACK || token_.type == ASSIGN) {
                        ungetToken();
                        ungetToken();
                        parseDecl();
                        continue;;
                    }
                    ungetToken();
                }
                ungetToken();
            }
            hasDecl = false;
        }
        /*while (hasFuncDef) {
            if (token_.type == VOIDTK || token_.type == INTTK || token_.type == CHARTK) {
                nextToken();
                if (tryMatch(IDENFR)) {
                    matchToken(LPARENT);
                    ungetToken();
                    ungetToken();
                    ungetToken();
                    parseFuncDef();
                    continue;
                }
                ungetToken();
            }
            hasFuncDef = false;
        }*/
        //parseMainFuncDef();
        finish("CompUnit");
        outfile.close();
    }

    void Parser::parseDecl() {
        if (token_.type == CONSTTK) parseConstDecl();
        else if (token_.type == INTTK || token_.type == CHARTK) parseVarDecl();
    }

    void Parser::parseConstDecl() {
        matchToken(CONSTTK);
        parseBType();
        parseConstDef();
        while (tryMatch(COMMA)) {
            parseConstDef();
        }
        matchToken(SEMICN);
        finish("ConstDecl");
    }

    void Parser::parseBType() {
        if (token_.type == INTTK || token_.type == CHARTK) {

        } else {
            errorReporter_.error(CompilerException(UNEXPECTED_TOKEN, token_.lineno));
        }
        nextToken();
    }


    void Parser::parseConstDef() {
        matchToken(IDENFR);
        if (token_.type == LBRACK) {
            nextToken();
            if (token_.type != RBRACK) {
                parseConstExp();
            }
            matchToken(RBRACK);
        }
        matchToken(ASSIGN);
        parseConstInitVal();
        finish("ConstDef");
    }

    void Parser::parseConstInitVal() {
        if (token_.type == PLUS || token_.type == MINU || token_.type == NOT || token_.type == IDENFR || token_.type == LPARENT || token_.type == INTCON || token_.type == CHRCON) {
            parseConstExp();
        } else if (token_.type == LBRACE) {
            nextToken();
            if (token_.type == PLUS || token_.type == MINU || token_.type == NOT || token_.type == IDENFR || token_.type == LPARENT || token_.type == INTCON || token_.type == CHRCON) {
                parseConstExp();
                while (token_.type == COMMA) {
                    nextToken();
                    parseConstExp();
                }
            }
            matchToken(RBRACE);
        } else if (token_.type == STRCON) {
            matchToken(STRCON);
        } else {
            errorReporter_.error(CompilerException(UNEXPECTED_TOKEN, token_.lineno));
        }
        finish("ConstInitVal");
    }

    void Parser::parseVarDecl() {
        parseBType();
        parseVarDef();
        while (token_.type == COMMA) {
            nextToken();
            parseVarDef();
        }
        matchToken(SEMICN);
        finish("VarDecl");
    }

    void Parser::parseVarDef() {
        matchToken(IDENFR);
        if (token_.type == LBRACK) {
            nextToken();
            if (token_.type != RBRACK) {
                parseConstExp();
            }
            matchToken(RBRACK);
        }
        if (token_.type == ASSIGN) {
            nextToken();
            parseInitVal();
        }
        finish("VarDef");
    }

    void Parser::parseInitVal() {
        if (token_.type == PLUS || token_.type == MINU || token_.type == NOT || token_.type == IDENFR || token_.type == LPARENT || token_.type == INTCON || token_.type == CHRCON) {
            parseExp();
        } else if (token_.type == LBRACE) {
            nextToken();
            if (token_.type == PLUS || token_.type == MINU || token_.type == NOT || token_.type == IDENFR || token_.type == LPARENT || token_.type == INTCON || token_.type == CHRCON) {
                parseExp();
                while (token_.type == COMMA) {
                    nextToken();
                    parseExp();
                }
            }
            matchToken(RBRACE);
        } else {
            matchToken(STRCON);
        }
        finish("InitVal");
    }

    void Parser::parseFuncDef() {
        parseFuncType();
        matchToken(IDENFR);
        matchToken(LPARENT);
        if (token_.type == INTTK || token_.type == CHARTK) {
            parseFuncFParams();
        }
        matchToken(RPARENT);
        parseBlock();
        finish("FuncDef");
    }

    void Parser::parseMainFuncDef() {
        matchToken(INTTK);
        matchToken(MAINTK);
        matchToken(LPARENT);
        matchToken(RPARENT);
        parseBlock();
        finish("MainFuncDef");
    }

    void Parser::parseFuncType() {
        if (token_.type == INTTK || token_.type == CHARTK || token_.type == VOIDTK) {
        } else {
            errorReporter_.error(CompilerException(UNEXPECTED_TOKEN, token_.lineno));
        }
        nextToken();
        finish("FuncType");
    }

    void Parser::parseFuncFParams() {
        parseFuncFParam();
        while (tryMatch(COMMA)) {
            parseFuncFParam();
        }
        finish("FuncFParams");
    }

    void Parser::parseFuncFParam() {
        parseBType();
        matchToken(IDENFR);
        if (token_.type == LBRACK) {
            nextToken();
            matchToken(RBRACK);
        }
        finish("FuncFParam");
    }

    void Parser::parseBlock() {
        matchToken(LBRACE);
        while (token_.type != RBRACE) {
            parseBlockItem();
        }
        matchToken(RBRACE);
        finish("Block");
    }

    void Parser::parseBlockItem() {
        if (token_.type == INTTK || token_.type == CHARTK || token_.type == CONSTTK) {
            parseDecl();
        } else {
            parseStmt();
        }
    }

    void Parser::parseStmt() {
        if (token_.type == LBRACE) {
            parseBlock();
        } else if (token_.type == IFTK) {
            matchToken(IFTK);
            matchToken(LPARENT);
            parseCond();
            matchToken(RPARENT);
            parseStmt();
            if (token_.type == ELSETK) {
                matchToken(ELSETK);
                parseStmt();
            }
        } else if (token_.type == FORTK) {
            matchToken(FORTK);
            matchToken(LPARENT);
            if (token_.type == SEMICN) {
                matchToken(SEMICN);
            } else {
                parseForStmt();
                matchToken(SEMICN);
            }
            if (token_.type == SEMICN) {
                matchToken(SEMICN);
            } else {
                parseCond();
                matchToken(SEMICN);
            }
            if (token_.type == RPARENT) {
                matchToken(RPARENT);
            } else {
                parseForStmt();
                matchToken(RPARENT);
            }
            parseStmt();
        } else if (token_.type == BREAKTK) {
            matchToken(BREAKTK);
            matchToken(SEMICN);
        } else if (token_.type == CONTINUETK) {
            matchToken(CONTINUETK);
            matchToken(SEMICN);
        } else if (token_.type == RETURNTK) {
            nextToken();
            if (token_.type == SEMICN) {
                matchToken(SEMICN);
            } else {
                parseExp();
                matchToken(SEMICN);
            }
        } else if (token_.type == PRINTFTK) {
            matchToken(PRINTFTK);
            matchToken(LPARENT);
            matchToken(STRCON);
            while (tryMatch(COMMA)) {
                parseExp();
            }
            matchToken(RPARENT);
            matchToken(SEMICN);
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
                    parseLVal();
                    matchToken(ASSIGN);
                    if (tryMatch(GETINTTK) || tryMatch(GETCHARTK)) {
                        matchToken(LPARENT);
                        matchToken(RPARENT);
                    } else {
                        parseExp();
                    }
                } else {
                    parseExp();
                }
                matchToken(SEMICN);
            }
        }
        finish("Stmt");
    }

    void Parser::parseForStmt() {
        parseLVal();
        matchToken(ASSIGN);
        parseExp();
        finish("ForStmt");
    }

    void Parser::parseExp() {
        parseAddExp();
        finish("Exp");
    }

    void Parser::parseCond() {
        parseLOrExp();
        finish("Cond");
    }

    void Parser::parseLVal() {
        matchToken(IDENFR);
        if (token_.type == LBRACK) {
            matchToken(LBRACK);
            parseExp();
            matchToken(RBRACK);
        }
        finish("LVal");
    }

    void Parser::parsePrimaryExp() {
        if (token_.type == LPARENT) {
            matchToken(LPARENT);
            parseExp();
            matchToken(RPARENT);
        } else if (token_.type == IDENFR) {
            parseLVal();
        } else if (token_.type == INTCON) {
            parseNumber();
        } else {
            parseCharacter();
        }
        finish("PrimaryExp");
    }

    void Parser::parseNumber() {
        matchToken(INTCON);
        finish("Number");
    }

    void Parser::parseCharacter() {
        matchToken(CHRCON);
        finish("Character");
    }

    void Parser::parseUnaryExp() {
        if (token_.type == IDENFR) {
            nextToken();
            if (token_.type == LPARENT) {
                nextToken();
                if (token_.type != RPARENT) {
                    parseFuncRParams();
                }
                matchToken(RPARENT);
                finish("UnaryExp");
                return;
            }
            ungetToken();
        }
        if (token_.type == PLUS || token_.type == MINU || token_.type == NOT) {
            parseUnaryOp();
            parseUnaryExp();
        } else {
            parsePrimaryExp();
        }
        finish("UnaryExp");
    }

    void Parser::parseFuncRParams() {
        parseExp();
        while (token_.type == COMMA) {
            matchToken(COMMA);
            parseExp();
        }
        finish("FuncRParams");
    }

    void Parser::parseMulExp() {
        parseUnaryExp();
        finish("MulExp");
        while (token_.type == MULT || token_.type == DIV || token_.type == MOD) {
            nextToken();
            parseUnaryExp();
            finish("MulExp");
        }
    }

    void Parser::parseAddExp() {
        parseMulExp();
        finish("AddExp");
        while (token_.type == PLUS || token_.type == MINU) {
            nextToken();
            parseMulExp();
            finish("AddExp");
        }
    }

    void Parser::parseRelExp() {
        parseAddExp();
        finish("RelExp");
        while (token_.type == LSS || token_.type == LEQ || token_.type == GEQ || token_.type == GRE) {
            nextToken();
            parseAddExp();
            finish("RelExp");
        }
    }

    void Parser::parseEqExp() {
        parseRelExp();
        finish("EqExp");
        while (token_.type == EQL || token_.type == NEQ) {
            nextToken();
            parseRelExp();
            finish("EqExp");
        }
    }

    void Parser::parseLAndExp() {
        parseEqExp();
        finish("LAndExp");
        while (token_.type == AND) {
            nextToken();
            parseEqExp();
            finish("LAndExp");
        }
    }

    void Parser::parseLOrExp() {
        parseLAndExp();
        finish("LOrExp");
        while (token_.type == OR) {
            nextToken();
            parseLAndExp();
            finish("LOrExp");
        }
    }

    void Parser::parseConstExp() {
        parseAddExp();
        finish("ConstExp");
    }

    void Parser::parseUnaryOp() {
        switch (token_.type) {
            case PLUS:
            case MINU:
            case NOT:
                break;
            default:
                errorReporter_.error(CompilerException(UNEXPECTED_TOKEN, token_.lineno));
        }
        nextToken();
        finish("UnaryOp");
    }

} // thm