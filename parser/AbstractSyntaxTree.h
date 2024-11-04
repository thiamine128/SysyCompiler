//
// Created by slty5 on 24-9-27.
//

#ifndef ABSTRACTSYNTAXTREE_H
#define ABSTRACTSYNTAXTREE_H
#include <variant>
#include <vector>

#include "../core/Scope.h"
#include "../lexer/Token.h"
#include "../llvm/LLVM.h"
#include "../semantic/Symbol.h"

#define ASTNODES \
    X(Default, "Default", DEFAULT) \
    X(LAndExp, "LAndExp", LANDEXP) \
    X(RelExp, "RelExp", RELEXP) \
    X(MulExp, "MulExp", MULEXP) \
    X(UnaryExp, "UnaryExp", UNARYEXP) \
    X(PrimaryExp, "PrimaryExp", PRIMARYEXP) \
    X(CompUnit, "CompUnit", COMPUNIT) \
    X(FuncFParams, "FuncFParams", FUNCFPARAMS) \
    X(EqExp, "EqExp", EQEXP) \
    X(UnaryOp, "UnaryOp", UNARYOP) \
    X(FuncRParams, "FuncRParams", FUNCRPARAMS) \
    X(Character, "Character", CHARACTER) \
    X(Number, "Number", NUMBER) \
    X(LOrExp, "LOrExp", LOREXP) \
    X(AddExp, "AddExp", ADDEXP) \
    X(ForStmt, "ForStmt", FORSTMT) \
    X(Cond, "Cond", COND) \
    X(LVal, "LVal", LVAL) \
    X(Stmt, "Stmt", STMT) \
    X(BlockItem, "BlockItem", BLOCKITEM) \
    X(FuncFParam, "FuncFParam", FUNCFPARAM) \
    X(Block, "Block", BLOCK) \
    X(FuncType, "FuncType", FUNCTYPE) \
    X(Exp, "Exp", EXP) \
    X(InitVal, "InitVal", INITVAL) \
    X(VarDef, "VarDef", VARDEF) \
    X(ConstExp, "ConstExp", CONSTEXP) \
    X(ConstInitVal, "ConstInitVal", CONSTINITVAL) \
    X(ConstDef, "ConstDef", CONSTDEF) \
    X(BType, "BType", BTYPE) \
    X(VarDecl, "VarDecl", VARDECL) \
    X(ConstDecl, "ConstDecl", CONSTDECL) \
    X(MainFuncDef, "MainFuncDef", MAINFUNCDEF) \
    X(FuncDef, "FuncDef", FUNCDEF) \
    X(Decl, "Decl", DECL) \

namespace thm {
    class SymbolTable;
    class ASTVisitor;
#define X(a, b, c) class a;
    ASTNODES
#undef X

    class ASTNode {
    public:
        enum ASTNodeType {
#define X(a, b, c) c,
            ASTNODES
#undef X
        };
        int lineno;
        Scope* scope;
        Value* value;

        static void consume(std::vector<Token>& tokens);
        virtual ASTNodeType nodeType() const { return ASTNode::DEFAULT; }
        virtual ~ASTNode() = default;
        virtual void visit(ASTVisitor* visitor) {};
    };

    std::ostream& operator<<(std::ostream& os, const ASTNode& node);

    std::string nodeTypeToString(ASTNode::ASTNodeType type);

    class CompUnit : public ASTNode {
    public:
        std::vector<Decl*> decls;
        std::vector<FuncDef*> funcDefs;
        MainFuncDef* mainFuncDef;

        ASTNodeType nodeType() const override {return ASTNode::COMPUNIT;}
        void visit(ASTVisitor* visitor) override;
    };
    class Decl : public ASTNode {
    public:
        std::variant<ConstDecl*, VarDecl*> decl;

        ASTNodeType nodeType() const override {return ASTNode::DECL;}
        void visit(ASTVisitor* visitor) override;
    };
    class ConstDecl : public ASTNode {
    public:
        BType* bType;
        std::vector<ConstDef*> constDefs;
        ASTNodeType nodeType() const override {return ASTNode::CONSTDECL;}
        void visit(ASTVisitor* visitor) override;
    };
    class BType : public ASTNode {
    public:
        VariableType::Type type;
        ASTNodeType nodeType() const override {return ASTNode::BTYPE;}
        void visit(ASTVisitor* visitor) override;
    };
    class ConstDef : public ASTNode {
    public:
        struct ConstDefBasic {};
        struct ConstDefArray {
            ConstExp* size = nullptr;

            ConstDefArray(ConstExp* size) : size(size) {}
            ConstDefArray() {}
        };
        VariableType::Type type;
        Token ident;
        std::variant<ConstDefBasic, ConstDefArray> def;
        ConstInitVal* val;
        VariableSymbol* symbol;
        bool lazyArrayLen;
        ASTNodeType nodeType() const override {return ASTNode::CONSTDEF;}
        void visit(ASTVisitor* visitor) override;
    };
    class ConstInitVal : public ASTNode {
    public:
        struct ConstInitValBasic {
            ConstExp* exp = nullptr;

            ConstInitValBasic(ConstExp* exp) : exp(exp) {}
            ConstInitValBasic() {}
        };
        struct ConstInitValArray {
            std::vector<ConstExp*> exps;

            ConstInitValArray(const std::vector<ConstExp*>& exps) : exps(exps) {}
            ConstInitValArray() {}
        };
        ConstDef* constDef;
        std::variant<ConstInitValBasic, ConstInitValArray, std::string> val;
        ASTNodeType nodeType() const override {return ASTNode::CONSTINITVAL;}
        void visit(ASTVisitor* visitor) override;
    };
    class VarDecl : public ASTNode {
    public:
        BType* bType;
        std::vector<VarDef*> varDefs;

        ASTNodeType nodeType() const override {return ASTNode::VARDECL;}
        void visit(ASTVisitor* visitor) override;
    };
    class VarDef : public ASTNode {
    public:
        struct VarDefBasic {};
        struct VarDefArray {
            ConstExp* size = nullptr;
            VarDefArray(ConstExp* size) : size(size) {}
            VarDefArray() {}
        };
        VariableType::Type type;
        Token ident;
        std::variant<VarDefBasic, VarDefArray> def;
        InitVal* val;
        VariableSymbol* symbol;
        bool lazyArrayLen;
        ASTNodeType nodeType() const override {return ASTNode::VARDEF;}
        void visit(ASTVisitor* visitor) override;
    };
    class InitVal : public ASTNode {
    public:
        struct InitValBasic {
            Exp* exp = nullptr;
            InitValBasic(Exp* exp) : exp(exp) {}
            InitValBasic() {}
        };
        struct InitValArray {
            std::vector<Exp*> exps;
            InitValArray(const std::vector<Exp*>& exps) : exps(exps) {}
            InitValArray() {}
        };
        VarDef* varDef;
        std::variant<InitValBasic, InitValArray, std::string> val;
        ASTNodeType nodeType() const override {return ASTNode::INITVAL;}
        void visit(ASTVisitor* visitor) override;
    };
    class FuncDef : public ASTNode {
    public:
        FuncType* funcType;
        Token ident;
        FuncFParams* params;
        Block* block;
        FunctionSymbol* symbol;

        ASTNodeType nodeType() const override {return ASTNode::FUNCDEF;}
        void visit(ASTVisitor* visitor) override;
    };
    class MainFuncDef : public ASTNode {
    public:
        Block* block;
        ASTNodeType nodeType() const override {return ASTNode::MAINFUNCDEF;}
        void visit(ASTVisitor* visitor) override;
    };
    class FuncType : public ASTNode {
    public:
        FunctionSymbol::Type type;
        ASTNodeType nodeType() const override {return ASTNode::FUNCTYPE;}
        void visit(ASTVisitor* visitor) override;
    };
    class FuncFParams : public ASTNode {
    public:
        std::vector<FuncFParam*> params;
        ASTNodeType nodeType() const override {return ASTNode::FUNCFPARAMS;}
        void visit(ASTVisitor* visitor) override;
    };
    class FuncFParam : public ASTNode {
    public:
        BType* bType;
        Token ident;
        bool isArray;
        VariableSymbol* symbol;
        ASTNodeType nodeType() const override {return ASTNode::FUNCFPARAM;}
        void visit(ASTVisitor* visitor) override;
    };
    class Block : public ASTNode {
    public:
        std::vector<BlockItem*> items;
        Token rBrace;
        ASTNodeType nodeType() const override {return ASTNode::BLOCK;}
        void visit(ASTVisitor* visitor) override;
    };
    class BlockItem : public ASTNode {
    public:
        std::variant<Decl*, Stmt*> item;
        ASTNodeType nodeType() const override {return ASTNode::BLOCKITEM;}
        void visit(ASTVisitor* visitor) override;
    };
    class Stmt : public ASTNode {
    public:
        struct StmtAssign {
            LVal* lVal = nullptr;
            Exp* exp = nullptr;

            StmtAssign(LVal* lVal, Exp* exp) : lVal(lVal), exp(exp) {}
            StmtAssign() {}
        };
        struct StmtIf {
            Cond* cond = nullptr;
            Stmt* stmt = nullptr;
            Stmt* elseStmt = nullptr;

            StmtIf(Cond* cond, Stmt* stmt, Stmt* elseStmt) : cond(cond), stmt(stmt), elseStmt(elseStmt) {}
            StmtIf() {}
        };
        struct StmtFor {
            ForStmt* initStmt = nullptr;
            Cond* cond = nullptr;
            ForStmt* updateStmt = nullptr;
            Stmt* stmt = nullptr;

            StmtFor(ForStmt* initStmt, Cond* cond, ForStmt* updateStmt, Stmt* stmt) : initStmt(initStmt), cond(cond), updateStmt(updateStmt), stmt(stmt) {}
            StmtFor() {}
        };
        enum BreakOrContinue {
            BREAK,
            CONTINUE
        };
        struct StmtReturn {
            Exp* exp = nullptr;

            StmtReturn(Exp* exp) : exp(exp) {}
            StmtReturn() {}
        };
        struct StmtRead {
            LVal* lVal = nullptr;
            enum ReadType {
                INT,
                CHAR
            } type = INT;

            StmtRead(LVal* lVal, ReadType type) : lVal(lVal), type(type) {}
            StmtRead() {}
        };
        struct StmtPrintf {
            std::string fmt;
            std::vector<Exp*> exps;
            Token printfToken;

            StmtPrintf(std::string const& fmt, const std::vector<Exp*>& exps, Token const& printfToken) : fmt(fmt), exps(exps), printfToken(printfToken) {}
            StmtPrintf() {}
        };


        std::variant<StmtAssign,
            Exp*,
            Block*,
            StmtIf,
            StmtFor,
            BreakOrContinue,
            StmtReturn,
            StmtRead,
            StmtPrintf> stmt;
        ASTNodeType nodeType() const override { return ASTNodeType::STMT; }
        void visit(ASTVisitor* visitor) override;
    };
    class ForStmt : public ASTNode {
    public:
        LVal* lVal;
        Exp* exp;
        ASTNodeType nodeType() const override { return ASTNodeType::FORSTMT; }
        void visit(ASTVisitor* visitor) override;
    };
    class Exp : public ASTNode {
    public:
        int len;
        AddExp* addExp;
        bool isConst;
        int constVal;
        ASTNodeType nodeType() const override { return ASTNode::EXP; }
        void visit(ASTVisitor* visitor) override;
        void evalConst();
        BasicValueType *getBasicType() const;
    };
    class Cond : public ASTNode {
    public:
        LOrExp *lOrExp;
        BasicBlock *ifTrue;
        BasicBlock *ifFalse;
        BasicBlock *block;

        ASTNodeType nodeType() const override { return ASTNode::COND; }
        void visit(ASTVisitor* visitor) override;
    };
    class LVal : public ASTNode {
    public:
        Token ident;
        Exp* exp;
        bool isConst = false;
        int constVal = 0;
        VariableSymbol* symbol;

        ASTNodeType nodeType() const override { return ASTNode::LVAL; }
        void visit(ASTVisitor* visitor) override;
        void evalConst();
        BasicValueType* getBasicType() const;
    };
    class PrimaryExp : public ASTNode {
    public:
        std::variant<
            Exp*,
            LVal*,
            Number*,
            Character*> primaryExp;
        bool isConst = false;
        int constVal = 0;
        ASTNodeType nodeType() const override { return ASTNode::PRIMARYEXP; }
        void visit(ASTVisitor* visitor) override;
        void evalConst();
    };
    class Number : public ASTNode {
    public:
        int num;
        ASTNodeType nodeType() const override { return ASTNode::NUMBER; }
        void visit(ASTVisitor* visitor) override;
    };
    class Character : public ASTNode {
    public:
        char ch;
        ASTNodeType nodeType() const override { return ASTNode::CHARACTER; }
        void visit(ASTVisitor* visitor) override;
    };
    class UnaryExp : public ASTNode {
    public:
        struct FuncExp {
            Token ident;
            FuncRParams* params;

            FuncExp(Token const& ident, FuncRParams* params) : ident(ident), params(params) {}
        };
        struct OpExp {
            UnaryOp* op;
            UnaryExp* exp;

            OpExp(UnaryOp* op, UnaryExp* exp) : op(op), exp(exp) {}
        };
        std::variant<PrimaryExp*, FuncExp, OpExp> exp;
        bool isConst = false;
        int constVal = 0;

        ASTNodeType nodeType() const override { return ASTNode::UNARYEXP; }
        void visit(ASTVisitor* visitor) override;
        void evalConst();
        BasicValueType *getBasicType() const;
    };
    class UnaryOp : public ASTNode {
    public:
        enum Type {
            PLUS, MINUS, NOT
        } type;
        ASTNodeType nodeType() const override { return ASTNode::UNARYOP; }
        void visit(ASTVisitor* visitor) override;
    };
    class FuncRParams : public ASTNode {
    public:
        std::vector<Exp*> params;
        std::vector<Value*> values;
        ASTNodeType nodeType() const override { return ASTNode::FUNCRPARAMS; }
        void visit(ASTVisitor* visitor) override;
    };
    class MulExp : public ASTNode {
    public:
        struct OpExp {
            MulExp* mulExp;
            enum Op {
                MUL, DIV, MOD
            } op;
            UnaryExp* unaryExp;

            OpExp(MulExp* mulExp, Op op, UnaryExp* unaryExp) : mulExp(mulExp), op(op), unaryExp(unaryExp) {}
        };
        std::variant<UnaryExp*, OpExp> exp;
        bool isConst = false;
        int constVal = 0;
        ASTNodeType nodeType() const override { return ASTNode::MULEXP; }
        void visit(ASTVisitor* visitor) override;
        void evalConst();
    };
    class AddExp : public ASTNode {
    public:
        struct OpExp {
            AddExp* addExp;
            enum Op {
                ADD, MINUS
            } op;
            MulExp* mulExp;
            OpExp(AddExp* addExp, Op op, MulExp* mulExp) : addExp(addExp), op(op), mulExp(mulExp) {}
        };
        std::variant<MulExp*, OpExp> exp;
        bool isConst = false;
        int constVal = 0;
        ASTNodeType nodeType() const override { return ASTNode::ADDEXP; }
        void visit(ASTVisitor* visitor) override;
        void evalConst();
    };
    class RelExp : public ASTNode {
    public:
        struct OpExp {
            RelExp* relExp;
            enum Op {
                GT, LT, GE, LE
            } op;
            AddExp* addExp;

            OpExp(RelExp* relExp, Op op, AddExp* addExp) : relExp(relExp), op(op), addExp(addExp) {}
        };
        std::variant<AddExp*, OpExp> exp;
        ASTNodeType nodeType() const override { return ASTNode::RELEXP; }
        void visit(ASTVisitor* visitor) override;
        BasicValueType *getBasicType() const;
    };
    class EqExp : public ASTNode {
    public:
        struct OpExp {
            EqExp* eqExp;
            enum Op {
                EQ, NEQ
            } op;
            RelExp* relExp;

            OpExp(EqExp* eqExp, Op op, RelExp* relExp) : eqExp(eqExp), op(op), relExp(relExp) {}
        };
        std::variant<RelExp*, OpExp> exp;
        ASTNodeType nodeType() const override { return ASTNode::EQEXP; }
        void visit(ASTVisitor* visitor) override;
        BasicValueType *getBasicType() const;
    };
    class LAndExp : public ASTNode {
    public:
        struct OpExp {
            LAndExp* lAndExp;
            EqExp* eqExp;

            OpExp(LAndExp* lAndExp, EqExp* eqExp) : lAndExp(lAndExp), eqExp(eqExp) {}
        };
        BasicBlock *ifTrue;
        BasicBlock *ifFalse;
        BasicBlock *create;
        BasicBlock *block;

        std::variant<EqExp*, OpExp> exp;
        ASTNodeType nodeType() const override { return ASTNode::LANDEXP; }
        void visit(ASTVisitor* visitor) override;
    };
    class LOrExp : public ASTNode {
    public:
        struct OpExp {
            LOrExp* lOrExp;
            LAndExp* lAndExp;

            OpExp(LOrExp* lOrExp, LAndExp* lAndExp) : lOrExp(lOrExp), lAndExp(lAndExp) {}
        };
        BasicBlock *ifTrue;
        BasicBlock *ifFalse;
        BasicBlock *create;
        BasicBlock *block;

        std::variant<LAndExp*, OpExp> exp;
        ASTNodeType nodeType() const override { return ASTNode::LOREXP; }
        void visit(ASTVisitor* visitor) override;
    };
    class ConstExp : public ASTNode {
    public:
        AddExp* addExp;
        int constVal;
        bool isConst = true;

        ASTNodeType nodeType() const override { return ASTNode::CONSTEXP; }
        void visit(ASTVisitor* visitor) override;
        void evalConst();
    };
    class AbstractSyntaxTree {

    };

} // thm

#endif //ABSTRACTSYNTAXTREE_H
