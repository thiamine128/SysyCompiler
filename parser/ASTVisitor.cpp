//
// Created by slty5 on 24-10-11.
//

#include "ASTVisitor.h"

#include <bits/fs_fwd.h>

#include "../util/util.h"

namespace thm {
    void ASTVisitor::visitCompUnit(std::shared_ptr<CompUnit> compUnit) {
        for (auto decl : compUnit->decls) {
            decl->visit(shared_from_this());
        }
        for (auto funcDef : compUnit->funcDefs) {
            funcDef->visit(shared_from_this());
        }
        compUnit->mainFuncDef->visit(shared_from_this());
    }

    void ASTVisitor::visitDecl(std::shared_ptr<Decl> decl) {
        std::visit(overloaded{
            [&](std::shared_ptr<ConstDecl> constDecl) {
                constDecl->visit(shared_from_this());
            },
            [&](std::shared_ptr<VarDecl> varDecl) {
                varDecl->visit(shared_from_this());
            },
        }, decl->decl);
    }

    void ASTVisitor::visitConstDecl(std::shared_ptr<ConstDecl> constDecl) {
        constDecl->bType->visit(shared_from_this());
        for (auto constDef : constDecl->constDefs) {
            constDef->visit(shared_from_this());
        }
    }

    void ASTVisitor::visitBType(std::shared_ptr<BType> bType) {

    }

    void ASTVisitor::visitConstDef(std::shared_ptr<ConstDef> constDef) {
        std::visit(overloaded{
            [&](ConstDef::ConstDefBasic& basic) {},
            [&](ConstDef::ConstDefArray& array) {
                if (array.size != nullptr) {
                    array.size->visit(shared_from_this());
                }
            }
        }, constDef->def);
        constDef->val->visit(shared_from_this());
    }

    void ASTVisitor::visitConstInitVal(std::shared_ptr<ConstInitVal> constInitVal) {
        std::visit(overloaded{
            [&](ConstInitVal::ConstInitValBasic& basic) {
                basic.exp->visit(shared_from_this());
            },
            [&](ConstInitVal::ConstInitValArray& array) {
                for (auto exp : array.exps) {
                    exp->visit(shared_from_this());
                }
            },
            [&](std::string& str) {

            }
        }, constInitVal->val);
    }

    void ASTVisitor::visitVarDecl(std::shared_ptr<VarDecl> varDecl) {
        varDecl->bType->visit(shared_from_this());
        for (auto def : varDecl->varDefs) {
            def->visit(shared_from_this());
        }
    }

    void ASTVisitor::visitVarDef(std::shared_ptr<VarDef> varDef) {
        std::visit(overloaded{
            [&](VarDef::VarDefBasic& basic) {

            },
            [&](VarDef::VarDefArray& array) {
                if (array.size != nullptr) {
                    array.size->visit(shared_from_this());
                }
            }
        }, varDef->def);
        if (varDef->val != nullptr) {
            varDef->val->visit(shared_from_this());
        }
    }

    void ASTVisitor::visitInitVal(std::shared_ptr<InitVal> initVal) {
        std::visit(overloaded{
            [&](InitVal::InitValBasic& basic) {
                basic.exp->visit(shared_from_this());
            },
            [&](InitVal::InitValArray& array) {
                for (auto exp : array.exps) {
                    exp->visit(shared_from_this());
                }
            },
            [&](std::string& str) {

            }
        }, initVal->val);
    }

    void ASTVisitor::visitFuncDef(std::shared_ptr<FuncDef> funcDef) {
        funcDef->funcType->visit(shared_from_this());
        if (funcDef->params != nullptr) {
            funcDef->params->visit(shared_from_this());
        }
        funcDef->block->visit(shared_from_this());
    }

    void ASTVisitor::visitMainFuncDef(std::shared_ptr<MainFuncDef> mainFuncDef) {
        mainFuncDef->block->visit(shared_from_this());
    }

    void ASTVisitor::visitFuncType(std::shared_ptr<FuncType> funcType) {

    }

    void ASTVisitor::visitFuncFParams(std::shared_ptr<FuncFParams> funcFParams) {
        for (auto funcFParam : funcFParams->params) {
            funcFParam->visit(shared_from_this());
        }
    }

    void ASTVisitor::visitFuncFParam(std::shared_ptr<FuncFParam> funcFParam) {
        funcFParam->bType->visit(shared_from_this());
    }

    void ASTVisitor::visitBlock(std::shared_ptr<Block> block) {
        for (auto item : block->items) {
            item->visit(shared_from_this());
        }
    }

    void ASTVisitor::visitBlockItem(std::shared_ptr<BlockItem> blockItem) {
        std::visit(overloaded{
            [&](std::shared_ptr<Decl> decl) {
                decl->visit(shared_from_this());
            },
            [&](std::shared_ptr<Stmt> stmt) {
                stmt->visit(shared_from_this());
            }
        }, blockItem->item);
    }

    void ASTVisitor::visitStmt(std::shared_ptr<Stmt> stmt) {
        std::visit(overloaded{
            [&](Stmt::StmtAssign& stmtAssign) {
                stmtAssign.lVal->visit(shared_from_this());
                stmtAssign.exp->visit(shared_from_this());
            },
            [&](std::shared_ptr<Exp> exp) {
                if (exp != nullptr)
                    exp->visit(shared_from_this());
            },
            [&](std::shared_ptr<Block> block) {
                block->visit(shared_from_this());
            },
            [&](Stmt::StmtIf& stmtIf) {
                stmtIf.cond->visit(shared_from_this());
                stmtIf.stmt->visit(shared_from_this());
                if (stmtIf.elseStmt != nullptr) {
                    stmtIf.elseStmt->visit(shared_from_this());
                }
            },
            [&](Stmt::StmtFor& stmtFor) {
                if (stmtFor.initStmt != nullptr) {
                    stmtFor.initStmt->visit(shared_from_this());
                }
                if (stmtFor.cond != nullptr) {
                    stmtFor.cond->visit(shared_from_this());
                }
                if (stmtFor.updateStmt != nullptr) {
                    stmtFor.updateStmt->visit(shared_from_this());
                }
                stmtFor.stmt->visit(shared_from_this());
            },
            [&](Stmt::BreakOrContinue& breakOrContinue) {

            },
            [&](Stmt::StmtReturn& stmtReturn) {
                if (stmtReturn.exp != nullptr)
                    stmtReturn.exp->visit(shared_from_this());
            },
            [&](Stmt::StmtRead& stmtRead) {
                stmtRead.lVal->visit(shared_from_this());
            },
            [&](Stmt::StmtPrintf& stmtPrintf) {
                for (auto exp : stmtPrintf.exps) {
                    exp->visit(shared_from_this());
                }
            },
        }, stmt->stmt);
    }

    void ASTVisitor::visitForStmt(std::shared_ptr<ForStmt> forStmt) {
        forStmt->lVal->visit(shared_from_this());
        forStmt->exp->visit(shared_from_this());
    }

    void ASTVisitor::visitExp(std::shared_ptr<Exp> exp) {
        exp->addExp->visit(shared_from_this());
    }

    void ASTVisitor::visitCond(std::shared_ptr<Cond> cond) {
        cond->lOrExp->visit(shared_from_this());
    }

    void ASTVisitor::visitLVal(std::shared_ptr<LVal> lVal) {
        if (lVal->exp != nullptr) {
            lVal->exp->visit(shared_from_this());
        }
    }

    void ASTVisitor::visitPrimaryExp(std::shared_ptr<PrimaryExp> primaryExp) {
        std::visit(overloaded{
            [&](std::shared_ptr<Exp> exp) {
                exp->visit(shared_from_this());
            },
            [&](std::shared_ptr<LVal> lVal) {
                lVal->visit(shared_from_this());
            },
            [&](std::shared_ptr<Number> number) {
                number->visit(shared_from_this());
            },
            [&](std::shared_ptr<Character> character) {
                character->visit(shared_from_this());
            }
        }, primaryExp->primaryExp);
    }

    void ASTVisitor::visitNumber(std::shared_ptr<Number> number) {

    }

    void ASTVisitor::visitCharacter(std::shared_ptr<Character> character) {

    }

    void ASTVisitor::visitUnaryExp(std::shared_ptr<UnaryExp> unaryExp) {
        std::visit(overloaded{
            [&](std::shared_ptr<PrimaryExp> primaryExp) {
                primaryExp->visit(shared_from_this());
            },
            [&](UnaryExp::FuncExp& funcExp) {
                if (funcExp.params != nullptr)
                    funcExp.params->visit(shared_from_this());
            },
            [&](UnaryExp::OpExp& opExp) {
                opExp.op->visit(shared_from_this());
                opExp.exp->visit(shared_from_this());
            }
        }, unaryExp->exp);
    }

    void ASTVisitor::visitUnaryOp(std::shared_ptr<UnaryOp> unaryOp) {

    }

    void ASTVisitor::visitFuncRParams(std::shared_ptr<FuncRParams> funcRParams) {
        for (auto param : funcRParams->params) {
            param->visit(shared_from_this());
        }
    }

    void ASTVisitor::visitMulExp(std::shared_ptr<MulExp> mulExp) {
        std::visit(overloaded{
            [&](std::shared_ptr<UnaryExp> unaryExp) {
                unaryExp->visit(shared_from_this());
            },
            [&](MulExp::OpExp& opExp) {
                opExp.mulExp->visit(shared_from_this());
                opExp.unaryExp->visit(shared_from_this());
            }
        }, mulExp->exp);
    }

    void ASTVisitor::visitAddExp(std::shared_ptr<AddExp> addExp) {
        std::visit(overloaded{
            [&](std::shared_ptr<MulExp> mulExp) {
                mulExp->visit(shared_from_this());
            },
            [&](AddExp::OpExp& opExp) {
                opExp.addExp->visit(shared_from_this());
                opExp.mulExp->visit(shared_from_this());
            }
        }, addExp->exp);
    }

    void ASTVisitor::visitRelExp(std::shared_ptr<RelExp> relExp) {
        std::visit(overloaded{
            [&](std::shared_ptr<AddExp> addExp) {
                addExp->visit(shared_from_this());
            },
            [&](RelExp::OpExp& opExp) {
                opExp.relExp->visit(shared_from_this());
                opExp.addExp->visit(shared_from_this());
            }
        }, relExp->exp);
    }

    void ASTVisitor::visitEqExp(std::shared_ptr<EqExp> eqExp) {
        std::visit(overloaded{
            [&](std::shared_ptr<RelExp> relExp) {
                relExp->visit(shared_from_this());
            },
            [&](EqExp::OpExp& opExp) {
                opExp.eqExp->visit(shared_from_this());
                opExp.relExp->visit(shared_from_this());
            }
        }, eqExp->exp);
    }

    void ASTVisitor::visitLAndExp(std::shared_ptr<LAndExp> lAndExp) {
        std::visit(overloaded{
            [&](std::shared_ptr<EqExp> eqExp) {
                eqExp->visit(shared_from_this());
            },
            [&](LAndExp::OpExp& opExp) {
                opExp.lAndExp->visit(shared_from_this());
                opExp.eqExp->visit(shared_from_this());
            }
        }, lAndExp->exp);
    }

    void ASTVisitor::visitLOrExp(std::shared_ptr<LOrExp> lOrExp) {
        std::visit(overloaded{
            [&](std::shared_ptr<LAndExp> lAndExp) {
                lAndExp->visit(shared_from_this());
            },
            [&](LOrExp::OpExp& opExp) {
                opExp.lOrExp->visit(shared_from_this());
                opExp.lAndExp->visit(shared_from_this());
            }
        }, lOrExp->exp);
    }

    void ASTVisitor::visitConstExp(std::shared_ptr<ConstExp> constExp) {
        constExp->addExp->visit(shared_from_this());
    }
} // thm