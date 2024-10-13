//
// Created by slty5 on 24-10-11.
//

#include "SymbolTableBuilder.h"

#include <iostream>
#include <bits/locale_facets_nonio.h>

#include "../core/Scope.h"
#include "../error/ErrorReporter.h"
#include "../util/overloaded.h"

namespace thm {
    SymbolTableBuilder::SymbolTableBuilder(ErrorReporter &errorReporter) : errorReporter_(errorReporter) {
    }

    void SymbolTableBuilder::pushScope(bool isReturnScope, bool requireReturnValue) {
        ++scopeNum;

        scopeStack.push(std::make_shared<Scope>(scopeNum, currentScope, std::make_shared<SymbolTable>(scopeNum, currentScope == nullptr ? nullptr : currentScope->symbolTable), isReturnScope, requireReturnValue));
        currentScope = scopeStack.top();
        scopes.push_back(currentScope);
    }

    void SymbolTableBuilder::popScope() {
        scopeStack.pop();
        currentScope = scopeStack.top();
    }

    void SymbolTableBuilder::submitSymbol(std::shared_ptr<Symbol> symbol) {
        if (!currentScope->symbolTable->addSymbol(symbol)) {
            errorReporter_.error(CompilerException(ErrorType::REDEFINITION, symbol->ident.lineno));
        }
    }

    bool SymbolTableBuilder::tryAccessSymbol(Token const &ident) const {
        if (!currentScope->symbolTable->findSymbol(ident.content)) {
            errorReporter_.error(CompilerException(ErrorType::UNDEFINED_IDENTIFIER, ident.lineno));
            return false;
        }
        return true;
    }

    std::shared_ptr<VariableSymbol> SymbolTableBuilder::getArray(std::unique_ptr<Exp> &exp) const {
        std::shared_ptr<VariableSymbol> result = nullptr;
        std::visit(overloaded{
            [&](std::unique_ptr<MulExp>& mulExp) {
                std::visit(overloaded{
                    [&](std::unique_ptr<UnaryExp>& unaryExp) {
                        std::visit(overloaded{
                            [&](std::unique_ptr<PrimaryExp>& primaryExp) {
                                std::visit(overloaded{
                                    [&](std::unique_ptr<Exp>& exp) {

                                    },
                                    [&](std::unique_ptr<LVal>& lVal) {
                                        std::shared_ptr<Symbol> symbol = currentScope->symbolTable->findSymbol(lVal->ident.content);
                                        if (lVal->exp == nullptr && symbol != nullptr && symbol->symbolType() == Symbol::VARIABLE) {
                                            std::shared_ptr<VariableSymbol> variableSymbol = std::static_pointer_cast<VariableSymbol>(symbol);
                                            if (variableSymbol->type.isArray) {
                                                result = variableSymbol;
                                            }
                                        }
                                    },
                                    [&](std::unique_ptr<Number>& number) {

                                    },
                                    [&](std::unique_ptr<Character>& character) {

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

    bool SymbolTableBuilder::endWithReturn(std::unique_ptr<Block> &block) const {
        if (block->items.empty()) return false;
        auto const& last = block->items.back();
        bool result = false;
        std::visit(overloaded{
            [&](std::unique_ptr<Stmt>& stmt) {
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
                    [&](std::unique_ptr<Exp>& exp) {},
                    [&](std::unique_ptr<Block>& block) {}
                }, stmt->stmt);
            },
            [&](std::unique_ptr<Decl>& decl) {}
        }, last->item);
        return result;
    }


    void SymbolTableBuilder::visitConstDecl(std::unique_ptr<ConstDecl> &constDecl) {
        for (auto const& def : constDecl->constDefs) {
            std::shared_ptr<VariableSymbol> symbol = std::make_shared<VariableSymbol>();
            symbol->id = ++symbolNum;
            symbol->scopeId = currentScope->scopeId;
            symbol->type.isConst = true;
            symbol->type.type = constDecl->bType->type;
            std::visit(overloaded{
                [&](ConstDef::ConstDefBasic& basic) {
                    symbol->ident = basic.ident;
                    symbol->type.isArray = false;
                },
                [&](ConstDef::ConstDefArray& array) {
                    symbol->ident = array.ident;
                    symbol->type.isArray = true;
                }
            }, def->def);
            submitSymbol(symbol);
        }
    }

    void SymbolTableBuilder::visitVarDecl(std::unique_ptr<VarDecl> &varDecl) {
        for (auto const& def : varDecl->varDefs) {
            std::shared_ptr<VariableSymbol> symbol = std::make_shared<VariableSymbol>();
            symbol->id = ++symbolNum;
            symbol->scopeId = currentScope->scopeId;
            symbol->type.isConst = false;
            symbol->type.type = varDecl->bType->type;
            std::visit(overloaded{
                [&](VarDef::VarDefBasic& basic) {
                    symbol->ident = basic.ident;
                    symbol->type.isArray = false;
                },
                [&](VarDef::VarDefArray& array) {
                    symbol->ident = array.ident;
                    symbol->type.isArray = true;
                }
            }, def->def);
            submitSymbol(symbol);
        }
    }

    void SymbolTableBuilder::visitFuncDef(std::unique_ptr<FuncDef> &funcDef) {
        std::shared_ptr<FunctionSymbol> symbol = std::make_shared<FunctionSymbol>();
        symbol->id = ++symbolNum;
        symbol->scopeId = currentScope->scopeId;
        symbol->type = funcDef->funcType->type;
        symbol->ident = funcDef->ident;
        if (funcDef->params != nullptr)
            for (auto const& param : funcDef->params->params) {
                VariableType paramType;
                paramType.type = param->bType->type;
                paramType.isConst = false;
                paramType.isArray =  param->isArray;
                symbol->paramTypes.push_back(paramType);
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
            }
        }
        funcDef->block->visitChildren(shared_from_this());
        if (currentScope->requireReturnValue && !endWithReturn(funcDef->block)) {
            errorReporter_.error(CompilerException(VAL_RETURN_NONE, funcDef->block->rBrace.lineno));
        }
        popScope();
    }

    void SymbolTableBuilder::visitMainFuncDef(std::unique_ptr<MainFuncDef> &mainFuncDef) {
        pushScope(true, true);
        mainFuncDef->block->visitChildren(shared_from_this());
        if (currentScope->requireReturnValue && !endWithReturn(mainFuncDef->block)) {
            errorReporter_.error(CompilerException(VAL_RETURN_NONE, mainFuncDef->block->rBrace.lineno));
        }
        popScope();
    }

    void SymbolTableBuilder::visitStmt(std::unique_ptr<Stmt> &stmt) {
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
            [&](std::unique_ptr<Exp>& exp) {},
            [&](std::unique_ptr<Block>& block) {}
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
            [&](std::unique_ptr<Exp>& exp) {},
            [&](std::unique_ptr<Block>& block) {}
        }, stmt->stmt);
    }

    void SymbolTableBuilder::visitCompUnit(std::unique_ptr<CompUnit> &compUnit) {
        pushScope(false, false);
        ASTVisitor::visitCompUnit(compUnit);
    }

    void SymbolTableBuilder::visitBlock(std::unique_ptr<Block> &block) {
        pushScope(false, false);
        ASTVisitor::visitBlock(block);
        popScope();
    }

    void SymbolTableBuilder::visitLVal(std::unique_ptr<LVal> &lval) {
        tryAccessSymbol(lval->ident);
        ASTVisitor::visitLVal(lval);
    }

    void SymbolTableBuilder::visitUnaryExp(std::unique_ptr<UnaryExp> &unaryExp) {
        std::visit(overloaded{
            [&](std::unique_ptr<PrimaryExp>& exp) {

            },
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
                                if (functionSymbol->paramTypes[idx].isArray != (array != nullptr)) {
                                    errorReporter_.error(CompilerException(ErrorType::MISMATCHED_TYPE, exp.ident.lineno));
                                }
                                if (array != nullptr && functionSymbol->paramTypes[idx].isArray && array->type.type != functionSymbol->paramTypes[idx].type) {
                                    errorReporter_.error(CompilerException(ErrorType::MISMATCHED_TYPE, exp.ident.lineno));
                                }
                            }
                        }
                    } else {
                        int *a = 0;
                        *a = 1;
                    }
                }
            },
            [&](UnaryExp::OpExp& exp) {

            },
        }, unaryExp->exp);
        ASTVisitor::visitUnaryExp(unaryExp);
    }
} // thm