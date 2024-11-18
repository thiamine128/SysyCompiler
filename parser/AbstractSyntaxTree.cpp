//
// Created by slty5 on 24-9-27.
//

#include "AbstractSyntaxTree.h"

#include <ostream>

#include "ASTVisitor.h"
#include "../llvm/LLVM.h"
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

    void CompUnit::visit(ASTVisitor* visitor) {
        visitor->visitCompUnit(this);
    }

    void Decl::visit(ASTVisitor* visitor) {
        visitor->visitDecl(this);
    }

    void ConstDecl::visit(ASTVisitor* visitor) {
        visitor->visitConstDecl(this);
    }

    void BType::visit(ASTVisitor* visitor) {
        visitor->visitBType(this);
    }

    void ConstDef::visit(ASTVisitor* visitor) {
        visitor->visitConstDef(this);
    }

    void ConstInitVal::visit(ASTVisitor* visitor) {
        visitor->visitConstInitVal(this);
    }

    void VarDecl::visit(ASTVisitor* visitor) {
        visitor->visitVarDecl(this);
    }

    void VarDef::visit(ASTVisitor* visitor) {
        visitor->visitVarDef(this);
    }

    void InitVal::visit(ASTVisitor* visitor) {
        visitor->visitInitVal(this);
    }

    void FuncDef::visit(ASTVisitor* visitor) {
        visitor->visitFuncDef(this);
    }

    void MainFuncDef::visit(ASTVisitor* visitor) {
        visitor->visitMainFuncDef(this);
    }

    void FuncType::visit(ASTVisitor* visitor) {
        visitor->visitFuncType(this);
    }

    void FuncFParams::visit(ASTVisitor* visitor) {
        visitor->visitFuncFParams(this);
    }

    void FuncFParam::visit(ASTVisitor* visitor) {
        visitor->visitFuncFParam(this);
    }

    void Block::visit(ASTVisitor* visitor) {
        visitor->visitBlock(this);
    }

    void BlockItem::visit(ASTVisitor* visitor) {
        visitor->visitBlockItem(this);
    }

    void Stmt::visit(ASTVisitor* visitor) {
        visitor->visitStmt(this);
    }

    void ForStmt::visit(ASTVisitor* visitor) {
        visitor->visitForStmt(this);
    }

    void Exp::visit(ASTVisitor* visitor) {
        visitor->visitExp(this);
    }

    void Exp::evalConst() {
        isConst = addExp->isConst;
        constVal = addExp->constVal;
    }

    BasicValueType * Exp::getBasicType() const {
        return static_cast<BasicValueType *>(value->valueType);
    }

    void Cond::visit(ASTVisitor* visitor) {
        visitor->visitCond(this);
    }

    void LVal::visit(ASTVisitor* visitor) {
        visitor->visitLVal(this);
    }

    void LVal::evalConst() {
        isConst = symbol->type.isConst;
        if (exp != nullptr) {
            isConst &= exp->isConst;
        }
        if (isConst) {
            if (symbol->type.isArray) {
                constVal = symbol->initVals[exp->constVal];
            } else {
                constVal = symbol->initVal;
            }
        }
    }

    BasicValueType * LVal::getBasicType() const {
        PtrValueType *ptrType = static_cast<PtrValueType *>(value->valueType);
        return static_cast<BasicValueType *>(ptrType->value);
    }

    void PrimaryExp::visit(ASTVisitor* visitor) {
        visitor->visitPrimaryExp(this);
    }

    void PrimaryExp::evalConst() {
        std::visit(overloaded{
            [&](const Exp* exp) {
                isConst = exp->isConst;
                constVal = exp->constVal;
            },
            [&](const LVal* lVal) {
                isConst = lVal->isConst;
                constVal = lVal->constVal;
            },
            [&](const Number* number) {
                isConst = true;
                constVal = number->num;
            },
            [&](const Character* character) {
                isConst = true;
                constVal = character->ch;
            }
        }, primaryExp);
    }

    void Number::visit(ASTVisitor* visitor) {
        visitor->visitNumber(this);
    }

    void Character::visit(ASTVisitor* visitor) {
        visitor->visitCharacter(this);
    }

    void UnaryExp::visit(ASTVisitor* visitor) {
        visitor->visitUnaryExp(this);
    }

    void UnaryExp::evalConst() {
        std::visit(overloaded{
            [&](const PrimaryExp* exp) {
                isConst = exp->isConst;
                constVal = exp->constVal;
            },
            [&](UnaryExp::FuncExp& funcExp) {
                // TODO: Const Function ?
                isConst = false;
            },
            [&](const UnaryExp::OpExp& opExp) {
                isConst = opExp.exp->isConst;
                switch (opExp.op->type) {
                    case UnaryOp::NOT:
                        constVal = !opExp.exp->constVal;
                    break;
                    case UnaryOp::MINUS:
                        constVal = -opExp.exp->constVal;
                    break;
                    case UnaryOp::PLUS:
                        constVal = opExp.exp->constVal;
                    break;
                }
            }
        }, exp);
    }

    BasicValueType * UnaryExp::getBasicType() const {
        return static_cast<BasicValueType *>(value->valueType);
    }

    void UnaryOp::visit(ASTVisitor* visitor) {
        visitor->visitUnaryOp(this);
    }

    void FuncRParams::visit(ASTVisitor* visitor) {
        visitor->visitFuncRParams(this);
    }

    void MulExp::visit(ASTVisitor* visitor) {
        visitor->visitMulExp(this);
    }

    void MulExp::evalConst() {
        std::visit(overloaded{
            [&](const UnaryExp* unaryExp) {
                isConst = unaryExp->isConst;
                constVal = unaryExp->constVal;
            },
            [&](const MulExp::OpExp& opExp) {
                isConst = opExp.unaryExp->isConst && opExp.mulExp->isConst;
                if (isConst) {
                    switch (opExp.op) {
                        case OpExp::MUL:
                            constVal = opExp.mulExp->constVal * opExp.unaryExp->constVal;
                        break;
                        case OpExp::DIV:
                            constVal = opExp.mulExp->constVal / opExp.unaryExp->constVal;
                        break;
                        case OpExp::MOD:
                            constVal = opExp.mulExp->constVal % opExp.unaryExp->constVal;
                        break;
                    }
                }
            }
        }, exp);
    }

    void AddExp::visit(ASTVisitor* visitor) {
        visitor->visitAddExp(this);
    }

    void AddExp::evalConst() {
        std::visit(overloaded{
            [&](const MulExp* mulExp) {
                isConst = mulExp->isConst;
                constVal = mulExp->constVal;
            },
            [&](const AddExp::OpExp& opExp) {
                isConst = opExp.mulExp->isConst && opExp.addExp->isConst;
                switch (opExp.op) {
                    case OpExp::ADD:
                        constVal = opExp.addExp->constVal + opExp.mulExp->constVal;
                    break;
                    case OpExp::MINUS:
                        constVal = opExp.addExp->constVal - opExp.mulExp->constVal;
                    break;
                }
            }
        }, exp);
    }

    void RelExp::visit(ASTVisitor* visitor) {
        visitor->visitRelExp(this);
    }

    BasicValueType * RelExp::getBasicType() const {
        return static_cast<BasicValueType *>(value->valueType);
    }

    void EqExp::visit(ASTVisitor* visitor) {
        visitor->visitEqExp(this);
    }

    BasicValueType * EqExp::getBasicType() const {
        return static_cast<BasicValueType *>(value->valueType);
    }

    void LAndExp::visit(ASTVisitor* visitor) {
        visitor->visitLAndExp(this);
    }

    void LOrExp::visit(ASTVisitor* visitor) {
        visitor->visitLOrExp(this);
    }

    void ConstExp::visit(ASTVisitor* visitor) {
        visitor->visitConstExp(this);
    }

    void ConstExp::evalConst() {
        constVal = addExp->constVal;
    }
} // thm