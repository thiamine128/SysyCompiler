//
// Created by slty5 on 24-10-11.
//

#ifndef SYMBOLTABLEBUILDER_H
#define SYMBOLTABLEBUILDER_H
#include <stack>
#include "../parser/ASTVisitor.h"

namespace thm {
    class Scope;
    class ErrorReporter;

class SemanticVisitor : public ASTVisitor{
public:
    int scopeNum = 0;
    int symbolNum = 0;
    int loops = 0;
    Scope* currentScope = nullptr;
    std::stack<Scope*> scopeStack;
    std::vector<Scope*> scopes;
    ErrorReporter& errorReporter_;

    SemanticVisitor(ErrorReporter& errorReporter);

    void pushScope(bool isReturnScope, bool requireReturnValue);
    void popScope();
    void submitSymbol(Symbol* symbol) const;
    bool tryAccessSymbol(Token const& ident) const;
    VariableSymbol* getArray(const Exp* exp) const;

    static bool endWithReturn(const Block* block);

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
    void visitLAndExp(LAndExp* landExp) override;
    void visitLOrExp(LOrExp* lorExp) override;
    void visitConstExp(ConstExp* constExp) override;
};

} // thm

#endif //SYMBOLTABLEBUILDER_H
