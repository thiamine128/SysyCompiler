//
// Created by slty5 on 24-10-11.
//

#include "SemanticVisitor.h"

#include <iostream>
#include <bits/locale_facets_nonio.h>

#include "../core/Scope.h"
#include "../error/ErrorReporter.h"
#include "../util/util.h"

namespace thm {
    SemanticVisitor::SemanticVisitor(ErrorReporter &errorReporter) : errorReporter_(errorReporter) {

    }

    void SemanticVisitor::pushScope(bool isReturnScope, bool requireReturnValue) {
        ++scopeNum;

        scopeStack.push(std::make_shared<Scope>(scopeNum, currentScope, std::make_shared<SymbolTable>(scopeNum, currentScope == nullptr ? nullptr : currentScope->symbolTable), isReturnScope, requireReturnValue));
        currentScope = scopeStack.top();
        scopes.push_back(currentScope);
    }

    void SemanticVisitor::popScope() {
        scopeStack.pop();
        currentScope = scopeStack.top();
    }

    void SemanticVisitor::submitSymbol(std::shared_ptr<Symbol> symbol) {
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

    std::shared_ptr<VariableSymbol> SemanticVisitor::getArray(std::shared_ptr<Exp> exp) const {
        std::shared_ptr<VariableSymbol> result = nullptr;
        std::visit(overloaded{
            [&](std::shared_ptr<MulExp>& mulExp) {
                std::visit(overloaded{
                    [&](std::shared_ptr<UnaryExp>& unaryExp) {
                        std::visit(overloaded{
                            [&](std::shared_ptr<PrimaryExp>& primaryExp) {
                                std::visit(overloaded{
                                    [&](std::shared_ptr<Exp>& exp) {
                                        result = getArray(exp);
                                    },
                                    [&](std::shared_ptr<LVal>& lVal) {
                                        std::shared_ptr<Symbol> symbol = currentScope->symbolTable->findSymbol(lVal->ident.content);
                                        if (lVal->exp == nullptr && symbol != nullptr && symbol->symbolType() == Symbol::VARIABLE) {
                                            std::shared_ptr<VariableSymbol> variableSymbol = std::static_pointer_cast<VariableSymbol>(symbol);
                                            if (variableSymbol->type.isArray) {
                                                result = variableSymbol;
                                            }
                                        }
                                    },
                                    [&](std::shared_ptr<Number>& number) {

                                    },
                                    [&](std::shared_ptr<Character>& character) {

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

    bool SemanticVisitor::endWithReturn(std::shared_ptr<Block> block) const {
        if (block->items.empty()) return false;
        auto const& last = block->items.back();
        bool result = false;
        std::visit(overloaded{
            [&](std::shared_ptr<Stmt>& stmt) {
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
                    [&](std::shared_ptr<Exp>& exp) {},
                    [&](std::shared_ptr<Block>& block) {}
                }, stmt->stmt);
            },
            [&](std::shared_ptr<Decl>& decl) {}
        }, last->item);
        return result;
    }


    void SemanticVisitor::visitConstDecl(std::shared_ptr<ConstDecl> constDecl) {
        constDecl->scope = currentScope;
        constDecl->bType->visit(shared_from_this());
        for (auto const& def : constDecl->constDefs) {
            def->type = constDecl->bType->type;
            def->visit(shared_from_this());
        }
    }

    void SemanticVisitor::visitVarDecl(std::shared_ptr<VarDecl> varDecl) {
        varDecl->scope = currentScope;
        varDecl->bType->visit(shared_from_this());
        for (auto const& def : varDecl->varDefs) {
            def->type = varDecl->bType->type;
            def->visit(shared_from_this());
        }
    }

    void SemanticVisitor::visitFuncDef(std::shared_ptr<FuncDef> funcDef) {
        funcDef->scope = currentScope;
        funcDef->funcType->visit(shared_from_this());
        std::shared_ptr<FunctionSymbol> symbol = std::make_shared<FunctionSymbol>();
        symbol->id = ++symbolNum;
        symbol->scopeId = currentScope->scopeId;
        symbol->type = funcDef->funcType->type;
        symbol->ident = funcDef->ident;
        if (funcDef->params != nullptr) {
            for (auto const& param : funcDef->params->params) {
                VariableType paramType;
                paramType.type = param->bType->type;
                paramType.isConst = false;
                paramType.isArray =  param->isArray;
                symbol->paramTypes.push_back(paramType);
            }
            funcDef->params->visit(shared_from_this());
        }
        submitSymbol(symbol);
        pushScope(true, symbol->type != FunctionSymbol::VOID);
        if (funcDef->params != nullptr) {
            for (auto& param : funcDef->params->params) {
                std::shared_ptr<VariableSymbol> paramSymbol = std::make_shared<VariableSymbol>();
                paramSymbol->id = ++symbolNum;
                paramSymbol->scopeId = currentScope->scopeId;
                paramSymbol->type.isConst = false;
                paramSymbol->type.type = param->bType->type;
                paramSymbol->type.isArray = param->isArray;
                paramSymbol->ident = param->ident;

                submitSymbol(paramSymbol);
                param->visit(shared_from_this());
            }
        }

        funcDef->block->scope = currentScope;
        ASTVisitor::visitBlock(funcDef->block);

        if (currentScope->requireReturnValue && !endWithReturn(funcDef->block)) {
            errorReporter_.error(CompilerException(RETURN_NOT_FOUND, funcDef->block->rBrace.lineno));
        }
        popScope();
    }

    void SemanticVisitor::visitMainFuncDef(std::shared_ptr<MainFuncDef> mainFuncDef) {
        mainFuncDef->scope = currentScope;
        pushScope(true, true);

        mainFuncDef->block->scope = currentScope;
        ASTVisitor::visitBlock(mainFuncDef->block);

        if (currentScope->requireReturnValue && !endWithReturn(mainFuncDef->block)) {
            errorReporter_.error(CompilerException(RETURN_NOT_FOUND, mainFuncDef->block->rBrace.lineno));
        }
        popScope();
    }

    void SemanticVisitor::visitStmt(std::shared_ptr<Stmt> stmt) {
        stmt->scope = currentScope;
        std::visit(overloaded{
            [&](Stmt::StmtAssign& assign) {
                auto symbol = currentScope->symbolTable->findSymbol(assign.lVal->ident.content);
                if (symbol != nullptr && symbol->symbolType() == Symbol::VARIABLE) {
                    std::shared_ptr<VariableSymbol> variableSymbol = std::static_pointer_cast<VariableSymbol>(symbol);
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
            [&](Stmt::StmtReturn& stmtReturn) {
                if (!currentScope->canReturnWithValue() && stmtReturn.exp != nullptr) {
                    errorReporter_.error(CompilerException(VOID_RETURN_VAL, stmt->lineno));
                }
            },
            [&](Stmt::StmtRead& stmtRead) {
                auto symbol = currentScope->symbolTable->findSymbol(stmtRead.lVal->ident.content);
                if (symbol != nullptr && symbol->symbolType() == Symbol::VARIABLE) {
                    std::shared_ptr<VariableSymbol> variableSymbol = std::static_pointer_cast<VariableSymbol>(symbol);
                    if (variableSymbol->type.isConst) {
                        errorReporter_.error(CompilerException(ASSIGN_TO_CONST, stmtRead.lVal->lineno));
                    }
                }
            },
            [&](Stmt::StmtPrintf& stmtPrintf) {
                int exprs = 0;
                for (int i = 0; i < stmtPrintf.fmt.size(); i++) {
                    if (stmtPrintf.fmt[i] == '\\') {
                        i += 1;
                        continue;
                    }
                    if (stmtPrintf.fmt[i] == '%' && i + 1 < stmtPrintf.fmt.size() && (stmtPrintf.fmt[i + 1] == 'c' || stmtPrintf.fmt[i + 1] == 'd')) {
                        exprs++;
                    }
                }
                if (exprs != stmtPrintf.exps.size()) {
                    errorReporter_.error(CompilerException(MISMATCHED_PRINTF_PARAMS, stmtPrintf.printfToken.lineno));
                }
            },
            [&](std::shared_ptr<Exp>& exp) {},
            [&](std::shared_ptr<Block>& block) {}
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
            [&](std::shared_ptr<Exp>& exp) {},
            [&](std::shared_ptr<Block>& block) {}
        }, stmt->stmt);
    }

    void SemanticVisitor::visitForStmt(std::shared_ptr<ForStmt> forStmt) {
        forStmt->scope = currentScope;
        auto symbol = currentScope->symbolTable->findSymbol(forStmt->lVal->ident.content);
        if (symbol != nullptr && symbol->symbolType() == Symbol::VARIABLE) {
            std::shared_ptr<VariableSymbol> variableSymbol = std::static_pointer_cast<VariableSymbol>(symbol);
            if (variableSymbol->type.isConst) {
                errorReporter_.error(CompilerException(ASSIGN_TO_CONST, forStmt->lVal->lineno));
            }
        }
        ASTVisitor::visitForStmt(forStmt);
    }

    void SemanticVisitor::visitCompUnit(std::shared_ptr<CompUnit> compUnit) {
        pushScope(false, false);
        compUnit->scope = currentScope;
        ASTVisitor::visitCompUnit(compUnit);
    }

    void SemanticVisitor::visitBlock(std::shared_ptr<Block> block) {
        pushScope(false, false);
        block->scope = currentScope;
        ASTVisitor::visitBlock(block);
        popScope();
    }

    void SemanticVisitor::visitLVal(std::shared_ptr<LVal> lval) {
        lval->scope = currentScope;
        tryAccessSymbol(lval->ident);
        ASTVisitor::visitLVal(lval);
    }

    void SemanticVisitor::visitUnaryExp(std::shared_ptr<UnaryExp> unaryExp) {
        unaryExp->scope = currentScope;
        std::visit(overloaded{
            [&](std::shared_ptr<PrimaryExp>& exp) {},
            [&](UnaryExp::FuncExp& exp) {
                if (tryAccessSymbol(exp.ident)) {
                    auto symbol = currentScope->symbolTable->findSymbol(exp.ident.content);
                    if (symbol->symbolType() == Symbol::FUNCTION) {
                        std::shared_ptr<FunctionSymbol> functionSymbol = std::static_pointer_cast<FunctionSymbol>(symbol);
                        if ((exp.params == nullptr && functionSymbol->paramTypes.size() != 0) || (exp.params != nullptr && exp.params->params.size() != functionSymbol->paramTypes.size())) {
                            errorReporter_.error(CompilerException(ErrorType::MISMATCHED_PARAMS, exp.ident.lineno));
                        } else {
                            for (size_t idx = 0; idx < functionSymbol->paramTypes.size(); idx++) {
                                auto array = getArray(exp.params->params[idx]);
                                bool isArray = array != nullptr && array->type.isArray;

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
    }

    void SemanticVisitor::visitDecl(std::shared_ptr<Decl> decl) {
        decl->scope = currentScope;
        ASTVisitor::visitDecl(decl);
    }

    void SemanticVisitor::visitBType(std::shared_ptr<BType> bType) {
        bType->scope = currentScope;
        ASTVisitor::visitBType(bType);
    }

    void SemanticVisitor::visitConstDef(std::shared_ptr<ConstDef> constDef) {
        constDef->scope = currentScope;
        std::shared_ptr<VariableSymbol> symbol = std::make_shared<VariableSymbol>();
        symbol->id = ++symbolNum;
        symbol->scopeId = currentScope->scopeId;
        symbol->type.isConst = true;
        symbol->type.type = constDef->type;
        symbol->ident = constDef->ident;
        std::visit(overloaded{
            [&](ConstDef::ConstDefBasic& basic) {
                symbol->type.isArray = false;
            },
            [&](ConstDef::ConstDefArray& array) {
                symbol->type.isArray = true;
            }
        }, constDef->def);
        submitSymbol(symbol);
        ASTVisitor::visitConstDef(constDef);
    }

    void SemanticVisitor::visitConstInitVal(std::shared_ptr<ConstInitVal> constInitVal) {
        constInitVal->scope = currentScope;
        ASTVisitor::visitConstInitVal(constInitVal);
    }

    void SemanticVisitor::visitVarDef(std::shared_ptr<VarDef> varDef) {
        varDef->scope = currentScope;
        std::shared_ptr<VariableSymbol> symbol = std::make_shared<VariableSymbol>();
        symbol->id = ++symbolNum;
        symbol->scopeId = currentScope->scopeId;
        symbol->type.isConst = false;
        symbol->type.type = varDef->type;
        symbol->ident = varDef->ident;
        std::visit(overloaded{
            [&](VarDef::VarDefBasic& basic) {
                symbol->type.isArray = false;
            },
            [&](VarDef::VarDefArray& array) {
                symbol->type.isArray = true;
            }
        }, varDef->def);
        submitSymbol(symbol);
        ASTVisitor::visitVarDef(varDef);
    }

    void SemanticVisitor::visitInitVal(std::shared_ptr<InitVal> initVal) {
        initVal->scope = currentScope;
        ASTVisitor::visitInitVal(initVal);
    }

    void SemanticVisitor::visitFuncType(std::shared_ptr<FuncType> funcType) {
        funcType->scope = currentScope;
        ASTVisitor::visitFuncType(funcType);
    }

    void SemanticVisitor::visitFuncFParams(std::shared_ptr<FuncFParams> funcFParams) {
        funcFParams->scope = currentScope;
        ASTVisitor::visitFuncFParams(funcFParams);
    }

    void SemanticVisitor::visitFuncFParam(std::shared_ptr<FuncFParam> funcFParam) {
        funcFParam->scope = currentScope;
        ASTVisitor::visitFuncFParam(funcFParam);
    }

    void SemanticVisitor::visitBlockItem(std::shared_ptr<BlockItem> blockItem) {
        blockItem->scope = currentScope;
        ASTVisitor::visitBlockItem(blockItem);
    }

    void SemanticVisitor::visitExp(std::shared_ptr<Exp> exp) {
        exp->scope = currentScope;
        ASTVisitor::visitExp(exp);
    }

    void SemanticVisitor::visitCond(std::shared_ptr<Cond> cond) {
        cond->scope = currentScope;
        ASTVisitor::visitCond(cond);
    }

    void SemanticVisitor::visitPrimaryExp(std::shared_ptr<PrimaryExp> primaryExp) {
        primaryExp->scope = currentScope;
        ASTVisitor::visitPrimaryExp(primaryExp);
    }

    void SemanticVisitor::visitNumber(std::shared_ptr<Number> number) {
        number->scope = currentScope;
        ASTVisitor::visitNumber(number);
    }

    void SemanticVisitor::visitCharacter(std::shared_ptr<Character> character) {
        character->scope = currentScope;
        ASTVisitor::visitCharacter(character);
    }

    void SemanticVisitor::visitUnaryOp(std::shared_ptr<UnaryOp> unaryOp) {
        unaryOp->scope = currentScope;
        ASTVisitor::visitUnaryOp(unaryOp);
    }

    void SemanticVisitor::visitFuncRParams(std::shared_ptr<FuncRParams> funcRParams) {
        funcRParams->scope = currentScope;
        ASTVisitor::visitFuncRParams(funcRParams);
    }

    void SemanticVisitor::visitMulExp(std::shared_ptr<MulExp> mulExp) {
        mulExp->scope = currentScope;
        ASTVisitor::visitMulExp(mulExp);
    }

    void SemanticVisitor::visitAddExp(std::shared_ptr<AddExp> addExp) {
        addExp->scope = currentScope;
        ASTVisitor::visitAddExp(addExp);
    }

    void SemanticVisitor::visitRelExp(std::shared_ptr<RelExp> relExp) {
        relExp->scope = currentScope;
        ASTVisitor::visitRelExp(relExp);
    }

    void SemanticVisitor::visitEqExp(std::shared_ptr<EqExp> eqExp) {
        eqExp->scope = currentScope;
        ASTVisitor::visitEqExp(eqExp);
    }

    void SemanticVisitor::visitLAndExp(std::shared_ptr<LAndExp> landExp) {
        landExp->scope = currentScope;
        ASTVisitor::visitLAndExp(landExp);
    }

    void SemanticVisitor::visitLOrExp(std::shared_ptr<LOrExp> lorExp) {
        lorExp->scope = currentScope;
        ASTVisitor::visitLOrExp(lorExp);
    }

    void SemanticVisitor::visitConstExp(std::shared_ptr<ConstExp> constExp) {
        constExp->scope = currentScope;
        ASTVisitor::visitConstExp(constExp);
    }
} // thm