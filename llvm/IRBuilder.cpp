//
// Created by slty5 on 24-10-27.
//

#include "IRBuilder.h"

namespace thm {
    IRBuilder::IRBuilder() {
        module = new Module();
    }

    void IRBuilder::visitConstDecl(ConstDecl* constDecl) {
        for (auto& def : constDecl->constDefs) {
            if (def->scope->scopeId == 1) {
                // replace const variables with value
            } else {

            }
        }
        ASTVisitor::visitConstDecl(constDecl);
    }

    void IRBuilder::visitVarDecl(VarDecl* varDecl) {
        for (auto& def : varDecl->varDefs) {
            if (def->scope->scopeId == 1) {
                //module->variables.push_back(new GlobalVariable(static_cast<VariableSymbol*>(def->scope->symbolTable->findSymbol(def->ident.content))));
            } else {

            }
        }
        ASTVisitor::visitVarDecl(varDecl);
    }

    void IRBuilder::visitFuncDef(FuncDef* funcDef) {
        ASTVisitor::visitFuncDef(funcDef);
    }

    void IRBuilder::visitMainFuncDef(MainFuncDef* mainFuncDef) {
        ASTVisitor::visitMainFuncDef(mainFuncDef);
    }

    void IRBuilder::visitStmt(Stmt* stmt) {
        ASTVisitor::visitStmt(stmt);
    }

    void IRBuilder::visitForStmt(ForStmt* forStmt) {
        ASTVisitor::visitForStmt(forStmt);
    }

    void IRBuilder::visitCompUnit(CompUnit* compUnit) {
        ASTVisitor::visitCompUnit(compUnit);
    }

    void IRBuilder::visitBlock(Block* block) {
        ASTVisitor::visitBlock(block);
    }

    void IRBuilder::visitLVal(LVal* lVal) {
        ASTVisitor::visitLVal(lVal);
    }

    void IRBuilder::visitUnaryExp(UnaryExp* unaryExp) {
        ASTVisitor::visitUnaryExp(unaryExp);
    }

    void IRBuilder::visitDecl(Decl* decl) {
        ASTVisitor::visitDecl(decl);
    }

    void IRBuilder::visitBType(BType* bType) {
        ASTVisitor::visitBType(bType);
    }

    void IRBuilder::visitConstDef(ConstDef* constDef) {
        ASTVisitor::visitConstDef(constDef);
    }

    void IRBuilder::visitConstInitVal(ConstInitVal* constInitVal) {
        ASTVisitor::visitConstInitVal(constInitVal);
    }

    void IRBuilder::visitVarDef(VarDef* varDef) {
        ASTVisitor::visitVarDef(varDef);
    }

    void IRBuilder::visitInitVal(InitVal* initVal) {
        ASTVisitor::visitInitVal(initVal);
    }

    void IRBuilder::visitFuncType(FuncType* funcType) {
        ASTVisitor::visitFuncType(funcType);
    }

    void IRBuilder::visitFuncFParams(FuncFParams* funcFParams) {
        ASTVisitor::visitFuncFParams(funcFParams);
    }

    void IRBuilder::visitFuncFParam(FuncFParam* funcFParam) {
        ASTVisitor::visitFuncFParam(funcFParam);
    }

    void IRBuilder::visitBlockItem(BlockItem* blockItem) {
        ASTVisitor::visitBlockItem(blockItem);
    }

    void IRBuilder::visitExp(Exp* exp) {
        ASTVisitor::visitExp(exp);
    }

    void IRBuilder::visitCond(Cond* cond) {
        ASTVisitor::visitCond(cond);
    }

    void IRBuilder::visitPrimaryExp(PrimaryExp* primaryExp) {
        ASTVisitor::visitPrimaryExp(primaryExp);
    }

    void IRBuilder::visitNumber(Number* number) {
        ASTVisitor::visitNumber(number);
    }

    void IRBuilder::visitCharacter(Character* character) {
        ASTVisitor::visitCharacter(character);
    }

    void IRBuilder::visitUnaryOp(UnaryOp* unaryOp) {
        ASTVisitor::visitUnaryOp(unaryOp);
    }

    void IRBuilder::visitFuncRParams(FuncRParams* funcRParams) {
        ASTVisitor::visitFuncRParams(funcRParams);
    }

    void IRBuilder::visitMulExp(MulExp* mulExp) {
        ASTVisitor::visitMulExp(mulExp);
    }

    void IRBuilder::visitAddExp(AddExp* addExp) {
        ASTVisitor::visitAddExp(addExp);
    }

    void IRBuilder::visitRelExp(RelExp* relExp) {
        ASTVisitor::visitRelExp(relExp);
    }

    void IRBuilder::visitEqExp(EqExp* eqExp) {
        ASTVisitor::visitEqExp(eqExp);
    }

    void IRBuilder::visitLAndExp(LAndExp* lAndExp) {
        ASTVisitor::visitLAndExp(lAndExp);
    }

    void IRBuilder::visitLOrExp(LOrExp* lOrExp) {
        ASTVisitor::visitLOrExp(lOrExp);
    }

    void IRBuilder::visitConstExp(ConstExp* constExp) {
        ASTVisitor::visitConstExp(constExp);
    }
} // thm