//
// Created by slty5 on 24-10-27.
//

#ifndef IRBUILDER_H
#define IRBUILDER_H
#include "LLVM.h"
#include "../parser/ASTVisitor.h"

namespace thm {

class IRBuilder : public ASTVisitor {
public:
    Module* module;

    IRBuilder();
    void visitConstDecl(ConstDecl* constDecl) override;
    void visitVarDecl(VarDecl* varDecl) override;
    void visitFuncDef(FuncDef* funcDef) override;
    void visitMainFuncDef(MainFuncDef* mainFuncDef) override;
    void visitStmt(Stmt* stmt) override;
    void visitForStmt(ForStmt* forStmt) override;
    void visitCompUnit(CompUnit* compUnit) override;
    void visitBlock(Block* block) override;
    void visitLVal(LVal* lVal) override;
    void visitUnaryExp(UnaryExp* unaryExp) override;
    void visitDecl(Decl* decl) override;
    void visitBType(BType* bType) override;
    void visitConstDef(ConstDef* constDef) override;
    void visitConstInitVal(ConstInitVal* constInitVal) override;
    void visitVarDef(VarDef* varDef) override;
    void visitInitVal(InitVal* initVal) override;
    void visitFuncType(FuncType* funcType) override;
    void visitFuncFParams(FuncFParams* funcFParams) override;
    void visitFuncFParam(FuncFParam* funcFParam) override;
    void visitBlockItem(BlockItem* blockItem) override;
    void visitExp(Exp* exp) override;
    void visitCond(Cond* cond) override;
    void visitPrimaryExp(PrimaryExp* primaryExp) override;
    void visitNumber(Number* number) override;
    void visitCharacter(Character* character) override;
    void visitUnaryOp(UnaryOp* unaryOp) override;
    void visitFuncRParams(FuncRParams* funcRParams) override;
    void visitMulExp(MulExp* mulExp) override;
    void visitAddExp(AddExp* addExp) override;
    void visitRelExp(RelExp* relExp) override;
    void visitEqExp(EqExp* eqExp) override;
    void visitLAndExp(LAndExp* lAndExp) override;
    void visitLOrExp(LOrExp* lOrExp) override;
    void visitConstExp(ConstExp* constExp) override;
};

} // thm

#endif //IRBUILDER_H
