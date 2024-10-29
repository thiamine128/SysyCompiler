//
// Created by slty5 on 24-10-11.
//

#ifndef ASTVISITOR_H
#define ASTVISITOR_H

#include "AbstractSyntaxTree.h"

namespace thm {

class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;

    virtual void visitCompUnit(CompUnit* compUnit);
    virtual void visitDecl(Decl* decl);
    virtual void visitConstDecl(ConstDecl* constDecl);
    virtual void visitBType(BType* bType);
    virtual void visitConstDef(ConstDef* constDef);
    virtual void visitConstInitVal(ConstInitVal* constInitVal);
    virtual void visitVarDecl(VarDecl* varDecl);
    virtual void visitVarDef(VarDef* varDef);
    virtual void visitInitVal(InitVal* initVal);
    virtual void visitFuncDef(FuncDef* funcDef);
    virtual void visitMainFuncDef(MainFuncDef* mainFuncDef);
    virtual void visitFuncType(FuncType* funcType);
    virtual void visitFuncFParams(FuncFParams* funcFParams);
    virtual void visitFuncFParam(FuncFParam* funcFParam);
    virtual void visitBlock(Block* block);
    virtual void visitBlockItem(BlockItem* blockItem);
    virtual void visitStmt(Stmt* stmt);
    virtual void visitForStmt(ForStmt* forStmt);
    virtual void visitExp(Exp* exp);
    virtual void visitCond(Cond* cond);
    virtual void visitLVal(LVal* lVal);
    virtual void visitPrimaryExp(PrimaryExp* primaryExp);
    virtual void visitNumber(Number* number);
    virtual void visitCharacter(Character* character);
    virtual void visitUnaryExp(UnaryExp* unaryExp);
    virtual void visitUnaryOp(UnaryOp* unaryOp);
    virtual void visitFuncRParams(FuncRParams* funcRParams);
    virtual void visitMulExp(MulExp* mulExp);
    virtual void visitAddExp(AddExp* addExp);
    virtual void visitRelExp(RelExp* relExp);
    virtual void visitEqExp(EqExp* eqExp);
    virtual void visitLAndExp(LAndExp* landExp);
    virtual void visitLOrExp(LOrExp* lorExp);
    virtual void visitConstExp(ConstExp* constExp);
};

} // thm

#endif //ASTVISITOR_H
