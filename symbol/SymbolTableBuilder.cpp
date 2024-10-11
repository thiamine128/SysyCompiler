//
// Created by slty5 on 24-10-11.
//

#include "SymbolTableBuilder.h"

#include <iostream>
#include <bits/locale_facets_nonio.h>

#include "../error/ErrorReporter.h"
#include "../util/overloaded.h"

namespace thm {
    SymbolTableBuilder::SymbolTableBuilder(ErrorReporter &errorReporter) : errorReporter_(errorReporter) {
    }

    void SymbolTableBuilder::pushScope() {
        ++scopeNum;
        symbolTableStack.push(std::make_shared<SymbolTable>(scopeNum));
        symbolTable = symbolTableStack.top();
        symbolTables.push_back(symbolTable);
    }

    void SymbolTableBuilder::popScope() {
        symbolTableStack.pop();
        symbolTable = symbolTableStack.top();
    }

    void SymbolTableBuilder::submitSymbol(std::shared_ptr<Symbol> symbol) {
        if (!symbolTable->addSymbol(symbol)) {
            errorReporter_.error(CompilerException(ErrorType::REDEFINITION, symbol->ident.lineno));
        }
    }

    bool SymbolTableBuilder::tryAccessSymbol(Token const &ident) const {
        if (!symbolTable->findSymbol(ident.content)) {
            errorReporter_.error(CompilerException(ErrorType::UNDEFINED_IDENTIFIER, ident.lineno));
            return false;
        }
        return true;
    }

    void SymbolTableBuilder::visitConstDecl(std::unique_ptr<ConstDecl> &constDecl) {
        for (auto const& def : constDecl->constDefs) {
            std::shared_ptr<VariableSymbol> symbol = std::make_shared<VariableSymbol>();
            symbol->id = ++symbolNum;
            symbol->scopeId = symbolTable->getScopeId();
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
            symbol->scopeId = symbolTable->getScopeId();
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
        symbol->scopeId = symbolTable->getScopeId();
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
        pushScope();
        if (funcDef->params != nullptr) {
            for (auto& param : funcDef->params->params) {
                std::shared_ptr<VariableSymbol> paramSymbol = std::make_shared<VariableSymbol>();
                paramSymbol->id = ++symbolNum;
                paramSymbol->scopeId = symbolTable->getScopeId();
                paramSymbol->type.isConst = false;
                paramSymbol->type.type = param->bType->type;
                paramSymbol->type.isArray = param->isArray;
                paramSymbol->ident = param->ident;

                submitSymbol(paramSymbol);
            }
        }
        funcDef->block->visitChildren(shared_from_this());
        popScope();
    }

    void SymbolTableBuilder::visitCompUnit(std::unique_ptr<CompUnit> &compUnit) {
        pushScope();
        ASTVisitor::visitCompUnit(compUnit);
    }

    void SymbolTableBuilder::visitBlock(std::unique_ptr<Block> &block) {
        pushScope();
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
                    auto symbol = symbolTable->findSymbol(exp.ident.content);
                    if (symbol->symbolType() == Symbol::FUNCTION) {
                        std::shared_ptr<FunctionSymbol> functionSymbol = std::static_pointer_cast<FunctionSymbol>(symbol);
                        if (exp.params->params.size() != functionSymbol->paramTypes.size()) {
                            errorReporter_.error(CompilerException(ErrorType::MISMATCHED_PARAMS, exp.ident.lineno));
                        } else {
                            bool match = true;
                            for (int idx = 0; idx < functionSymbol->paramTypes.size() && match; idx++) {
                                if (functionSymbol->paramTypes[idx].isArray) {

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
} // thm