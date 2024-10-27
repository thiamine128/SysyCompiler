//
// Created by slty5 on 24-10-11.
//

#ifndef SYMBOLTABLEBUILDER_H
#define SYMBOLTABLEBUILDER_H
#include <stack>

#include "SymbolTable.h"
#include "../parser/ASTVisitor.h"

namespace thm {
    class Scope;
    class ErrorReporter;

class SemanticVisitor : public ASTVisitor{
public:
    int scopeNum = 0;
    int symbolNum = 0;
    int loops = 0;
    std::shared_ptr<Scope> currentScope;
    std::stack<std::shared_ptr<Scope>> scopeStack;
    std::vector<std::shared_ptr<Scope>> scopes;
    ErrorReporter& errorReporter_;

    SemanticVisitor(ErrorReporter& errorReporter);

    void pushScope(bool isReturnScope, bool requireReturnValue);
    void popScope();
    void submitSymbol(std::shared_ptr<Symbol> symbol);
    bool tryAccessSymbol(Token const& ident) const;
    std::shared_ptr<VariableSymbol> getArray(std::shared_ptr<Exp> exp) const;
    bool endWithReturn(std::shared_ptr<Block> block) const;

    void visitConstDecl(std::shared_ptr<ConstDecl> constDecl) override;
    void visitVarDecl(std::shared_ptr<VarDecl> varDecl) override;
    void visitFuncDef(std::shared_ptr<FuncDef> funcDef) override;
    void visitMainFuncDef(std::shared_ptr<MainFuncDef> mainFuncDef) override;
    void visitStmt(std::shared_ptr<Stmt> stmt) override;
    void visitForStmt(std::shared_ptr<ForStmt> forStmt) override;
    void visitCompUnit(std::shared_ptr<CompUnit> compUnit) override;
    void visitBlock(std::shared_ptr<Block> block) override;
    void visitLVal(std::shared_ptr<LVal> lval) override;
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
    void visitLAndExp(std::shared_ptr<LAndExp> landExp) override;
    void visitLOrExp(std::shared_ptr<LOrExp> lorExp) override;
    void visitConstExp(std::shared_ptr<ConstExp> constExp) override;
};

} // thm

#endif //SYMBOLTABLEBUILDER_H
