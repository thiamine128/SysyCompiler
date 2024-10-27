//
// Created by slty5 on 24-9-27.
//

#include "AbstractSyntaxTree.h"

#include <ostream>

#include "ASTVisitor.h"
#include "../semantic/SymbolTable.h"
#include "../util/util.h"

namespace thm {
    void ASTNode::consume(std::vector<Token> &tokens) {
        tokens.clear();
    }

    std::ostream & operator<<(std::ostream &os, const ASTNode &node) {
        if (node.nodeType() != ASTNode::BLOCKITEM && node.nodeType() != ASTNode::DECL && node.nodeType() != ASTNode::BTYPE)
            os << "<" << nodeTypeToString(node.nodeType()) << ">" << std::endl;
        return os;
    }

    std::string nodeTypeToString(ASTNode::ASTNodeType type) {
        switch (type) {
#define X(a, b, c) \
        case ASTNode::c: \
        return b;
            ASTNODES
            default:
                return "UNKNOWN";
        }
#undef X
    }

    void CompUnit::visit(std::shared_ptr<ASTVisitor> visitor) {
        visitor->visitCompUnit(std::static_pointer_cast<CompUnit>(shared_from_this()));
    }

    void Decl::visit(std::shared_ptr<ASTVisitor> visitor) {
        visitor->visitDecl(std::static_pointer_cast<Decl>(shared_from_this()));
    }

    void ConstDecl::visit(std::shared_ptr<ASTVisitor> visitor) {
        visitor->visitConstDecl(std::static_pointer_cast<ConstDecl>(shared_from_this()));
    }

    void BType::visit(std::shared_ptr<ASTVisitor> visitor) {
        visitor->visitBType(std::static_pointer_cast<BType>(shared_from_this()));
    }

    void ConstDef::visit(std::shared_ptr<ASTVisitor> visitor) {
        visitor->visitConstDef(std::static_pointer_cast<ConstDef>(shared_from_this()));
    }

    void ConstInitVal::visit(std::shared_ptr<ASTVisitor> visitor) {
        visitor->visitConstInitVal(std::static_pointer_cast<ConstInitVal>(shared_from_this()));
    }

    void VarDecl::visit(std::shared_ptr<ASTVisitor> visitor) {
        visitor->visitVarDecl(std::static_pointer_cast<VarDecl>(shared_from_this()));
    }

    void VarDef::visit(std::shared_ptr<ASTVisitor> visitor) {
        visitor->visitVarDef(std::static_pointer_cast<VarDef>(shared_from_this()));
    }

    void InitVal::visit(std::shared_ptr<ASTVisitor> visitor) {
        visitor->visitInitVal(std::static_pointer_cast<InitVal>(shared_from_this()));
    }

    void FuncDef::visit(std::shared_ptr<ASTVisitor> visitor) {
        visitor->visitFuncDef(std::static_pointer_cast<FuncDef>(shared_from_this()));
    }

    void MainFuncDef::visit(std::shared_ptr<ASTVisitor> visitor) {
        visitor->visitMainFuncDef(std::static_pointer_cast<MainFuncDef>(shared_from_this()));
    }

    void FuncType::visit(std::shared_ptr<ASTVisitor> visitor) {
        visitor->visitFuncType(std::static_pointer_cast<FuncType>(shared_from_this()));
    }

    void FuncFParams::visit(std::shared_ptr<ASTVisitor> visitor) {
        visitor->visitFuncFParams(std::static_pointer_cast<FuncFParams>(shared_from_this()));
    }

    void FuncFParam::visit(std::shared_ptr<ASTVisitor> visitor) {
        visitor->visitFuncFParam(std::static_pointer_cast<FuncFParam>(shared_from_this()));
    }

    void Block::visit(std::shared_ptr<ASTVisitor> visitor) {
        visitor->visitBlock(std::static_pointer_cast<Block>(shared_from_this()));
    }

    void BlockItem::visit(std::shared_ptr<ASTVisitor> visitor) {
        visitor->visitBlockItem(std::static_pointer_cast<BlockItem>(shared_from_this()));
    }

    void Stmt::visit(std::shared_ptr<ASTVisitor> visitor) {
        visitor->visitStmt(std::static_pointer_cast<Stmt>(shared_from_this()));
    }

    void ForStmt::visit(std::shared_ptr<ASTVisitor> visitor) {
        visitor->visitForStmt(std::static_pointer_cast<ForStmt>(shared_from_this()));
    }

    void Exp::visit(std::shared_ptr<ASTVisitor> visitor) {
        visitor->visitExp(std::static_pointer_cast<Exp>(shared_from_this()));
    }

    void Exp::evalConst(std::shared_ptr<SymbolTable> symbolTable) {

    }

    void Cond::visit(std::shared_ptr<ASTVisitor> visitor) {
        visitor->visitCond(std::static_pointer_cast<Cond>(shared_from_this()));
    }

    void LVal::visit(std::shared_ptr<ASTVisitor> visitor) {
        visitor->visitLVal(std::static_pointer_cast<LVal>(shared_from_this()));
    }

    void LVal::evalConst(std::shared_ptr<SymbolTable> symbolTable) {
        auto symbol = symbolTable->findSymbol(ident.content);
        if (symbol != nullptr && symbol->symbolType() == Symbol::VARIABLE) {
            std::shared_ptr<VariableSymbol> variableSymbol = std::static_pointer_cast<VariableSymbol>(symbol);
            if (variableSymbol->type.isConst) {
                if (!variableSymbol->type.isArray) {
                    isConst = true;
                    constVal = variableSymbol->constVal;
                } else if (exp != nullptr && exp->isConst) {
                    isConst &= exp->isConst;
                }
            }
        }
    }

    void PrimaryExp::visit(std::shared_ptr<ASTVisitor> visitor) {
        visitor->visitPrimaryExp(std::static_pointer_cast<PrimaryExp>(shared_from_this()));
    }

    void PrimaryExp::evalConst() {
        std::visit(overloaded{
            [&](std::shared_ptr<Exp>& exp) {
                //constVal = exp->evalConst();
            },
            [&](std::shared_ptr<LVal>& lVal) {
                //lVal->evalConst();
                //constVal = lVal->constVal;
            },
            [&](std::shared_ptr<Number>& number) {
                constVal = number->num;
            },
            [&](std::shared_ptr<Character>& character) {
                constVal = character->ch;
            }
        }, primaryExp);
    }

    void Number::visit(std::shared_ptr<ASTVisitor> visitor) {
        visitor->visitNumber(std::static_pointer_cast<Number>(shared_from_this()));
    }

    void Character::visit(std::shared_ptr<ASTVisitor> visitor) {
        visitor->visitCharacter(std::static_pointer_cast<Character>(shared_from_this()));
    }

    void UnaryExp::visit(std::shared_ptr<ASTVisitor> visitor) {
        visitor->visitUnaryExp(std::static_pointer_cast<UnaryExp>(shared_from_this()));
    }

    void UnaryExp::evalConst() {
    }

    void UnaryOp::visit(std::shared_ptr<ASTVisitor> visitor) {
        visitor->visitUnaryOp(std::static_pointer_cast<UnaryOp>(shared_from_this()));
    }

    void FuncRParams::visit(std::shared_ptr<ASTVisitor> visitor) {
        visitor->visitFuncRParams(std::static_pointer_cast<FuncRParams>(shared_from_this()));
    }

    void MulExp::visit(std::shared_ptr<ASTVisitor> visitor) {
        visitor->visitMulExp(std::static_pointer_cast<MulExp>(shared_from_this()));
    }

    void MulExp::evalConst() {

    }

    void AddExp::visit(std::shared_ptr<ASTVisitor> visitor) {
        visitor->visitAddExp(std::static_pointer_cast<AddExp>(shared_from_this()));
    }

    void AddExp::evalConst() {

    }

    void RelExp::visit(std::shared_ptr<ASTVisitor> visitor) {
        visitor->visitRelExp(std::static_pointer_cast<RelExp>(shared_from_this()));
    }

    void EqExp::visit(std::shared_ptr<ASTVisitor> visitor) {
        visitor->visitEqExp(std::static_pointer_cast<EqExp>(shared_from_this()));
    }

    void LAndExp::visit(std::shared_ptr<ASTVisitor> visitor) {
        visitor->visitLAndExp(std::static_pointer_cast<LAndExp>(shared_from_this()));
    }

    void LOrExp::visit(std::shared_ptr<ASTVisitor> visitor) {
        visitor->visitLOrExp(std::static_pointer_cast<LOrExp>(shared_from_this()));
    }

    void ConstExp::visit(std::shared_ptr<ASTVisitor> visitor) {
        visitor->visitConstExp(std::static_pointer_cast<ConstExp>(shared_from_this()));
    }

    void ConstExp::evalConst(std::shared_ptr<SymbolTable> symbolTable) {

    }
} // thm