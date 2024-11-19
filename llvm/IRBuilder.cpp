//
// Created by slty5 on 24-10-27.
//

#include "IRBuilder.h"

#include <complex>
#include <iostream>
#include <bits/fs_fwd.h>

#include "../util/util.h"

namespace thm {
    IRBuilder::IRBuilder() {
        module = new Module();
        currentBlock = nullptr;
        currentFunction = nullptr;
    }

    GlobalVariable* IRBuilder::globalVariableFromSymbol(VariableSymbol *symbol) {
        // 1. Create global value
        // 2. Set global var symbol value
        // 3. Set global var init
        GlobalVariable* globalVariable = new GlobalVariable();
        globalVariable->name = symbol->ident.content;
        globalVariable->valueType = new PtrValueType(ValueType::fromVariableType(symbol->type));
        globalVariable->zeroInit = !symbol->hasInit;
        globalVariable->initVal = symbol->initVal;
        globalVariable->initVals = symbol->initVals;
        symbol->value = globalVariable;
        return globalVariable;
    }

    void IRBuilder::submitInst(Instruction *inst) {
        if (currentBlock != nullptr) {
            currentBlock->addInst(inst);
        }
    }

    void IRBuilder::submitBlock(BasicBlock *block) {
        currentFunction->blocks.push_back(block);
    }

    void IRBuilder::doAssign(LVal *lVal, Exp *exp) {
        Value *rVal = exp->value;
        BasicValueType *expType = exp->getBasicType();
        BasicValueType *innerType = lVal->getBasicType();
        if (expType->basicType == BasicValueType::I8 && innerType->basicType == BasicValueType::I32) {
            // assign i8 to i32
            ZextInst *zexInst = new ZextInst(rVal);
            submitInst(zexInst);
            rVal = zexInst;
        } else if (expType->basicType == BasicValueType::I32 && innerType->basicType == BasicValueType::I8) {
            // assign i32 to i8
            TruncInst *truncInst = new TruncInst(rVal);
            submitInst(truncInst);
            rVal = truncInst;
        }
        StoreInst *storeInst = new StoreInst(rVal, lVal->value);
        submitInst(storeInst);
    }

    void IRBuilder::visitConstDecl(ConstDecl* constDecl) {
        // Nothing to translate
        ASTVisitor::visitConstDecl(constDecl);
    }

    void IRBuilder::visitVarDecl(VarDecl* varDecl) {
        // Nothing to translate
        ASTVisitor::visitVarDecl(varDecl);
    }

    void IRBuilder::visitFuncDef(FuncDef* funcDef) {
        // 1. Create function
        // 2. Load format args
        // 3. Create new block
        // 4. Slot format args
        // 5. Copy args
        funcDef->funcType->visit(this);
        Function* function = new Function(funcDef->ident.content, BasicValueType::fromFunctionType(funcDef->symbol->type));
        funcDef->value = function;
        funcDef->symbol->value = function;
        module->functions.push_back(function);
        currentFunction = function;
        if (funcDef->params != nullptr) {
            funcDef->params->visit(this);
        }
        currentBlock = new BasicBlock(currentFunction, forBlocks.size());

        if (funcDef->params != nullptr) {
            for (int i = 0; i < function->args.size(); i++) {
                AllocaInst* allocaInst = new AllocaInst(function->args[i]->valueType);
                funcDef->params->params[i]->symbol->value = allocaInst;
                submitInst(allocaInst);
                submitInst(new StoreInst(function->args[i], allocaInst, true));
            }
        }

        funcDef->block->visit(this);
        if (funcDef->symbol->type == FunctionSymbol::VOID && (currentBlock->insts.empty() || currentBlock->insts.back()->type() != LLVMType::RET_INST)) {
            submitInst(new RetInst(nullptr));
        }
        submitBlock(currentBlock);
        currentBlock = nullptr;
        currentFunction = nullptr;
    }

    void IRBuilder::visitMainFuncDef(MainFuncDef* mainFuncDef) {
        Function* function = new Function("main", BasicValueType::I32);
        mainFuncDef->value = function;
        module->main = function;
        currentFunction = function;
        currentBlock = new BasicBlock(currentFunction, forBlocks.size());
        ASTVisitor::visitMainFuncDef(mainFuncDef);
        submitBlock(currentBlock);
        currentBlock = nullptr;
        currentFunction = nullptr;
    }

    void IRBuilder::visitStmt(Stmt* stmt) {
        std::visit(overloaded{
            [&](const Stmt::StmtAssign& stmtAssign) {
                stmtAssign.lVal->visit(this);
                stmtAssign.exp->visit(this);
                doAssign(stmtAssign.lVal, stmtAssign.exp);
            },
            [&](Exp* exp) {
                if (exp != nullptr) {
                    exp->visit(this);
                    stmt->value = exp->value;
                }
            },
            [&](Block* block) {
                // Nothing to translate
                block->visit(this);
            },
            [&](const Stmt::StmtIf& stmtIf) {
                submitBlock(currentBlock);
                stmtIf.cond->ifTrue = new BasicBlock(currentFunction, forBlocks.size());
                stmtIf.cond->ifFalse = new BasicBlock(currentFunction, forBlocks.size());
                BasicBlock* after = new BasicBlock(currentFunction, forBlocks.size());
                stmtIf.cond->visit(this);

                currentBlock = stmtIf.cond->ifTrue;
                stmtIf.stmt->visit(this);
                submitInst(new BranchInst(after));
                submitBlock(currentBlock);
                currentBlock = stmtIf.cond->ifFalse;
                if (stmtIf.elseStmt != nullptr) {
                    stmtIf.elseStmt->visit(this);
                }
                submitInst(new BranchInst(after));
                submitBlock(currentBlock);
                currentBlock = after;
            },
            [&](const Stmt::StmtFor& stmtFor) {
                BasicBlock *initBlock = currentBlock;
                BasicBlock *condBlock = new BasicBlock(currentFunction, forBlocks.size() + 1);
                BasicBlock *stmtBlock = new BasicBlock(currentFunction, forBlocks.size() + 1);
                BasicBlock *updateBlock = new BasicBlock(currentFunction, forBlocks.size() + 1);
                BasicBlock *afterBlock = new BasicBlock(currentFunction, forBlocks.size());
                forBlocks.push_back({initBlock, condBlock, stmtBlock, updateBlock, afterBlock});
                currentBlock = initBlock;
                if (stmtFor.initStmt != nullptr) {
                    stmtFor.initStmt->visit(this);
                }
                submitInst(new BranchInst(condBlock));
                submitBlock(currentBlock);

                currentBlock = condBlock;
                submitBlock(currentBlock);
                if (stmtFor.cond != nullptr) {
                    stmtFor.cond->ifTrue = stmtBlock;
                    stmtFor.cond->ifFalse = afterBlock;
                    stmtFor.cond->visit(this);
                } else {
                    submitInst(new BranchInst(stmtBlock));
                }


                currentBlock = stmtBlock;
                stmtFor.stmt->visit(this);
                submitInst(new BranchInst(updateBlock));
                submitBlock(currentBlock);

                currentBlock = updateBlock;
                if (stmtFor.updateStmt != nullptr) {
                    stmtFor.updateStmt->visit(this);
                }
                submitInst(new BranchInst(condBlock));
                submitBlock(currentBlock);

                forBlocks.pop_back();
                currentBlock = afterBlock;
            },
            [&](Stmt::BreakOrContinue& breakOrContinue) {
                switch (breakOrContinue) {
                    case Stmt::BREAK:
                        submitInst(new BranchInst(forBlocks.back().afterBlock));
                        break;
                    case Stmt::CONTINUE:
                        submitInst(new BranchInst(forBlocks.back().updateBlock));
                        break;
                }
            },
            [&](const Stmt::StmtReturn& stmtReturn) {
                Value* value = nullptr;

                if (stmtReturn.exp != nullptr) {
                    stmtReturn.exp->visit(this);
                    value = stmtReturn.exp->value;
                    BasicValueType* requiredType = static_cast<BasicValueType *>(currentFunction->valueType);
                    BasicValueType* returnType = stmtReturn.exp->getBasicType();
                    if (requiredType->basicType == BasicValueType::I32 && returnType->basicType == BasicValueType::I8) {
                        ZextInst *zextInst = new ZextInst(value);
                        submitInst(zextInst);
                        value = zextInst;
                    } else if (requiredType->basicType == BasicValueType::I8 && returnType->basicType == BasicValueType::I32) {
                        TruncInst *truncInst = new TruncInst(value);
                        submitInst(truncInst);
                        value = truncInst;
                    }
                }
                RetInst* retInst = new RetInst(value);
                submitInst(retInst);
            },
            [&](const Stmt::StmtRead& stmtRead) {
                stmtRead.lVal->visit(this);
                CallInst* callInst = nullptr;
                BasicValueType *basicType = stmtRead.lVal->getBasicType();
                Value *rVal = nullptr;
                switch (stmtRead.type) {
                    case Stmt::StmtRead::INT:
                        callInst = new CallInst(true, module->getInt, {});
                        submitInst(callInst);
                        if (basicType->basicType == BasicValueType::I8) {
                            TruncInst *truncInst = new TruncInst(callInst);
                            submitInst(truncInst);
                            rVal = truncInst;
                        } else {
                            rVal = callInst;
                        }
                        break;
                    case Stmt::StmtRead::CHAR:
                        callInst = new CallInst(true, module->getChar, {});
                        submitInst(callInst);
                        if (basicType->basicType == BasicValueType::I8) {
                            TruncInst *truncInst = new TruncInst(callInst);
                            submitInst(truncInst);
                            rVal = truncInst;
                        } else {
                            rVal = callInst;
                        }
                        break;
                }
                StoreInst *storeInst = new StoreInst(rVal, stmtRead.lVal->value);
                submitInst(storeInst);
            },
            [&](Stmt::StmtPrintf& stmtPrintf) {
                for (auto exp : stmtPrintf.exps) {
                    exp->visit(this);
                }
                int expInd = 0;
                int last = 0;
                std::string fmt = fromRaw(stmtPrintf.fmt.c_str());
                for (int i = 0; i < fmt.length(); i++) {
                    if (fmt[i] == '%') {
                        if (fmt[i + 1] == 'd') {
                            std::string sep = fmt.substr(last, i - last);
                            if (sep.length() > 0) {
                                StringLiteral* str = module->addStringLiteral(sep);
                                GetElementPtr* addr = new GetElementPtr(str, new NumericLiteral(0, BasicValueType::I32));
                                submitInst(addr);
                                CallInst* putStr = new CallInst(false, module->putStr, {addr});
                                submitInst(putStr);
                            }
                            Value* p = stmtPrintf.exps[expInd]->value;
                            BasicValueType* basicType = stmtPrintf.exps[expInd]->getBasicType();
                            if (basicType->basicType == BasicValueType::I8) {
                                ZextInst* zextInst = new ZextInst(stmtPrintf.exps[expInd]->value);
                                submitInst(zextInst);
                                p = zextInst;
                            }
                            CallInst* put = new CallInst(false, module->putInt, {p});
                            expInd++;
                            submitInst(put);
                            i++;
                            last = i + 1;
                        } else if (fmt[i + 1] == 'c') {
                            std::string sep = fmt.substr(last, i - last);
                            if (sep.length() > 0) {
                                StringLiteral* str = module->addStringLiteral(sep);
                                GetElementPtr* addr = new GetElementPtr(str, new NumericLiteral(0, BasicValueType::I32));
                                submitInst(addr);
                                CallInst* putStr = new CallInst(false, module->putStr, {addr});
                                submitInst(putStr);
                            }
                            Value* p = stmtPrintf.exps[expInd]->value;
                            BasicValueType* basicType = stmtPrintf.exps[expInd]->getBasicType();
                            if (basicType->basicType == BasicValueType::I32) {
                                TruncInst* truncInst = new TruncInst(stmtPrintf.exps[expInd]->value);
                                submitInst(truncInst);
                                p = truncInst;
                            }
                            CallInst* put = new CallInst(false, module->putChar, {p});
                            expInd++;
                            submitInst(put);
                            i++;
                            last = i + 1;
                        }
                    }
                }
                if (last < fmt.length()) {
                    std::string sep = fmt.substr(last, fmt.length() - last);
                    if (sep.length() > 0) {
                        StringLiteral* str = module->addStringLiteral(sep);
                        GetElementPtr* addr = new GetElementPtr(str, new NumericLiteral(0, BasicValueType::I32));
                        submitInst(addr);
                        CallInst* putStr = new CallInst(false, module->putStr, {addr});
                        submitInst(putStr);
                    }
                }
            },
        }, stmt->stmt);
    }

    void IRBuilder::visitForStmt(ForStmt* forStmt) {
        forStmt->lVal->visit(this);
        forStmt->exp->visit(this);
        doAssign(forStmt->lVal, forStmt->exp);
    }

    void IRBuilder::visitCompUnit(CompUnit* compUnit) {
        ASTVisitor::visitCompUnit(compUnit);
    }

    void IRBuilder::visitBlock(Block* block) {
        ASTVisitor::visitBlock(block);
    }

    void IRBuilder::visitLVal(LVal* lVal) {
        PtrValueType* ptr = static_cast<PtrValueType*>(lVal->symbol->value->valueType);
        if (lVal->exp != nullptr) {
            lVal->exp->visit(this);
        }
        GetElementPtr* elementPtr = nullptr;
        Value* idx = nullptr;
        LoadInst* loadInst = nullptr;
        switch (ptr->value->type()) {
            case ValueType::ARRAY:
                if (lVal->exp != nullptr) {
                    idx = lVal->exp->value;
                } else {
                    idx = new NumericLiteral(0, BasicValueType::I32);
                }
                elementPtr = new GetElementPtr(lVal->symbol->value, idx);
                submitInst(elementPtr);
                lVal->value = elementPtr;
                break;
            case ValueType::PTR:
                loadInst = new LoadInst(lVal->symbol->value);
                submitInst(loadInst);
                if (lVal->exp != nullptr) {
                    idx = lVal->exp->value;
                } else {
                    idx = new NumericLiteral(0, BasicValueType::I32);
                }
                elementPtr = new GetElementPtr(loadInst, idx);
                submitInst(elementPtr);
                lVal->value = elementPtr;
                break;
            case ValueType::BASIC:
                lVal->value = lVal->symbol->value;
                break;
        }
    }

    void IRBuilder::visitUnaryExp(UnaryExp* unaryExp) {
        std::visit(overloaded{
            [&](PrimaryExp* primaryExp) {
                primaryExp->visit(this);
                unaryExp->value = primaryExp->value;
            },
            [&](const UnaryExp::FuncExp& funcExp) {
                if (funcExp.params != nullptr)
                    funcExp.params->visit(this);
                FunctionSymbol* symbol = static_cast<FunctionSymbol*>(unaryExp->scope->symbolTable->findSymbol(funcExp.ident.content));
                std::vector<Value *> params;
                if (funcExp.params != nullptr) {
                    for (int i = 0; i < funcExp.params->values.size(); i++) {
                        BasicValueType *basicType = static_cast<BasicValueType *>(funcExp.params->values[i]->valueType);
                        if (basicType->basicType == BasicValueType::I32 && symbol->paramTypes[i].type == VariableType::CHAR) {
                            TruncInst *trunc = new TruncInst(funcExp.params->values[i]);
                            submitInst(trunc);
                            params.push_back(trunc);
                        } else if (basicType->basicType == BasicValueType::I8 && symbol->paramTypes[i].type == VariableType::INT) {
                            ZextInst *zext = new ZextInst(funcExp.params->values[i]);
                            submitInst(zext);
                            params.push_back(zext);
                        } else {
                            params.push_back(funcExp.params->values[i]);
                        }
                    }
                }
                CallInst* callInst = new CallInst(symbol->type != FunctionSymbol::VOID, symbol->value, params);
                unaryExp->value = callInst;
                submitInst(callInst);
            },
            [&](const UnaryExp::OpExp& opExp) {
                opExp.op->visit(this);
                opExp.exp->visit(this);
                BinaryInst* bin;
                switch (opExp.op->type) {
                    case UnaryOp::PLUS:
                        bin = new BinaryInst(BinaryInst::ADD, new NumericLiteral(0, opExp.exp->getBasicType()->basicType), opExp.exp->value);
                        break;
                    case UnaryOp::MINUS:
                        bin = new BinaryInst(BinaryInst::SUB, new NumericLiteral(0, opExp.exp->getBasicType()->basicType), opExp.exp->value);
                        break;
                    case UnaryOp::NOT:
                        bin = new BinaryInst(BinaryInst::EQ, new NumericLiteral(0, opExp.exp->getBasicType()->basicType), opExp.exp->value);
                        break;
                }
                submitInst(bin);
                unaryExp->value = bin;
            }
        }, unaryExp->exp);
    }

    void IRBuilder::visitDecl(Decl* decl) {
        ASTVisitor::visitDecl(decl);
    }

    void IRBuilder::visitBType(BType* bType) {
        ASTVisitor::visitBType(bType);
    }

    void IRBuilder::visitConstDef(ConstDef* constDef) {
        if (constDef->scope->scopeId == 1) {
            GlobalVariable* globalVariable = globalVariableFromSymbol(constDef->symbol);
            constDef->value = globalVariable;
            module->variables.push_back(globalVariable);
        } else {
            VariableSymbol* variableSymbol = constDef->symbol;
            AllocaInst* allocaInst = new AllocaInst(variableSymbol->type);
            constDef->value = allocaInst;
            variableSymbol->value = allocaInst;
            submitInst(allocaInst);
            if (constDef->val != nullptr) {
                constDef->val->visit(this);
            }
        }
    }

    void IRBuilder::visitConstInitVal(ConstInitVal* constInitVal) {
        std::visit(overloaded{
            [&](const ConstInitVal::ConstInitValBasic& basic) {
                basic.exp->visit(this);
                BasicValueType *basicType = static_cast<BasicValueType *>(basic.exp->value->valueType);
                if (constInitVal->constDef->symbol->type.type == VariableType::INT && basicType->basicType == BasicValueType::I8) {
                    ZextInst *zext = new ZextInst(basic.exp->value);
                    submitInst(zext);
                    submitInst(new StoreInst(zext, constInitVal->constDef->value));
                } else if (constInitVal->constDef->symbol->type.type == VariableType::CHAR && basicType->basicType == BasicValueType::I32) {
                    TruncInst* trunc = new TruncInst(basic.exp->value);
                    submitInst(trunc);
                    submitInst(new StoreInst(trunc, constInitVal->constDef->value));
                } else {
                    submitInst(new StoreInst(basic.exp->value, constInitVal->constDef->value));
                }
            },
            [&](ConstInitVal::ConstInitValArray& array) {
                for (int i = 0; i < constInitVal->constDef->symbol->type.arrayLen; i++) {
                    if (i < array.exps.size())
                        array.exps[i]->visit(this);
                    GetElementPtr* getElementPtr = new GetElementPtr(constInitVal->constDef->value, new NumericLiteral(i, BasicValueType::I32));
                    submitInst(getElementPtr);
                    Value* value;
                    BasicValueType *basicType;
                    if (i < array.exps.size()) {
                        value = array.exps[i]->value;
                        basicType = static_cast<BasicValueType *>(array.exps[i]->value->valueType);
                    } else {
                        BasicValueType::BasicType basic = constInitVal->constDef->symbol->type.type == VariableType::INT ? BasicValueType::I32 : BasicValueType::I8;
                        value = new NumericLiteral(0, basic);
                        basicType = new BasicValueType(basic);
                    }
                    if (constInitVal->constDef->symbol->type.type == VariableType::INT && basicType->basicType == BasicValueType::I8) {
                        ZextInst *zext = new ZextInst(value);
                        submitInst(zext);
                        value = zext;
                    } else if (constInitVal->constDef->symbol->type.type == VariableType::CHAR && basicType->basicType == BasicValueType::I32) {
                        TruncInst* trunc = new TruncInst(value);
                        submitInst(trunc);
                        value = trunc;
                    }
                    StoreInst* storeInst = new StoreInst(value, getElementPtr);
                    submitInst(storeInst);
                }
            },
            [&](std::string& str) {
                std::string s = fromRaw(str.c_str());
                for (int i = 0; i < constInitVal->constDef->symbol->type.arrayLen; i++) {
                    GetElementPtr* getElementPtr = new GetElementPtr(constInitVal->constDef->value, new NumericLiteral(i, BasicValueType::I32));
                    submitInst(getElementPtr);
                    Value* value;
                    if (i < s.length()) {
                        value = new NumericLiteral(s[i], BasicValueType::I8);
                    } else {
                        value = new NumericLiteral(0, BasicValueType::I8);
                    }
                    StoreInst* storeInst = new StoreInst(value, getElementPtr);
                    submitInst(storeInst);
                }
            }
        }, constInitVal->val);
    }

    void IRBuilder::visitVarDef(VarDef* varDef) {
        if (varDef->scope->scopeId == 1) {
            GlobalVariable* globalVariable = globalVariableFromSymbol(varDef->symbol);
            module->variables.push_back(globalVariable);
            varDef->value = globalVariable;
        } else {
            VariableSymbol* variableSymbol = varDef->symbol;
            AllocaInst* allocaInst = new AllocaInst(variableSymbol->type);
            submitInst(allocaInst);
            varDef->value = allocaInst;
            varDef->symbol->value = allocaInst;
            if (varDef->val != nullptr) {
                varDef->val->visit(this);
            }
        }
    }

    void IRBuilder::visitInitVal(InitVal* initVal) {
        std::visit(overloaded{
            [&](const InitVal::InitValBasic& basic) {
                basic.exp->visit(this);
                BasicValueType *basicType = static_cast<BasicValueType *>(basic.exp->value->valueType);
                if (initVal->varDef->symbol->type.type == VariableType::INT && basicType->basicType == BasicValueType::I8) {
                    ZextInst *zext = new ZextInst(basic.exp->value);
                    submitInst(zext);
                    submitInst(new StoreInst(zext, initVal->varDef->value));
                } else if (initVal->varDef->symbol->type.type == VariableType::CHAR && basicType->basicType == BasicValueType::I32) {
                    TruncInst* trunc = new TruncInst(basic.exp->value);
                    submitInst(trunc);
                    submitInst(new StoreInst(trunc, initVal->varDef->value));
                } else {
                    submitInst(new StoreInst(basic.exp->value, initVal->varDef->value));
                }
            },
            [&](InitVal::InitValArray& array) {
                for (int i = 0; i < array.exps.size(); i++) {
                    array.exps[i]->visit(this);
                    GetElementPtr* getElementPtr = new GetElementPtr(initVal->varDef->value, new NumericLiteral(i, BasicValueType::I32));
                    submitInst(getElementPtr);
                    Value* value = array.exps[i]->value;
                    BasicValueType *basicType = static_cast<BasicValueType *>(array.exps[i]->value->valueType);
                    if (initVal->varDef->symbol->type.type == VariableType::INT && basicType->basicType == BasicValueType::I8) {
                        ZextInst *zext = new ZextInst(value);
                        submitInst(zext);
                        value = zext;
                    } else if (initVal->varDef->symbol->type.type == VariableType::CHAR && basicType->basicType == BasicValueType::I32) {
                        TruncInst* trunc = new TruncInst(value);
                        submitInst(trunc);
                        value = trunc;
                    }
                    StoreInst* storeInst = new StoreInst(value, getElementPtr);
                    submitInst(storeInst);
                }
            },
            [&](std::string& str) {
                std::string s = fromRaw(str.c_str());
                for (int i = 0; i < initVal->varDef->symbol->type.arrayLen; i++) {
                    GetElementPtr* getElementPtr = new GetElementPtr(initVal->varDef->value, new NumericLiteral(i, BasicValueType::I32));
                    submitInst(getElementPtr);
                    Value* value;
                    if (i < s.length()) {
                        value = new NumericLiteral(s[i], BasicValueType::I8);
                    } else {
                        value = new NumericLiteral(0, BasicValueType::I8);
                    }
                    StoreInst* storeInst = new StoreInst(value, getElementPtr);
                    submitInst(storeInst);
                }
            }
        }, initVal->val);
    }

    void IRBuilder::visitFuncType(FuncType* funcType) {
        ASTVisitor::visitFuncType(funcType);
    }

    void IRBuilder::visitFuncFParams(FuncFParams* funcFParams) {
        ASTVisitor::visitFuncFParams(funcFParams);
    }

    void IRBuilder::visitFuncFParam(FuncFParam* funcFParam) {
        Argument* arg = new Argument();
        arg->valueType = new BasicValueType(funcFParam->symbol->type.type == VariableType::INT ? BasicValueType::I32 : BasicValueType::I8);
        if (funcFParam->symbol->type.isArray) {
            arg->valueType = new PtrValueType(arg->valueType);
        }
        currentFunction->args.push_back(arg);
        ASTVisitor::visitFuncFParam(funcFParam);
    }

    void IRBuilder::visitBlockItem(BlockItem* blockItem) {
        ASTVisitor::visitBlockItem(blockItem);
    }

    void IRBuilder::visitExp(Exp* exp) {
        ASTVisitor::visitExp(exp);
        exp->value = exp->addExp->value;
    }

    void IRBuilder::visitCond(Cond* cond) {
        cond->block = currentBlock;

        cond->lOrExp->ifTrue = cond->ifTrue;
        cond->lOrExp->ifFalse = cond->ifFalse;
        cond->lOrExp->block = cond->block;

        cond->lOrExp->visit(this);
    }

    void IRBuilder::visitPrimaryExp(PrimaryExp* primaryExp) {
        std::visit(overloaded{
            [&](Exp* exp) {
                exp->visit(this);
                primaryExp->value = exp->value;
            },
            [&](LVal* lVal) {
                lVal->visit(this);
                if (lVal->symbol->type.isArray && lVal->exp == nullptr) {
                    primaryExp->value = lVal->value;
                } else {
                    LoadInst* loadInst = new LoadInst(lVal->value);
                    submitInst(loadInst);
                    Value* value = loadInst;
                    if (loadInst->valueType->type() == ValueType::BASIC) {
                        BasicValueType* basic = static_cast<BasicValueType *>(loadInst->valueType);
                        if (basic->basicType == BasicValueType::I8) {
                            ZextInst *zext = new ZextInst(value);
                            submitInst(zext);
                            value = zext;
                        }
                    }
                    primaryExp->value = value;
                }
            },
            [&](Number* number) {
                number->visit(this);
                primaryExp->value = new NumericLiteral(number->num, BasicValueType::I32);
            },
            [&](Character* character) {
                character->visit(this);
                primaryExp->value = new NumericLiteral(character->ch, BasicValueType::I32);
            }
        }, primaryExp->primaryExp);
    }

    void IRBuilder::visitNumber(Number* number) {
        ASTVisitor::visitNumber(number);
    }

    void IRBuilder::visitCharacter(Character* character) {
        ASTVisitor::visitCharacter(character);
    }

    void IRBuilder::visitUnaryOp(UnaryOp* unaryOp) {
        ASTVisitor::visitUnaryOp(unaryOp);
    }

    void IRBuilder::visitFuncRParams(FuncRParams* funcRParams) {
        for (Exp* exp : funcRParams->params) {
            exp->visit(this);
            funcRParams->values.push_back(exp->value);
        }
    }

    void IRBuilder::visitMulExp(MulExp* mulExp) {
        std::visit(overloaded{
            [&](UnaryExp* unaryExp) {
                unaryExp->visit(this);
                mulExp->value = unaryExp->value;
            },
            [&](const MulExp::OpExp& opExp) {
                opExp.mulExp->visit(this);
                opExp.unaryExp->visit(this);
                BinaryInst::Op op = BinaryInst::MUL;
                switch (opExp.op) {
                    case MulExp::OpExp::MUL:
                        op = BinaryInst::MUL;
                        break;
                    case MulExp::OpExp::DIV:
                        op = BinaryInst::SDIV;
                        break;
                    case MulExp::OpExp::MOD:
                        op = BinaryInst::SREM;
                        break;
                }
                BinaryInst* binaryInst = new BinaryInst(op, opExp.mulExp->value, opExp.unaryExp->value);
                mulExp->value = binaryInst;
                submitInst(binaryInst);
            }
        }, mulExp->exp);
    }

    void IRBuilder::visitAddExp(AddExp* addExp) {
        std::visit(overloaded{
            [&](MulExp* mulExp) {
                mulExp->visit(this);
                addExp->value = mulExp->value;
            },
            [&](const AddExp::OpExp& opExp) {
                opExp.addExp->visit(this);
                opExp.mulExp->visit(this);
                BinaryInst::Op op = BinaryInst::ADD;
                switch (opExp.op) {
                    case AddExp::OpExp::ADD:
                        op = BinaryInst::ADD;
                    break;
                    case AddExp::OpExp::MINUS:
                        op = BinaryInst::SUB;
                    break;
                }
                BinaryInst* add = new BinaryInst(op, opExp.addExp->value, opExp.mulExp->value);
                addExp->value = add;
                submitInst(add);
            }
        }, addExp->exp);
    }

    void IRBuilder::visitRelExp(RelExp* relExp) {
        std::visit(overloaded{
            [&](AddExp* addExp) {
                addExp->visit(this);
                relExp->value = addExp->value;
            },
            [&](const RelExp::OpExp& opExp) {
                opExp.relExp->visit(this);
                opExp.addExp->visit(this);
                Value *l = opExp.relExp->value;;
                if (opExp.relExp->getBasicType()->basicType == BasicValueType::I1) {
                    ZextInst *zext = new ZextInst(l);
                    submitInst(zext);
                    l = zext;
                }
                BinaryInst::Op op = BinaryInst::SGT;
                switch (opExp.op) {
                    case RelExp::OpExp::GT:
                        op = BinaryInst::SGT;
                        break;
                    case RelExp::OpExp::LT:
                        op = BinaryInst::SLT;
                        break;
                    case RelExp::OpExp::GE:
                        op = BinaryInst::SGE;
                        break;
                    case RelExp::OpExp::LE:
                        op = BinaryInst::SLE;
                        break;
                }

                BinaryInst *cmp = new BinaryInst(op, l, opExp.addExp->value);
                submitInst(cmp);
                relExp->value = cmp;
            }
        }, relExp->exp);
    }

    void IRBuilder::visitEqExp(EqExp* eqExp) {
        std::visit(overloaded{
            [&](RelExp* relExp) {
                relExp->visit(this);
                eqExp->value = relExp->value;
            },
            [&](const EqExp::OpExp& opExp) {
                opExp.eqExp->visit(this);
                opExp.relExp->visit(this);
                BinaryInst::Op op = BinaryInst::EQ;
                switch (opExp.op) {
                    case EqExp::OpExp::EQ:
                        op = BinaryInst::EQ;
                        break;
                    case EqExp::OpExp::NEQ:
                        op = BinaryInst::NE;
                        break;
                }
                Value *l = opExp.eqExp->value;;
                Value *r = opExp.relExp->value;
                if (opExp.eqExp->getBasicType()->basicType == BasicValueType::I32 && opExp.relExp->getBasicType()->basicType == BasicValueType::I1) {
                    ZextInst *zext = new ZextInst(r);
                    submitInst(zext);
                    r = zext;
                } else if (opExp.eqExp->getBasicType()->basicType == BasicValueType::I1 && opExp.relExp->getBasicType()->basicType == BasicValueType::I32) {
                    ZextInst *zext = new ZextInst(l);
                    submitInst(zext);
                    l = zext;
                }
                BinaryInst *cmp = new BinaryInst(op, l, r);
                submitInst(cmp);
                eqExp->value = cmp;
            }
        }, eqExp->exp);
    }

    void IRBuilder::visitLAndExp(LAndExp* lAndExp) {
        std::visit(overloaded{
            [&](EqExp* eqExp) {
                eqExp->visit(this);
                Value *exp = eqExp->value;
                if (eqExp->getBasicType()->basicType != BasicValueType::I1) {
                    BinaryInst *ne = new BinaryInst(BinaryInst::NE, exp, new NumericLiteral(0, eqExp->getBasicType()->basicType));
                    submitInst(ne);
                    exp = ne;
                }
                BranchInst *br = new BranchInst(exp, lAndExp->ifTrue, lAndExp->ifFalse);
                submitInst(br);
                lAndExp->value = exp;
            },
            [&](const LAndExp::OpExp& opExp) {
                lAndExp->create = new BasicBlock(currentFunction, forBlocks.size());
                opExp.lAndExp->ifTrue = lAndExp->create;
                opExp.lAndExp->ifFalse = lAndExp->ifFalse;
                opExp.lAndExp->block = lAndExp->block;
                opExp.lAndExp->visit(this);
                submitBlock(lAndExp->create);
                currentBlock = lAndExp->create;
                opExp.eqExp->visit(this);
                Value *exp = opExp.eqExp->value;
                if (opExp.eqExp->getBasicType()->basicType != BasicValueType::I1) {
                    BinaryInst *ne = new BinaryInst(BinaryInst::NE, exp, new NumericLiteral(0, opExp.eqExp->getBasicType()->basicType));
                    submitInst(ne);
                    exp = ne;
                }

                BranchInst *br = new BranchInst(exp, lAndExp->ifTrue, lAndExp->ifFalse);
                submitInst(br);
                lAndExp->value = exp;
            }
        }, lAndExp->exp);
    }

    void IRBuilder::visitLOrExp(LOrExp* lOrExp) {
        std::visit(overloaded{
            [&](LAndExp* lAndExp) {
                lAndExp->ifTrue = lOrExp->ifTrue;
                lAndExp->ifFalse = lOrExp->ifFalse;
                lAndExp->block = lOrExp->block;
                lAndExp->visit(this);
                lOrExp->value = lAndExp->value;
            },
            [&](const LOrExp::OpExp& opExp) {
                lOrExp->create = new BasicBlock(currentFunction, forBlocks.size());
                opExp.lOrExp->ifTrue = lOrExp->ifTrue;
                opExp.lOrExp->ifFalse = lOrExp->create;
                opExp.lOrExp->block = lOrExp->block;
                opExp.lOrExp->visit(this);
                submitBlock(lOrExp->create);

                currentBlock = lOrExp->create;
                opExp.lAndExp->ifTrue = lOrExp->ifTrue;
                opExp.lAndExp->ifFalse = lOrExp->ifFalse;
                opExp.lAndExp->block = lOrExp->create;
                opExp.lAndExp->visit(this);
            }
        }, lOrExp->exp);
    }

    void IRBuilder::visitConstExp(ConstExp* constExp) {
        ASTVisitor::visitConstExp(constExp);
        constExp->value = constExp->addExp->value;
    }
} // thm