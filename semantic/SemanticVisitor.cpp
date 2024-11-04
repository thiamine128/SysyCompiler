//
// Created by slty5 on 24-10-11.
//

#include "SemanticVisitor.h"

#include <cmath>
#include <iostream>
#include <math.h>

#include "../core/Scope.h"
#include "../error/ErrorReporter.h"
#include "../util/util.h"

namespace thm {
    SemanticVisitor::SemanticVisitor(ErrorReporter &errorReporter) : errorReporter_(errorReporter) {
    }

    void SemanticVisitor::pushScope(bool isReturnScope, bool requireReturnValue) {
        ++scopeNum;

        scopeStack.push(new Scope(scopeNum, currentScope, new SymbolTable(scopeNum, currentScope == nullptr ? nullptr : currentScope->symbolTable), isReturnScope, requireReturnValue));
        currentScope = scopeStack.top();
        scopes.push_back(currentScope);
    }

    void SemanticVisitor::popScope() {
        scopeStack.pop();
        currentScope = scopeStack.top();
    }

    void SemanticVisitor::submitSymbol(Symbol* symbol) const {
        if (!currentScope->symbolTable->addSymbol(symbol)) {
            errorReporter_.error(CompilerException(ErrorType::REDEFINITION, symbol->ident.lineno));
        }
    }

    bool SemanticVisitor::tryAccessSymbol(Token const &ident) const {
        if (!currentScope->symbolTable->findSymbol(ident.content)) {
            errorReporter_.error(CompilerException(ErrorType::UNDEFINED_IDENTIFIER, ident.lineno));
            return false;
        }
        return true;
    }

    VariableSymbol* SemanticVisitor::getArray(const Exp* exp) const {
        VariableSymbol* result = nullptr;
        std::visit(overloaded{
            [&](MulExp* mulExp) {
                std::visit(overloaded{
                    [&](UnaryExp* unaryExp) {
                        std::visit(overloaded{
                            [&](PrimaryExp* primaryExp) {
                                std::visit(overloaded{
                                    [&](const Exp* subExp) {
                                        result = getArray(subExp);
                                    },
                                    [&](const LVal* lVal) {
                                        Symbol* symbol = currentScope->symbolTable->findSymbol(lVal->ident.content);
                                        if (lVal->exp == nullptr && symbol != nullptr && symbol->symbolType() == Symbol::VARIABLE) {
                                            VariableSymbol* variableSymbol = static_cast<VariableSymbol*>(symbol);
                                            if (variableSymbol->type.isArray) {
                                                result = variableSymbol;
                                            }
                                        }
                                    },
                                    [&](Number* number) {

                                    },
                                    [&](Character* character) {

                                    }
                                }, primaryExp->primaryExp);
                            },
                            [&](UnaryExp::FuncExp& funcExp) {},
                            [&](UnaryExp::OpExp& opExp) {},
                        }, unaryExp->exp);
                    },
                    [&](MulExp::OpExp& opExp) {}
                }, mulExp->exp);
            },
            [&](AddExp::OpExp& opExp) {}
        }, exp->addExp->exp);
        return result;
    }

    bool SemanticVisitor::endWithReturn(const Block* block) {
        if (block->items.empty()) return false;
        auto const& last = block->items.back();
        bool result = false;
        std::visit(overloaded{
            [&](Stmt* stmt) {
                std::visit(overloaded{
                    [&](Stmt::StmtAssign& assign) {},
                    [&](Stmt::StmtIf& stmtIf) {},
                    [&](Stmt::StmtFor& stmtFor) {},
                    [&](Stmt::BreakOrContinue& breakOrContinue) {},
                    [&](Stmt::StmtReturn& stmtReturn) {
                        result = true;
                    },
                    [&](Stmt::StmtRead& stmtRead) {},
                    [&](Stmt::StmtPrintf& stmtPrintf) {},
                    [&](Exp* exp) {},
                    [&](Block* subBlock) {}
                }, stmt->stmt);
            },
            [&](Decl* decl) {}
        }, last->item);
        return result;
    }


    void SemanticVisitor::visitConstDecl(ConstDecl* constDecl) {
        constDecl->scope = currentScope;
        constDecl->bType->visit(this);
        for (auto const& def : constDecl->constDefs) {
            def->type = constDecl->bType->type;
            def->visit(this);
        }
    }

    void SemanticVisitor::visitVarDecl(VarDecl* varDecl) {
        varDecl->scope = currentScope;
        varDecl->bType->visit(this);
        for (auto const& def : varDecl->varDefs) {
            def->type = varDecl->bType->type;
            def->visit(this);
        }
    }

    void SemanticVisitor::visitFuncDef(FuncDef* funcDef) {
        funcDef->scope = currentScope;
        funcDef->funcType->visit(this);
        FunctionSymbol* symbol = new FunctionSymbol();
        symbol->id = ++symbolNum;
        symbol->scopeId = currentScope->scopeId;
        symbol->type = funcDef->funcType->type;
        symbol->ident = funcDef->ident;
        funcDef->symbol = symbol;
        if (funcDef->params != nullptr) {
            for (auto const& param : funcDef->params->params) {
                VariableType paramType;
                paramType.type = param->bType->type;
                paramType.isConst = false;
                paramType.isArray =  param->isArray;
                symbol->paramTypes.push_back(paramType);
            }
        }
        submitSymbol(symbol);

        pushScope(true, symbol->type != FunctionSymbol::VOID);
        if (funcDef->params != nullptr) {
            funcDef->params->scope = currentScope;
            funcDef->params->visit(this);
        }
        funcDef->block->scope = currentScope;
        funcDef->block->visit(this);
        popScope();

        if (funcDef->block->scope->requireReturnValue && !endWithReturn(funcDef->block)) {
            errorReporter_.error(CompilerException(RETURN_NOT_FOUND, funcDef->block->rBrace.lineno));
        }
    }

    void SemanticVisitor::visitMainFuncDef(MainFuncDef* mainFuncDef) {
        mainFuncDef->scope = currentScope;
        pushScope(true, true);

        mainFuncDef->block->scope = currentScope;
        mainFuncDef->block->visit(this);

        if (currentScope->requireReturnValue && !endWithReturn(mainFuncDef->block)) {
            errorReporter_.error(CompilerException(RETURN_NOT_FOUND, mainFuncDef->block->rBrace.lineno));
        }
        popScope();
    }

    void SemanticVisitor::visitStmt(Stmt* stmt) {
        stmt->scope = currentScope;
        std::visit(overloaded{
            [&](const Stmt::StmtAssign& assign) {
                auto symbol = currentScope->symbolTable->findSymbol(assign.lVal->ident.content);
                if (symbol != nullptr && symbol->symbolType() == Symbol::VARIABLE) {
                    VariableSymbol* variableSymbol = static_cast<VariableSymbol*>(symbol);
                    if (variableSymbol->type.isConst) {
                        errorReporter_.error(CompilerException(ASSIGN_TO_CONST, assign.lVal->lineno));
                    }
                }
            },
            [&](Stmt::StmtIf& stmtIf) {},
            [&](Stmt::StmtFor& stmtFor) {
                loops++;
            },
            [&](Stmt::BreakOrContinue& breakOrContinue) {
                if (loops == 0) {
                    errorReporter_.error(CompilerException(UNEXPECTED_BREAK_CONTINUE, stmt->lineno));
                }
            },
            [&](const Stmt::StmtReturn& stmtReturn) {
                if (!currentScope->canReturnWithValue() && stmtReturn.exp != nullptr) {
                    errorReporter_.error(CompilerException(VOID_RETURN_VAL, stmt->lineno));
                }
            },
            [&](const Stmt::StmtRead& stmtRead) {
                auto symbol = currentScope->symbolTable->findSymbol(stmtRead.lVal->ident.content);
                if (symbol != nullptr && symbol->symbolType() == Symbol::VARIABLE) {
                    VariableSymbol* variableSymbol = static_cast<VariableSymbol*>(symbol);
                    if (variableSymbol->type.isConst) {
                        errorReporter_.error(CompilerException(ASSIGN_TO_CONST, stmtRead.lVal->lineno));
                    }
                }
            },
            [&](const Stmt::StmtPrintf& stmtPrintf) {
                int expressions = 0;
                for (int i = 0; i < stmtPrintf.fmt.size(); i++) {
                    if (stmtPrintf.fmt[i] == '\\') {
                        i += 1;
                        continue;
                    }
                    if (stmtPrintf.fmt[i] == '%' && i + 1 < stmtPrintf.fmt.size() && (stmtPrintf.fmt[i + 1] == 'c' || stmtPrintf.fmt[i + 1] == 'd')) {
                        expressions++;
                    }
                }
                if (expressions != stmtPrintf.exps.size()) {
                    errorReporter_.error(CompilerException(MISMATCHED_PRINTF_PARAMS, stmtPrintf.printfToken.lineno));
                }
            },
            [&](Exp* exp) {},
            [&](Block* block) {
                pushScope(false, false);
                block->scope = currentScope;
            }
        }, stmt->stmt);
        ASTVisitor::visitStmt(stmt);
        std::visit(overloaded{
            [&](Stmt::StmtAssign& assign) {},
            [&](Stmt::StmtIf& stmtIf) {},
            [&](Stmt::StmtFor& stmtFor) {
                loops--;
            },
            [&](Stmt::BreakOrContinue& breakOrContinue) {},
            [&](Stmt::StmtReturn& stmtReturn) {},
            [&](Stmt::StmtRead& stmtRead) {},
            [&](Stmt::StmtPrintf& stmtPrintf) {},
            [&](Exp* exp) {},
            [&](Block* block) {
                popScope();
            }
        }, stmt->stmt);
    }

    void SemanticVisitor::visitForStmt(ForStmt* forStmt) {
        forStmt->scope = currentScope;
        auto symbol = currentScope->symbolTable->findSymbol(forStmt->lVal->ident.content);
        if (symbol != nullptr && symbol->symbolType() == Symbol::VARIABLE) {
            VariableSymbol* variableSymbol = static_cast<VariableSymbol*>(symbol);
            if (variableSymbol->type.isConst) {
                errorReporter_.error(CompilerException(ASSIGN_TO_CONST, forStmt->lVal->lineno));
            }
        }
        ASTVisitor::visitForStmt(forStmt);
    }

    void SemanticVisitor::visitCompUnit(CompUnit* compUnit) {
        pushScope(false, false);
        compUnit->scope = currentScope;
        ASTVisitor::visitCompUnit(compUnit);
    }

    void SemanticVisitor::visitBlock(Block* block) {
        ASTVisitor::visitBlock(block);
    }

    void SemanticVisitor::visitLVal(LVal* lVal) {
        lVal->scope = currentScope;
        tryAccessSymbol(lVal->ident);
        lVal->symbol = static_cast<VariableSymbol*>(currentScope->symbolTable->findSymbol(lVal->ident.content));
        ASTVisitor::visitLVal(lVal);
        lVal->evalConst();
    }

    void SemanticVisitor::visitUnaryExp(UnaryExp* unaryExp) {
        unaryExp->scope = currentScope;
        std::visit(overloaded{
            [&](PrimaryExp* exp) {},
            [&](const UnaryExp::FuncExp& exp) {
                if (tryAccessSymbol(exp.ident)) {
                    auto symbol = currentScope->symbolTable->findSymbol(exp.ident.content);
                    if (symbol->symbolType() == Symbol::FUNCTION) {
                        FunctionSymbol* functionSymbol = static_cast<FunctionSymbol*>(symbol);
                        if ((exp.params == nullptr && functionSymbol->paramTypes.size() != 0) || (exp.params != nullptr && exp.params->params.size() != functionSymbol->paramTypes.size())) {
                            errorReporter_.error(CompilerException(ErrorType::MISMATCHED_PARAMS, exp.ident.lineno));
                        } else if (exp.params != nullptr) {
                            for (size_t idx = 0; idx < functionSymbol->paramTypes.size(); idx++) {
                                auto array = getArray(exp.params->params[idx]);
                                bool isArray = false;
                                if (array != nullptr) {
                                    isArray = array->type.isArray;
                                }

                                if (functionSymbol->paramTypes[idx].isArray ^ isArray) {
                                    errorReporter_.error(CompilerException(ErrorType::MISMATCHED_TYPE, exp.ident.lineno));
                                } else if (isArray && array->type.type != functionSymbol->paramTypes[idx].type) {
                                    errorReporter_.error(CompilerException(ErrorType::MISMATCHED_TYPE, exp.ident.lineno));
                                }
                            }
                        }
                    }
                }
            },
            [&](UnaryExp::OpExp& exp) {

            },
        }, unaryExp->exp);
        ASTVisitor::visitUnaryExp(unaryExp);
        unaryExp->evalConst();
    }

    void SemanticVisitor::visitDecl(Decl* decl) {
        decl->scope = currentScope;
        ASTVisitor::visitDecl(decl);
    }

    void SemanticVisitor::visitBType(BType* bType) {
        bType->scope = currentScope;
        ASTVisitor::visitBType(bType);
    }

    void SemanticVisitor::visitConstDef(ConstDef* constDef) {
        constDef->scope = currentScope;
        VariableSymbol* symbol = new VariableSymbol();
        symbol->id = ++symbolNum;
        symbol->scopeId = currentScope->scopeId;
        symbol->type.isConst = true;
        symbol->type.type = constDef->type;
        symbol->ident = constDef->ident;
        constDef->symbol = symbol;
        constDef->lazyArrayLen = false;
        std::visit(overloaded{
            [&](ConstDef::ConstDefBasic& basic) {
                symbol->type.isArray = false;
            },
            [&](ConstDef::ConstDefArray& array) {
                symbol->type.isArray = true;
                if (array.size != nullptr) {
                    array.size->visit(this);
                    symbol->type.arrayLen = array.size->constVal;
                } else {
                    constDef->lazyArrayLen = true;
                }
            }
        }, constDef->def);
        submitSymbol(symbol);
        if (constDef->val != nullptr) {
            constDef->val->constDef = constDef;
            constDef->val->visit(this);
        } else {
            symbol->hasInit = false;
        }
    }

    void SemanticVisitor::visitConstInitVal(ConstInitVal* constInitVal) {
        constInitVal->scope = currentScope;
        VariableSymbol* symbol = constInitVal->constDef->symbol;
        symbol->hasInit = true;
        std::visit(overloaded{
            [&](ConstInitVal::ConstInitValBasic& basic) {
                basic.exp->visit(this);
                symbol->initVal = basic.exp->constVal;
                if (symbol->type.type == VariableType::CHAR) {
                    symbol->initVal &= 0xff;
                }
            },
            [&](ConstInitVal::ConstInitValArray& array) {
                if (!constInitVal->constDef->lazyArrayLen) {
                    symbol->initVals.resize(symbol->type.arrayLen);
                } else {
                    symbol->type.arrayLen = array.exps.size();
                    symbol->initVals.resize(array.exps.size());
                }
                for (int i = 0; i < array.exps.size(); i++) {
                    array.exps[i]->visit(this);
                    if (i >= symbol->type.arrayLen) {
                        break;
                    }
                    symbol->initVals[i] = array.exps[i]->constVal;
                    if (symbol->type.type == VariableType::CHAR) {
                        symbol->initVals[i] &= 0xff;
                    }
                }
            },
            [&](std::string& str) {
                std::string escaped = fromRaw(str.c_str());
                if (!constInitVal->constDef->lazyArrayLen) {
                    symbol->initVals.resize(symbol->type.arrayLen);
                } else {
                    symbol->type.arrayLen = escaped.size() + 1;
                    symbol->initVals.resize(escaped.size() + 1);
                }

                for (int i = 0; i < escaped.size(); i++) {
                    if (i >= symbol->type.arrayLen) {
                        break;
                    }
                    symbol->initVals[i] = escaped[i];
                }
            }
        }, constInitVal->val);
    }

    void SemanticVisitor::visitVarDef(VarDef* varDef) {
        varDef->scope = currentScope;
        VariableSymbol* symbol = new VariableSymbol();
        symbol->id = ++symbolNum;
        symbol->scopeId = currentScope->scopeId;
        symbol->type.isConst = false;
        symbol->type.type = varDef->type;
        symbol->ident = varDef->ident;
        varDef->symbol = symbol;
        varDef->lazyArrayLen = false;
        std::visit(overloaded{
            [&](VarDef::VarDefBasic& basic) {
                symbol->type.isArray = false;
            },
            [&](VarDef::VarDefArray& array) {
                symbol->type.isArray = true;
                if (array.size != nullptr) {
                    array.size->visit(this);
                    symbol->type.arrayLen = array.size->constVal;
                } else {
                    varDef->lazyArrayLen = true;
                }
            }
        }, varDef->def);
        submitSymbol(symbol);
        if (varDef->val != nullptr) {
            varDef->val->varDef = varDef;
            varDef->val->visit(this);
        } else {
            symbol->hasInit = false;
        }
    }

    void SemanticVisitor::visitInitVal(InitVal* initVal) {
        initVal->scope = currentScope;
        VariableSymbol* symbol = initVal->varDef->symbol;
        symbol->hasInit = true;
        std::visit(overloaded{
            [&](InitVal::InitValBasic& basic) {
                basic.exp->visit(this);
                if (basic.exp->isConst) {
                    symbol->initVal = basic.exp->constVal;

                    if (symbol->type.type == VariableType::CHAR) {
                        symbol->initVal &= 0xff;
                    }
                }
            },
            [&](InitVal::InitValArray& array) {
                if (!initVal->varDef->lazyArrayLen) {
                    symbol->initVals.resize(symbol->type.arrayLen);
                } else {
                    symbol->type.arrayLen = array.exps.size();
                    symbol->initVals.resize(array.exps.size());
                }
                for (int i = 0; i < array.exps.size(); i++) {
                    if (i >= symbol->type.arrayLen) {
                        break;
                    }
                    array.exps[i]->visit(this);
                    symbol->initVals[i] = array.exps[i]->constVal;
                    if (symbol->type.type == VariableType::CHAR) {
                        symbol->initVals[i] &= 0xff;
                    }
                }
            },
            [&](std::string& str) {
                std::string escaped = fromRaw(str.c_str());
                if (!initVal->varDef->lazyArrayLen) {
                    symbol->initVals.resize(symbol->type.arrayLen);
                } else {
                    symbol->type.arrayLen = escaped.size() + 1;
                    symbol->initVals.resize(escaped.size() + 1);
                }
                for (int i = 0; i < escaped.size(); i++) {
                    if (i >= symbol->type.arrayLen) {
                        break;
                    }
                    symbol->initVals[i] = escaped[i];
                }
            }
        }, initVal->val);
    }

    void SemanticVisitor::visitFuncType(FuncType* funcType) {
        funcType->scope = currentScope;
        ASTVisitor::visitFuncType(funcType);
    }

    void SemanticVisitor::visitFuncFParams(FuncFParams* funcFParams) {
        ASTVisitor::visitFuncFParams(funcFParams);
    }

    void SemanticVisitor::visitFuncFParam(FuncFParam* funcFParam) {
        VariableSymbol* paramSymbol = new VariableSymbol();
        ASTVisitor::visitFuncFParam(funcFParam);
        paramSymbol->id = ++symbolNum;
        paramSymbol->scopeId = currentScope->scopeId;
        paramSymbol->type.isConst = false;
        paramSymbol->type.type = funcFParam->bType->type;
        paramSymbol->type.isArray = funcFParam->isArray;
        paramSymbol->ident = funcFParam->ident;
        funcFParam->symbol = paramSymbol;
        submitSymbol(paramSymbol);
    }

    void SemanticVisitor::visitBlockItem(BlockItem* blockItem) {
        blockItem->scope = currentScope;
        ASTVisitor::visitBlockItem(blockItem);
    }

    void SemanticVisitor::visitExp(Exp* exp) {
        exp->scope = currentScope;
        ASTVisitor::visitExp(exp);
        exp->evalConst();
    }

    void SemanticVisitor::visitCond(Cond* cond) {
        cond->scope = currentScope;
        ASTVisitor::visitCond(cond);
    }

    void SemanticVisitor::visitPrimaryExp(PrimaryExp* primaryExp) {
        primaryExp->scope = currentScope;
        ASTVisitor::visitPrimaryExp(primaryExp);
        primaryExp->evalConst();
    }

    void SemanticVisitor::visitNumber(Number* number) {
        number->scope = currentScope;
        ASTVisitor::visitNumber(number);
    }

    void SemanticVisitor::visitCharacter(Character* character) {
        character->scope = currentScope;
        ASTVisitor::visitCharacter(character);
    }

    void SemanticVisitor::visitUnaryOp(UnaryOp* unaryOp) {
        unaryOp->scope = currentScope;
        ASTVisitor::visitUnaryOp(unaryOp);
    }

    void SemanticVisitor::visitFuncRParams(FuncRParams* funcRParams) {
        funcRParams->scope = currentScope;
        ASTVisitor::visitFuncRParams(funcRParams);
    }

    void SemanticVisitor::visitMulExp(MulExp* mulExp) {
        mulExp->scope = currentScope;
        ASTVisitor::visitMulExp(mulExp);
        mulExp->evalConst();
    }

    void SemanticVisitor::visitAddExp(AddExp* addExp) {
        addExp->scope = currentScope;
        ASTVisitor::visitAddExp(addExp);
        addExp->evalConst();
    }

    void SemanticVisitor::visitRelExp(RelExp* relExp) {
        relExp->scope = currentScope;
        ASTVisitor::visitRelExp(relExp);
    }

    void SemanticVisitor::visitEqExp(EqExp* eqExp) {
        eqExp->scope = currentScope;
        ASTVisitor::visitEqExp(eqExp);
    }

    void SemanticVisitor::visitLAndExp(LAndExp* landExp) {
        landExp->scope = currentScope;
        ASTVisitor::visitLAndExp(landExp);
    }

    void SemanticVisitor::visitLOrExp(LOrExp* lorExp) {
        lorExp->scope = currentScope;
        ASTVisitor::visitLOrExp(lorExp);
    }

    void SemanticVisitor::visitConstExp(ConstExp* constExp) {
        constExp->scope = currentScope;
        ASTVisitor::visitConstExp(constExp);
        constExp->evalConst();
    }
} // thm