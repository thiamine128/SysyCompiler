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

    virtual void visitCompUnit(std::unique_ptr<CompUnit>& compUnit) { compUnit->visitChildren(shared_from_this()); };
    virtual void visitDecl(std::unique_ptr<Decl>& decl) { decl->visitChildren(shared_from_this()); };
    virtual void visitConstDecl(std::unique_ptr<ConstDecl>& constDecl) { constDecl->visitChildren(shared_from_this()); };
    virtual void visitBType(std::unique_ptr<BType>& btype) { btype->visitChildren(shared_from_this()); };
    virtual void visitConstDef(std::unique_ptr<ConstDef>& constDef) { constDef->visitChildren(shared_from_this()); };
    virtual void visitConstInitVal(std::unique_ptr<ConstInitVal>& constInitVal) { constInitVal->visitChildren(shared_from_this()); };
    virtual void visitVarDecl(std::unique_ptr<VarDecl>& varDecl) { varDecl->visitChildren(shared_from_this()); };
    virtual void visitVarDef(std::unique_ptr<VarDef>& varDef) { varDef->visitChildren(shared_from_this()); };
    virtual void visitInitVal(std::unique_ptr<InitVal>& initVal) { initVal->visitChildren(shared_from_this()); };
    virtual void visitFuncDef(std::unique_ptr<FuncDef>& funcDef) { funcDef->visitChildren(shared_from_this()); };
    virtual void visitMainFuncDef(std::unique_ptr<MainFuncDef>& mainFuncDef) { mainFuncDef->visitChildren(shared_from_this()); };
    virtual void visitFuncType(std::unique_ptr<FuncType>& funcType) { funcType->visitChildren(shared_from_this()); };
    virtual void visitFuncFParams(std::unique_ptr<FuncFParams>& funcFParams) { funcFParams->visitChildren(shared_from_this()); };
    virtual void visitFuncFParam(std::unique_ptr<FuncFParam>& funcFParam) { funcFParam->visitChildren(shared_from_this()); };
    virtual void visitBlock(std::unique_ptr<Block>& block) { block->visitChildren(shared_from_this()); };
    virtual void visitBlockItem(std::unique_ptr<BlockItem>& blockItem) {
        blockItem->visitChildren(shared_from_this());
    };
    virtual void visitStmt(std::unique_ptr<Stmt>& stmt) { stmt->visitChildren(shared_from_this()); };
    virtual void visitForStmt(std::unique_ptr<ForStmt>& forStmt) { forStmt->visitChildren(shared_from_this()); };
    virtual void visitExp(std::unique_ptr<Exp>& exp) { exp->visitChildren(shared_from_this()); };
    virtual void visitCond(std::unique_ptr<Cond>& cond) { cond->visitChildren(shared_from_this()); };
    virtual void visitLVal(std::unique_ptr<LVal>& lval) { lval->visitChildren(shared_from_this()); };
    virtual void visitPrimaryExp(std::unique_ptr<PrimaryExp>& primaryExp) { primaryExp->visitChildren(shared_from_this()); };
    virtual void visitNumber(std::unique_ptr<Number>& number) { number->visitChildren(shared_from_this()); };
    virtual void visitCharacter(std::unique_ptr<Character>& character) { character->visitChildren(shared_from_this()); };
    virtual void visitUnaryExp(std::unique_ptr<UnaryExp>& unaryExp) { unaryExp->visitChildren(shared_from_this()); };
    virtual void visitUnaryOp(std::unique_ptr<UnaryOp>& unaryOp) { unaryOp->visitChildren(shared_from_this()); };
    virtual void visitFuncRParams(std::unique_ptr<FuncRParams>& funcRParams) { funcRParams->visitChildren(shared_from_this()); };
    virtual void visitMulExp(std::unique_ptr<MulExp>& mulExp) { mulExp->visitChildren(shared_from_this()); };
    virtual void visitAddExp(std::unique_ptr<AddExp>& addExp) { addExp->visitChildren(shared_from_this()); };
    virtual void visitRelExp(std::unique_ptr<RelExp>& relExp) { relExp->visitChildren(shared_from_this()); };
    virtual void visitEqExp(std::unique_ptr<EqExp>& eqExp) { eqExp->visitChildren(shared_from_this()); };
    virtual void visitLAndExp(std::unique_ptr<LAndExp>& landExp) { landExp->visitChildren(shared_from_this()); };
    virtual void visitLOrExp(std::unique_ptr<LOrExp>& lorExp) { lorExp->visitChildren(shared_from_this()); };
    virtual void visitConstExp(std::unique_ptr<ConstExp>& constExp) { constExp->visitChildren(shared_from_this()); };
};

} // thm

#endif //ASTVISITOR_H
