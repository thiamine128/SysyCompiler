//
// Created by slty5 on 24-9-27.
//

#include "AbstractSyntaxTree.h"

#include <ostream>

#include "ASTVisitor.h"
#include "../util/overloaded.h"

namespace thm {
    void ASTNode::consume(std::vector<Token> &tokens) {
        this->tokens = tokens;
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

    void CompUnit::visitChildren(std::shared_ptr<ASTVisitor> visitor) {
        for (auto& decl : this->decls) {
            visitor->visitDecl(decl);
        }
        for (auto& funcDef: this->funcDefs) {
            visitor->visitFuncDef(funcDef);
        }
        visitor->visitMainFuncDef(this->mainFuncDef);
    }

    void Decl::visitChildren(std::shared_ptr<ASTVisitor> visitor) {
        std::visit(overloaded{
            [&](std::unique_ptr<ConstDecl>& constDecl) {
                visitor->visitConstDecl(constDecl);
            },
            [&](std::unique_ptr<VarDecl>& varDecl) {
                visitor->visitVarDecl(varDecl);
            }}, decl);
        ASTNode::visitChildren(visitor);
    }

    void ConstDecl::visitChildren(std::shared_ptr<ASTVisitor> visitor) {
        visitor->visitBType(bType);
        for (auto& def : constDefs) {
            visitor->visitConstDef(def);
        }
    }

    void BType::visitChildren(std::shared_ptr<ASTVisitor> visitor) {

    }

    void ConstDef::visitChildren(std::shared_ptr<ASTVisitor> visitor) {
        std::visit(overloaded{
            [&](ConstDef::ConstDefBasic& basic) {

            },
            [&](ConstDef::ConstDefArray& array) {
                if (array.size != nullptr)
                    visitor->visitConstExp(array.size);
            }
        }, def);
        visitor->visitConstInitVal(val);
    }

    void ConstInitVal::visitChildren(std::shared_ptr<ASTVisitor> visitor) {
        std::visit(overloaded{
            [&](ConstInitVal::ConstInitValBasic& basic) {
                visitor->visitConstExp(basic.exp);
            },
            [&](ConstInitVal::ConstInitValArray& array) {
                for (auto& exp : array.exps) {
                    visitor->visitConstExp(exp);
                }
            },
            [&](std::string& str) {

            }
        }, val);
    }

    void VarDecl::visitChildren(std::shared_ptr<ASTVisitor> visitor) {
        visitor->visitBType(bType);
        for (auto& def : varDefs) {
            visitor->visitVarDef(def);
        }
    }

    void VarDef::visitChildren(std::shared_ptr<ASTVisitor> visitor) {
        std::visit(overloaded{
            [&](VarDef::VarDefBasic& basic) {

            },
            [&](VarDef::VarDefArray& array) {
                if (array.size != nullptr)
                    visitor->visitConstExp(array.size);
            }
        }, def);
        if (val != nullptr)
            visitor->visitInitVal(val);
    }

    void InitVal::visitChildren(std::shared_ptr<ASTVisitor> visitor) {
        std::visit(overloaded{
            [&](InitVal::InitValBasic& basic) {
                visitor->visitExp(basic.exp);
            },
            [&](InitVal::InitValArray& array) {
                for (auto& exp : array.exps) {
                    visitor->visitExp(exp);
                }
            },
            [&](std::string& str) {

            }
        }, val);
    }

    void FuncDef::visitChildren(std::shared_ptr<ASTVisitor> visitor) {
        visitor->visitFuncType(funcType);
        if (params != nullptr)
            visitor->visitFuncFParams(params);
        visitor->visitBlock(block);
    }

    void MainFuncDef::visitChildren(std::shared_ptr<ASTVisitor> visitor) {
        visitor->visitBlock(block);
    }

    void FuncType::visitChildren(std::shared_ptr<ASTVisitor> visitor) {

    }

    void FuncFParams::visitChildren(std::shared_ptr<ASTVisitor> visitor) {
        for (auto& param : params) {
            visitor->visitFuncFParam(param);
        }
    }

    void FuncFParam::visitChildren(std::shared_ptr<ASTVisitor> visitor) {
        visitor->visitBType(bType);
    }

    void Block::visitChildren(std::shared_ptr<ASTVisitor> visitor) {
        for (auto& item : items) {
            visitor->visitBlockItem(item);
        }
    }

    void BlockItem::visitChildren(std::shared_ptr<ASTVisitor> visitor) {
        std::visit(overloaded{
            [&](std::unique_ptr<Decl>& decl) {
                visitor->visitDecl(decl);
            },
            [&](std::unique_ptr<Stmt>& stmt) {
                visitor->visitStmt(stmt);
            }
        }, item);
    }

    void Stmt::visitChildren(std::shared_ptr<ASTVisitor> visitor) {
        std::visit(overloaded{
            [&](Stmt::StmtAssign& assign) {
                visitor->visitLVal(assign.lVal);
                visitor->visitExp(assign.exp);
            },
            [&](Stmt::StmtIf& stmtIf) {
                visitor->visitCond(stmtIf.cond);
                visitor->visitStmt(stmtIf.stmt);
                if (stmtIf.elseStmt != nullptr)
                    visitor->visitStmt(stmtIf.elseStmt);
            },
            [&](Stmt::StmtFor& stmtFor) {
                if (stmtFor.initStmt != nullptr)
                    visitor->visitForStmt(stmtFor.initStmt);
                if (stmtFor.cond != nullptr)
                    visitor->visitCond(stmtFor.cond);
                if (stmtFor.updateStmt != nullptr)
                    visitor->visitForStmt(stmtFor.updateStmt);
                visitor->visitStmt(stmtFor.stmt);
            },
            [&](Stmt::BreakOrContinue& breakOrContinue) {

            },
            [&](Stmt::StmtReturn& stmtReturn) {
                if (stmtReturn.exp != nullptr)
                    visitor->visitExp(stmtReturn.exp);
            },
            [&](Stmt::StmtRead& stmtRead) {
                if (stmtRead.lVal != nullptr)
                    visitor->visitLVal(stmtRead.lVal);
            },
            [&](Stmt::StmtPrintf& stmtPrintf) {
                for (auto& exp : stmtPrintf.exps) {
                    visitor->visitExp(exp);
                }
            },
            [&](std::unique_ptr<Exp>& exp) {
                if (exp != nullptr)
                    visitor->visitExp(exp);
            },
            [&](std::unique_ptr<Block>& block) {
                visitor->visitBlock(block);
            }
        }, stmt);
    }

    void ForStmt::visitChildren(std::shared_ptr<ASTVisitor> visitor) {
        visitor->visitLVal(lVal);
        visitor->visitExp(exp);
    }

    void Exp::visitChildren(std::shared_ptr<ASTVisitor> visitor) {
        visitor->visitAddExp(addExp);
    }

    void Cond::visitChildren(std::shared_ptr<ASTVisitor> visitor) {
        visitor->visitLOrExp(lOrExp);
    }

    void LVal::visitChildren(std::shared_ptr<ASTVisitor> visitor) {
        if (exp != nullptr)
            visitor->visitExp(exp);
    }

    void PrimaryExp::visitChildren(std::shared_ptr<ASTVisitor> visitor) {
        std::visit(overloaded{
            [&](std::unique_ptr<Exp>& exp) {
                visitor->visitExp(exp);
            },
            [&](std::unique_ptr<LVal>& lVal) {
                visitor->visitLVal(lVal);
            },
            [&](std::unique_ptr<Number>& number) {
                visitor->visitNumber(number);
            },
            [&](std::unique_ptr<Character>& character) {
                visitor->visitCharacter(character);
            }
        }, primaryExp);
    }

    void Number::visitChildren(std::shared_ptr<ASTVisitor> visitor) {

    }

    void Character::visitChildren(std::shared_ptr<ASTVisitor> visitor) {

    }

    void UnaryExp::visitChildren(std::shared_ptr<ASTVisitor> visitor) {
        std::visit(overloaded{
            [&](std::unique_ptr<PrimaryExp>& exp) {
                visitor->visitPrimaryExp(exp);
            },
            [&](FuncExp& funcExp) {
                if (funcExp.params != nullptr)
                    visitor->visitFuncRParams(funcExp.params);
            },
            [&](OpExp& opExp) {
                visitor->visitUnaryOp(opExp.op);
                visitor->visitUnaryExp(opExp.exp);
            }
        }, exp);
    }

    void UnaryOp::visitChildren(std::shared_ptr<ASTVisitor> visitor) {

    }

    void FuncRParams::visitChildren(std::shared_ptr<ASTVisitor> visitor) {
        for (auto& param : params) {
            visitor->visitExp(param);
        }
    }

    void MulExp::visitChildren(std::shared_ptr<ASTVisitor> visitor) {
        std::visit(overloaded{
            [&](std::unique_ptr<UnaryExp>& exp) {
                visitor->visitUnaryExp(exp);
            },
            [&](OpExp& opExp) {
                visitor->visitMulExp(opExp.mulExp);
                visitor->visitUnaryExp(opExp.unaryExp);
            }
        }, exp);
    }

    void AddExp::visitChildren(std::shared_ptr<ASTVisitor> visitor) {
        std::visit(overloaded{
            [&](std::unique_ptr<MulExp>& exp) {
                visitor->visitMulExp(exp);
            },
            [&](OpExp& opExp) {
                visitor->visitAddExp(opExp.addExp);
                visitor->visitMulExp(opExp.mulExp);
            }
        }, exp);
    }

    void RelExp::visitChildren(std::shared_ptr<ASTVisitor> visitor) {
        std::visit(overloaded{
            [&](std::unique_ptr<AddExp>& exp) {
                visitor->visitAddExp(exp);
            },
            [&](OpExp& opExp) {
                visitor->visitRelExp(opExp.relExp);
                visitor->visitAddExp(opExp.addExp);
            }
        }, exp);
    }

    void EqExp::visitChildren(std::shared_ptr<ASTVisitor> visitor) {
        std::visit(overloaded{
            [&](std::unique_ptr<RelExp>& exp) {
                visitor->visitRelExp(exp);
            },
            [&](OpExp& opExp) {
                visitor->visitEqExp(opExp.eqExp);
                visitor->visitRelExp(opExp.relExp);
            }
        }, exp);
    }

    void LAndExp::visitChildren(std::shared_ptr<ASTVisitor> visitor) {
        std::visit(overloaded{
            [&](std::unique_ptr<EqExp>& exp) {
                visitor->visitEqExp(exp);
            },
            [&](OpExp& opExp) {
                visitor->visitLAndExp(opExp.lAndExp);
                visitor->visitEqExp(opExp.eqExp);
            }
        }, exp);
    }

    void LOrExp::visitChildren(std::shared_ptr<ASTVisitor> visitor) {
        std::visit(overloaded{
            [&](std::unique_ptr<LAndExp>& exp) {
                visitor->visitLAndExp(exp);
            },
            [&](OpExp& opExp) {
                visitor->visitLOrExp(opExp.lOrExp);
                visitor->visitLAndExp(opExp.lAndExp);
            }
        }, exp);
    }

    void ConstExp::visitChildren(std::shared_ptr<ASTVisitor> visitor) {
        visitor->visitAddExp(addExp);
    }
} // thm