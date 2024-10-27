//
// Created by slty5 on 24-10-27.
//

#include "IRBuilder.h"

#include "../util/util.h"

namespace thm {
    IRBuilder::IRBuilder() {
        module = std::make_shared<Module>();
    }

    void IRBuilder::visitConstDecl(std::shared_ptr<ConstDecl> constDecl) {
        for (auto& def : constDecl->constDefs) {
            if (def->scope->scopeId == 1) {
                // replace const variables with value
            } else {

            }
        }
        ASTVisitor::visitConstDecl(constDecl);
    }

    void IRBuilder::visitVarDecl(std::shared_ptr<VarDecl> varDecl) {
        for (auto& def : varDecl->varDefs) {
            if (def->scope->scopeId == 1) {
                module->variables.push_back(std::make_shared<GlobalVariable>(def->scope->symbolTable->findSymbol(def->ident.content)));
            } else {

            }
        }
        ASTVisitor::visitVarDecl(varDecl);
    }

    void IRBuilder::visitFuncDef(std::shared_ptr<FuncDef> funcDef) {
        ASTVisitor::visitFuncDef(funcDef);
    }

    void IRBuilder::visitMainFuncDef(std::shared_ptr<MainFuncDef> mainFuncDef) {
        ASTVisitor::visitMainFuncDef(mainFuncDef);
    }

    void IRBuilder::visitStmt(std::shared_ptr<Stmt> stmt) {
        ASTVisitor::visitStmt(stmt);
    }

    void IRBuilder::visitForStmt(std::shared_ptr<ForStmt> forStmt) {
        ASTVisitor::visitForStmt(forStmt);
    }

    void IRBuilder::visitCompUnit(std::shared_ptr<CompUnit> compUnit) {
        ASTVisitor::visitCompUnit(compUnit);
    }

    void IRBuilder::visitBlock(std::shared_ptr<Block> block) {
        ASTVisitor::visitBlock(block);
    }

    void IRBuilder::visitLVal(std::shared_ptr<LVal> lVal) {
        ASTVisitor::visitLVal(lVal);
    }

    void IRBuilder::visitUnaryExp(std::shared_ptr<UnaryExp> unaryExp) {
        ASTVisitor::visitUnaryExp(unaryExp);
    }

    void IRBuilder::visitDecl(std::shared_ptr<Decl> decl) {
        ASTVisitor::visitDecl(decl);
    }

    void IRBuilder::visitBType(std::shared_ptr<BType> bType) {
        ASTVisitor::visitBType(bType);
    }

    void IRBuilder::visitConstDef(std::shared_ptr<ConstDef> constDef) {
        ASTVisitor::visitConstDef(constDef);
    }

    void IRBuilder::visitConstInitVal(std::shared_ptr<ConstInitVal> constInitVal) {
        ASTVisitor::visitConstInitVal(constInitVal);
    }

    void IRBuilder::visitVarDef(std::shared_ptr<VarDef> varDef) {
        ASTVisitor::visitVarDef(varDef);
    }

    void IRBuilder::visitInitVal(std::shared_ptr<InitVal> initVal) {
        ASTVisitor::visitInitVal(initVal);
    }

    void IRBuilder::visitFuncType(std::shared_ptr<FuncType> funcType) {
        ASTVisitor::visitFuncType(funcType);
    }

    void IRBuilder::visitFuncFParams(std::shared_ptr<FuncFParams> funcFParams) {
        ASTVisitor::visitFuncFParams(funcFParams);
    }

    void IRBuilder::visitFuncFParam(std::shared_ptr<FuncFParam> funcFParam) {
        ASTVisitor::visitFuncFParam(funcFParam);
    }

    void IRBuilder::visitBlockItem(std::shared_ptr<BlockItem> blockItem) {
        ASTVisitor::visitBlockItem(blockItem);
    }

    void IRBuilder::visitExp(std::shared_ptr<Exp> exp) {
        ASTVisitor::visitExp(exp);
    }

    void IRBuilder::visitCond(std::shared_ptr<Cond> cond) {
        ASTVisitor::visitCond(cond);
    }

    void IRBuilder::visitPrimaryExp(std::shared_ptr<PrimaryExp> primaryExp) {
        ASTVisitor::visitPrimaryExp(primaryExp);
    }

    void IRBuilder::visitNumber(std::shared_ptr<Number> number) {
        ASTVisitor::visitNumber(number);
    }

    void IRBuilder::visitCharacter(std::shared_ptr<Character> character) {
        ASTVisitor::visitCharacter(character);
    }

    void IRBuilder::visitUnaryOp(std::shared_ptr<UnaryOp> unaryOp) {
        ASTVisitor::visitUnaryOp(unaryOp);
    }

    void IRBuilder::visitFuncRParams(std::shared_ptr<FuncRParams> funcRParams) {
        ASTVisitor::visitFuncRParams(funcRParams);
    }

    void IRBuilder::visitMulExp(std::shared_ptr<MulExp> mulExp) {
        ASTVisitor::visitMulExp(mulExp);
    }

    void IRBuilder::visitAddExp(std::shared_ptr<AddExp> addExp) {
        ASTVisitor::visitAddExp(addExp);
    }

    void IRBuilder::visitRelExp(std::shared_ptr<RelExp> relExp) {
        ASTVisitor::visitRelExp(relExp);
    }

    void IRBuilder::visitEqExp(std::shared_ptr<EqExp> eqExp) {
        ASTVisitor::visitEqExp(eqExp);
    }

    void IRBuilder::visitLAndExp(std::shared_ptr<LAndExp> lAndExp) {
        ASTVisitor::visitLAndExp(lAndExp);
    }

    void IRBuilder::visitLOrExp(std::shared_ptr<LOrExp> lOrExp) {
        ASTVisitor::visitLOrExp(lOrExp);
    }

    void IRBuilder::visitConstExp(std::shared_ptr<ConstExp> constExp) {
        ASTVisitor::visitConstExp(constExp);
    }
} // thm