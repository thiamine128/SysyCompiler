//
// Created by slty5 on 24-10-11.
//

#ifndef ASTVISITOR_H
#define ASTVISITOR_H
#include <memory>

#include "AbstractSyntaxTree.h"

namespace thm {

class ASTVisitor : public std::enable_shared_from_this<ASTVisitor> {
public:
    virtual ~ASTVisitor() = default;

    virtual void visitCompUnit(std::shared_ptr<CompUnit> compUnit);
    virtual void visitDecl(std::shared_ptr<Decl> decl);
    virtual void visitConstDecl(std::shared_ptr<ConstDecl> constDecl);
    virtual void visitBType(std::shared_ptr<BType> bType);
    virtual void visitConstDef(std::shared_ptr<ConstDef> constDef);
    virtual void visitConstInitVal(std::shared_ptr<ConstInitVal> constInitVal);
    virtual void visitVarDecl(std::shared_ptr<VarDecl> varDecl);
    virtual void visitVarDef(std::shared_ptr<VarDef> varDef);
    virtual void visitInitVal(std::shared_ptr<InitVal> initVal);
    virtual void visitFuncDef(std::shared_ptr<FuncDef> funcDef);
    virtual void visitMainFuncDef(std::shared_ptr<MainFuncDef> mainFuncDef);
    virtual void visitFuncType(std::shared_ptr<FuncType> funcType);
    virtual void visitFuncFParams(std::shared_ptr<FuncFParams> funcFParams);
    virtual void visitFuncFParam(std::shared_ptr<FuncFParam> funcFParam);
    virtual void visitBlock(std::shared_ptr<Block> block);
    virtual void visitBlockItem(std::shared_ptr<BlockItem> blockItem);
    virtual void visitStmt(std::shared_ptr<Stmt> stmt);
    virtual void visitForStmt(std::shared_ptr<ForStmt> forStmt);
    virtual void visitExp(std::shared_ptr<Exp> exp);
    virtual void visitCond(std::shared_ptr<Cond> cond);
    virtual void visitLVal(std::shared_ptr<LVal> lVal);
    virtual void visitPrimaryExp(std::shared_ptr<PrimaryExp> primaryExp);
    virtual void visitNumber(std::shared_ptr<Number> number);
    virtual void visitCharacter(std::shared_ptr<Character> character);
    virtual void visitUnaryExp(std::shared_ptr<UnaryExp> unaryExp);
    virtual void visitUnaryOp(std::shared_ptr<UnaryOp> unaryOp);
    virtual void visitFuncRParams(std::shared_ptr<FuncRParams> funcRParams);
    virtual void visitMulExp(std::shared_ptr<MulExp> mulExp);
    virtual void visitAddExp(std::shared_ptr<AddExp> addExp);
    virtual void visitRelExp(std::shared_ptr<RelExp> relExp);
    virtual void visitEqExp(std::shared_ptr<EqExp> eqExp);
    virtual void visitLAndExp(std::shared_ptr<LAndExp> landExp);
    virtual void visitLOrExp(std::shared_ptr<LOrExp> lorExp);
    virtual void visitConstExp(std::shared_ptr<ConstExp> constExp);
};

} // thm

#endif //ASTVISITOR_H
