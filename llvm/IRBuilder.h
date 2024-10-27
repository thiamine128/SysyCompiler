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
    std::shared_ptr<Module> module;

    IRBuilder();
    void visitConstDecl(std::shared_ptr<ConstDecl> constDecl) override;
    void visitVarDecl(std::shared_ptr<VarDecl> varDecl) override;
    void visitFuncDef(std::shared_ptr<FuncDef> funcDef) override;
    void visitMainFuncDef(std::shared_ptr<MainFuncDef> mainFuncDef) override;
    void visitStmt(std::shared_ptr<Stmt> stmt) override;
    void visitForStmt(std::shared_ptr<ForStmt> forStmt) override;
    void visitCompUnit(std::shared_ptr<CompUnit> compUnit) override;
    void visitBlock(std::shared_ptr<Block> block) override;
    void visitLVal(std::shared_ptr<LVal> lVal) override;
    void visitUnaryExp(std::shared_ptr<UnaryExp> unaryExp) override;
    void visitDecl(std::shared_ptr<Decl> decl) override;
    void visitBType(std::shared_ptr<BType> bType) override;
    void visitConstDef(std::shared_ptr<ConstDef> constDef) override;
    void visitConstInitVal(std::shared_ptr<ConstInitVal> constInitVal) override;
    void visitVarDef(std::shared_ptr<VarDef> varDef) override;
    void visitInitVal(std::shared_ptr<InitVal> initVal) override;
    void visitFuncType(std::shared_ptr<FuncType> funcType) override;
    void visitFuncFParams(std::shared_ptr<FuncFParams> funcFParams) override;
    void visitFuncFParam(std::shared_ptr<FuncFParam> funcFParam) override;
    void visitBlockItem(std::shared_ptr<BlockItem> blockItem) override;
    void visitExp(std::shared_ptr<Exp> exp) override;
    void visitCond(std::shared_ptr<Cond> cond) override;
    void visitPrimaryExp(std::shared_ptr<PrimaryExp> primaryExp) override;
    void visitNumber(std::shared_ptr<Number> number) override;
    void visitCharacter(std::shared_ptr<Character> character) override;
    void visitUnaryOp(std::shared_ptr<UnaryOp> unaryOp) override;
    void visitFuncRParams(std::shared_ptr<FuncRParams> funcRParams) override;
    void visitMulExp(std::shared_ptr<MulExp> mulExp) override;
    void visitAddExp(std::shared_ptr<AddExp> addExp) override;
    void visitRelExp(std::shared_ptr<RelExp> relExp) override;
    void visitEqExp(std::shared_ptr<EqExp> eqExp) override;
    void visitLAndExp(std::shared_ptr<LAndExp> lAndExp) override;
    void visitLOrExp(std::shared_ptr<LOrExp> lOrExp) override;
    void visitConstExp(std::shared_ptr<ConstExp> constExp) override;
};

} // thm

#endif //IRBUILDER_H
