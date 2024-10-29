//
// Created by slty5 on 24-10-11.
//

#include "ASTVisitor.h"

#include <bits/fs_fwd.h>

#include "../util/util.h"

namespace thm {
    void ASTVisitor::visitCompUnit(CompUnit* compUnit) {
        for (auto decl : compUnit->decls) {
            decl->visit(this);
        }
        for (auto funcDef : compUnit->funcDefs) {
            funcDef->visit(this);
        }
        compUnit->mainFuncDef->visit(this);
    }

    void ASTVisitor::visitDecl(Decl* decl) {
        std::visit(overloaded{
            [&](ConstDecl* constDecl) {
                constDecl->visit(this);
            },
            [&](VarDecl* varDecl) {
                varDecl->visit(this);
            },
        }, decl->decl);
    }

    void ASTVisitor::visitConstDecl(ConstDecl* constDecl) {
        constDecl->bType->visit(this);
        for (auto constDef : constDecl->constDefs) {
            constDef->visit(this);
        }
    }

    void ASTVisitor::visitBType(BType* bType) {

    }

    void ASTVisitor::visitConstDef(ConstDef* constDef) {
        std::visit(overloaded{
            [&](ConstDef::ConstDefBasic& basic) {},
            [&](const ConstDef::ConstDefArray& array) {
                if (array.size != nullptr) {
                    array.size->visit(this);
                }
            }
        }, constDef->def);
        constDef->val->visit(this);
    }

    void ASTVisitor::visitConstInitVal(ConstInitVal* constInitVal) {
        std::visit(overloaded{
            [&](const ConstInitVal::ConstInitValBasic& basic) {
                basic.exp->visit(this);
            },
            [&](ConstInitVal::ConstInitValArray& array) {
                for (auto exp : array.exps) {
                    exp->visit(this);
                }
            },
            [&](std::string& str) {

            }
        }, constInitVal->val);
    }

    void ASTVisitor::visitVarDecl(VarDecl* varDecl) {
        varDecl->bType->visit(this);
        for (auto def : varDecl->varDefs) {
            def->visit(this);
        }
    }

    void ASTVisitor::visitVarDef(VarDef* varDef) {
        std::visit(overloaded{
            [&](VarDef::VarDefBasic& basic) {

            },
            [&](const VarDef::VarDefArray& array) {
                if (array.size != nullptr) {
                    array.size->visit(this);
                }
            }
        }, varDef->def);
        if (varDef->val != nullptr) {
            varDef->val->visit(this);
        }
    }

    void ASTVisitor::visitInitVal(InitVal* initVal) {
        std::visit(overloaded{
            [&](const InitVal::InitValBasic& basic) {
                basic.exp->visit(this);
            },
            [&](InitVal::InitValArray& array) {
                for (auto exp : array.exps) {
                    exp->visit(this);
                }
            },
            [&](std::string& str) {

            }
        }, initVal->val);
    }

    void ASTVisitor::visitFuncDef(FuncDef* funcDef) {
        funcDef->funcType->visit(this);
        if (funcDef->params != nullptr) {
            funcDef->params->visit(this);
        }
        funcDef->block->visit(this);
    }

    void ASTVisitor::visitMainFuncDef(MainFuncDef* mainFuncDef) {
        mainFuncDef->block->visit(this);
    }

    void ASTVisitor::visitFuncType(FuncType* funcType) {

    }

    void ASTVisitor::visitFuncFParams(FuncFParams* funcFParams) {
        for (auto funcFParam : funcFParams->params) {
            funcFParam->visit(this);
        }
    }

    void ASTVisitor::visitFuncFParam(FuncFParam* funcFParam) {
        funcFParam->bType->visit(this);
    }

    void ASTVisitor::visitBlock(Block* block) {
        for (auto item : block->items) {
            item->visit(this);
        }
    }

    void ASTVisitor::visitBlockItem(BlockItem* blockItem) {
        std::visit(overloaded{
            [&](Decl* decl) {
                decl->visit(this);
            },
            [&](Stmt* stmt) {
                stmt->visit(this);
            }
        }, blockItem->item);
    }

    void ASTVisitor::visitStmt(Stmt* stmt) {
        std::visit(overloaded{
            [&](const Stmt::StmtAssign& stmtAssign) {
                stmtAssign.lVal->visit(this);
                stmtAssign.exp->visit(this);
            },
            [&](Exp* exp) {
                if (exp != nullptr)
                    exp->visit(this);
            },
            [&](Block* block) {
                block->visit(this);
            },
            [&](const Stmt::StmtIf& stmtIf) {
                stmtIf.cond->visit(this);
                stmtIf.stmt->visit(this);
                if (stmtIf.elseStmt != nullptr) {
                    stmtIf.elseStmt->visit(this);
                }
            },
            [&](const Stmt::StmtFor& stmtFor) {
                if (stmtFor.initStmt != nullptr) {
                    stmtFor.initStmt->visit(this);
                }
                if (stmtFor.cond != nullptr) {
                    stmtFor.cond->visit(this);
                }
                if (stmtFor.updateStmt != nullptr) {
                    stmtFor.updateStmt->visit(this);
                }
                stmtFor.stmt->visit(this);
            },
            [&](Stmt::BreakOrContinue& breakOrContinue) {

            },
            [&](const Stmt::StmtReturn& stmtReturn) {
                if (stmtReturn.exp != nullptr)
                    stmtReturn.exp->visit(this);
            },
            [&](const Stmt::StmtRead& stmtRead) {
                stmtRead.lVal->visit(this);
            },
            [&](Stmt::StmtPrintf& stmtPrintf) {
                for (auto exp : stmtPrintf.exps) {
                    exp->visit(this);
                }
            },
        }, stmt->stmt);
    }

    void ASTVisitor::visitForStmt(ForStmt* forStmt) {
        forStmt->lVal->visit(this);
        forStmt->exp->visit(this);
    }

    void ASTVisitor::visitExp(Exp* exp) {
        exp->addExp->visit(this);
    }

    void ASTVisitor::visitCond(Cond* cond) {
        cond->lOrExp->visit(this);
    }

    void ASTVisitor::visitLVal(LVal* lVal) {
        if (lVal->exp != nullptr) {
            lVal->exp->visit(this);
        }
    }

    void ASTVisitor::visitPrimaryExp(PrimaryExp* primaryExp) {
        std::visit(overloaded{
            [&](Exp* exp) {
                exp->visit(this);
            },
            [&](LVal* lVal) {
                lVal->visit(this);
            },
            [&](Number* number) {
                number->visit(this);
            },
            [&](Character* character) {
                character->visit(this);
            }
        }, primaryExp->primaryExp);
    }

    void ASTVisitor::visitNumber(Number* number) {

    }

    void ASTVisitor::visitCharacter(Character* character) {

    }

    void ASTVisitor::visitUnaryExp(UnaryExp* unaryExp) {
        std::visit(overloaded{
            [&](PrimaryExp* primaryExp) {
                primaryExp->visit(this);
            },
            [&](const UnaryExp::FuncExp& funcExp) {
                if (funcExp.params != nullptr)
                    funcExp.params->visit(this);
            },
            [&](const UnaryExp::OpExp& opExp) {
                opExp.op->visit(this);
                opExp.exp->visit(this);
            }
        }, unaryExp->exp);
    }

    void ASTVisitor::visitUnaryOp(UnaryOp* unaryOp) {

    }

    void ASTVisitor::visitFuncRParams(FuncRParams* funcRParams) {
        for (auto param : funcRParams->params) {
            param->visit(this);
        }
    }

    void ASTVisitor::visitMulExp(MulExp* mulExp) {
        std::visit(overloaded{
            [&](UnaryExp* unaryExp) {
                unaryExp->visit(this);
            },
            [&](const MulExp::OpExp& opExp) {
                opExp.mulExp->visit(this);
                opExp.unaryExp->visit(this);
            }
        }, mulExp->exp);
    }

    void ASTVisitor::visitAddExp(AddExp* addExp) {
        std::visit(overloaded{
            [&](MulExp* mulExp) {
                mulExp->visit(this);
            },
            [&](const AddExp::OpExp& opExp) {
                opExp.addExp->visit(this);
                opExp.mulExp->visit(this);
            }
        }, addExp->exp);
    }

    void ASTVisitor::visitRelExp(RelExp* relExp) {
        std::visit(overloaded{
            [&](AddExp* addExp) {
                addExp->visit(this);
            },
            [&](const RelExp::OpExp& opExp) {
                opExp.relExp->visit(this);
                opExp.addExp->visit(this);
            }
        }, relExp->exp);
    }

    void ASTVisitor::visitEqExp(EqExp* eqExp) {
        std::visit(overloaded{
            [&](RelExp* relExp) {
                relExp->visit(this);
            },
            [&](const EqExp::OpExp& opExp) {
                opExp.eqExp->visit(this);
                opExp.relExp->visit(this);
            }
        }, eqExp->exp);
    }

    void ASTVisitor::visitLAndExp(LAndExp* lAndExp) {
        std::visit(overloaded{
            [&](EqExp* eqExp) {
                eqExp->visit(this);
            },
            [&](const LAndExp::OpExp& opExp) {
                opExp.lAndExp->visit(this);
                opExp.eqExp->visit(this);
            }
        }, lAndExp->exp);
    }

    void ASTVisitor::visitLOrExp(LOrExp* lOrExp) {
        std::visit(overloaded{
            [&](LAndExp* lAndExp) {
                lAndExp->visit(this);
            },
            [&](const LOrExp::OpExp& opExp) {
                opExp.lOrExp->visit(this);
                opExp.lAndExp->visit(this);
            }
        }, lOrExp->exp);
    }

    void ASTVisitor::visitConstExp(ConstExp* constExp) {
        constExp->addExp->visit(this);
    }
} // thm