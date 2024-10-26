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

class SymbolTableBuilder : public ASTVisitor{
public:
    int scopeNum = 0;
    int symbolNum = 0;
    int loops = 0;
    std::shared_ptr<Scope> currentScope;
    std::stack<std::shared_ptr<Scope>> scopeStack;
    std::vector<std::shared_ptr<Scope>> scopes;
    ErrorReporter& errorReporter_;

    SymbolTableBuilder(ErrorReporter& errorReporter);

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
};

} // thm

#endif //SYMBOLTABLEBUILDER_H
