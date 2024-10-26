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

    virtual void visitCompUnit(std::shared_ptr<CompUnit>& compUnit) { compUnit->visitChildren(shared_from_this()); };
    virtual void visitDecl(std::shared_ptr<Decl>& decl) { decl->visitChildren(shared_from_this()); };
    virtual void visitConstDecl(std::shared_ptr<ConstDecl>& constDecl) { constDecl->visitChildren(shared_from_this()); };
    virtual void visitBType(std::shared_ptr<BType>& btype) { btype->visitChildren(shared_from_this()); };
    virtual void visitConstDef(std::shared_ptr<ConstDef>& constDef) { constDef->visitChildren(shared_from_this()); };
    virtual void visitConstInitVal(std::shared_ptr<ConstInitVal>& constInitVal) { constInitVal->visitChildren(shared_from_this()); };
    virtual void visitVarDecl(std::shared_ptr<VarDecl>& varDecl) { varDecl->visitChildren(shared_from_this()); };
    virtual void visitVarDef(std::shared_ptr<VarDef>& varDef) { varDef->visitChildren(shared_from_this()); };
    virtual void visitInitVal(std::shared_ptr<InitVal>& initVal) { initVal->visitChildren(shared_from_this()); };
    virtual void visitFuncDef(std::shared_ptr<FuncDef>& funcDef) { funcDef->visitChildren(shared_from_this()); };
    virtual void visitMainFuncDef(std::shared_ptr<MainFuncDef>& mainFuncDef) { mainFuncDef->visitChildren(shared_from_this()); };
    virtual void visitFuncType(std::shared_ptr<FuncType>& funcType) { funcType->visitChildren(shared_from_this()); };
    virtual void visitFuncFParams(std::shared_ptr<FuncFParams>& funcFParams) { funcFParams->visitChildren(shared_from_this()); };
    virtual void visitFuncFParam(std::shared_ptr<FuncFParam>& funcFParam) { funcFParam->visitChildren(shared_from_this()); };
    virtual void visitBlock(std::shared_ptr<Block>& block) { block->visitChildren(shared_from_this()); };
    virtual void visitBlockItem(std::shared_ptr<BlockItem>& blockItem) {
        blockItem->visitChildren(shared_from_this());
    };
    virtual void visitStmt(std::shared_ptr<Stmt>& stmt) { stmt->visitChildren(shared_from_this()); };
    virtual void visitForStmt(std::shared_ptr<ForStmt>& forStmt) { forStmt->visitChildren(shared_from_this()); };
    virtual void visitExp(std::shared_ptr<Exp>& exp) { exp->visitChildren(shared_from_this()); };
    virtual void visitCond(std::shared_ptr<Cond>& cond) { cond->visitChildren(shared_from_this()); };
    virtual void visitLVal(std::shared_ptr<LVal>& lval) { lval->visitChildren(shared_from_this()); };
    virtual void visitPrimaryExp(std::shared_ptr<PrimaryExp>& primaryExp) { primaryExp->visitChildren(shared_from_this()); };
    virtual void visitNumber(std::shared_ptr<Number>& number) { number->visitChildren(shared_from_this()); };
    virtual void visitCharacter(std::shared_ptr<Character>& character) { character->visitChildren(shared_from_this()); };
    virtual void visitUnaryExp(std::shared_ptr<UnaryExp>& unaryExp) { unaryExp->visitChildren(shared_from_this()); };
    virtual void visitUnaryOp(std::shared_ptr<UnaryOp>& unaryOp) { unaryOp->visitChildren(shared_from_this()); };
    virtual void visitFuncRParams(std::shared_ptr<FuncRParams>& funcRParams) { funcRParams->visitChildren(shared_from_this()); };
    virtual void visitMulExp(std::shared_ptr<MulExp>& mulExp) { mulExp->visitChildren(shared_from_this()); };
    virtual void visitAddExp(std::shared_ptr<AddExp>& addExp) { addExp->visitChildren(shared_from_this()); };
    virtual void visitRelExp(std::shared_ptr<RelExp>& relExp) { relExp->visitChildren(shared_from_this()); };
    virtual void visitEqExp(std::shared_ptr<EqExp>& eqExp) { eqExp->visitChildren(shared_from_this()); };
    virtual void visitLAndExp(std::shared_ptr<LAndExp>& landExp) { landExp->visitChildren(shared_from_this()); };
    virtual void visitLOrExp(std::shared_ptr<LOrExp>& lorExp) { lorExp->visitChildren(shared_from_this()); };
    virtual void visitConstExp(std::shared_ptr<ConstExp>& constExp) { constExp->visitChildren(shared_from_this()); };
};

} // thm

#endif //ASTVISITOR_H
