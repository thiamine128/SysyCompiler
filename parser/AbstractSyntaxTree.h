//
// Created by slty5 on 24-9-27.
//

#ifndef ABSTRACTSYNTAXTREE_H
#define ABSTRACTSYNTAXTREE_H
#include <memory>
#include <variant>
#include <vector>

#include "../core/Scope.h"
#include "../lexer/Token.h"
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

    class ASTNode : public std::enable_shared_from_this<ASTNode> {
    public:
        enum ASTNodeType {
#define X(a, b, c) c,
            ASTNODES
#undef X
        };
        int lineno;
        std::shared_ptr<Scope> scope;

        void consume(std::vector<Token>& tokens);
        virtual ASTNodeType nodeType() const { return ASTNode::DEFAULT; }
        virtual ~ASTNode() = default;
        virtual void visit(std::shared_ptr<ASTVisitor> visitor) {};
    };

    std::ostream& operator<<(std::ostream& os, const ASTNode& node);

    std::string nodeTypeToString(ASTNode::ASTNodeType type);

    class CompUnit : public ASTNode {
    public:
        std::vector<std::shared_ptr<Decl>> decls;
        std::vector<std::shared_ptr<FuncDef>> funcDefs;
        std::shared_ptr<MainFuncDef> mainFuncDef;

        ASTNodeType nodeType() const override {return ASTNode::COMPUNIT;}
        void visit(std::shared_ptr<ASTVisitor> visitor) override;
    };
    class Decl : public ASTNode {
    public:
        std::variant<std::shared_ptr<ConstDecl>, std::shared_ptr<VarDecl>> decl;

        ASTNodeType nodeType() const override {return ASTNode::DECL;}
        void visit(std::shared_ptr<ASTVisitor> visitor) override;
    };
    class ConstDecl : public ASTNode {
    public:
        std::shared_ptr<BType> bType;
        std::vector<std::shared_ptr<ConstDef>> constDefs;
        ASTNodeType nodeType() const override {return ASTNode::CONSTDECL;}
        void visit(std::shared_ptr<ASTVisitor> visitor) override;
    };
    class BType : public ASTNode {
    public:
        VariableType::Type type;
        ASTNodeType nodeType() const override {return ASTNode::BTYPE;}
        void visit(std::shared_ptr<ASTVisitor> visitor) override;
    };
    class ConstDef : public ASTNode {
    public:
        struct ConstDefBasic {};
        struct ConstDefArray {
            std::shared_ptr<ConstExp> size;

            ConstDefArray(std::shared_ptr<ConstExp> size) : size(size) {}
            ConstDefArray() {}
        };
        VariableType::Type type;
        Token ident;
        std::variant<ConstDefBasic, ConstDefArray> def;
        std::shared_ptr<ConstInitVal> val;
        ASTNodeType nodeType() const override {return ASTNode::CONSTDEF;}
        void visit(std::shared_ptr<ASTVisitor> visitor) override;
    };
    class ConstInitVal : public ASTNode {
    public:
        struct ConstInitValBasic {
            std::shared_ptr<ConstExp> exp;

            ConstInitValBasic(std::shared_ptr<ConstExp> exp) : exp(exp) {}
            ConstInitValBasic() {}
        };
        struct ConstInitValArray {
            std::vector<std::shared_ptr<ConstExp>> exps;

            ConstInitValArray(std::vector<std::shared_ptr<ConstExp>>& exps) : exps(exps) {}
            ConstInitValArray() {}
        };
        std::variant<ConstInitValBasic, ConstInitValArray, std::string> val;
        ASTNodeType nodeType() const override {return ASTNode::CONSTINITVAL;}
        void visit(std::shared_ptr<ASTVisitor> visitor) override;
    };
    class VarDecl : public ASTNode {
    public:
        std::shared_ptr<BType> bType;
        std::vector<std::shared_ptr<VarDef>> varDefs;

        ASTNodeType nodeType() const override {return ASTNode::VARDECL;}
        void visit(std::shared_ptr<ASTVisitor> visitor) override;
    };
    class VarDef : public ASTNode {
    public:
        struct VarDefBasic {};
        struct VarDefArray {
            std::shared_ptr<ConstExp> size;
            VarDefArray(std::shared_ptr<ConstExp> size) : size(size) {}
            VarDefArray() {}
        };
        VariableType::Type type;
        Token ident;
        std::variant<VarDefBasic, VarDefArray> def;
        std::shared_ptr<InitVal> val;
        ASTNodeType nodeType() const override {return ASTNode::VARDEF;}
        void visit(std::shared_ptr<ASTVisitor> visitor) override;
    };
    class InitVal : public ASTNode {
    public:
        struct InitValBasic {
            std::shared_ptr<Exp> exp;
            InitValBasic(std::shared_ptr<Exp> exp) : exp(exp) {}
            InitValBasic() {}
        };
        struct InitValArray {
            std::vector<std::shared_ptr<Exp>> exps;
            InitValArray(std::vector<std::shared_ptr<Exp>>& exps) : exps(exps) {}
            InitValArray() {}
        };
        std::variant<InitValBasic, InitValArray, std::string> val;
        ASTNodeType nodeType() const override {return ASTNode::INITVAL;}
        void visit(std::shared_ptr<ASTVisitor> visitor) override;
    };
    class FuncDef : public ASTNode {
    public:
        std::shared_ptr<FuncType> funcType;
        Token ident;
        std::shared_ptr<FuncFParams> params;
        std::shared_ptr<Block> block;

        ASTNodeType nodeType() const override {return ASTNode::FUNCDEF;}
        void visit(std::shared_ptr<ASTVisitor> visitor) override;
    };
    class MainFuncDef : public ASTNode {
    public:
        std::shared_ptr<Block> block;
        ASTNodeType nodeType() const override {return ASTNode::MAINFUNCDEF;}
        void visit(std::shared_ptr<ASTVisitor> visitor) override;
    };
    class FuncType : public ASTNode {
    public:
        FunctionSymbol::Type type;
        ASTNodeType nodeType() const override {return ASTNode::FUNCTYPE;}
        void visit(std::shared_ptr<ASTVisitor> visitor) override;
    };
    class FuncFParams : public ASTNode {
    public:
        std::vector<std::shared_ptr<FuncFParam>> params;
        ASTNodeType nodeType() const override {return ASTNode::FUNCFPARAMS;}
        void visit(std::shared_ptr<ASTVisitor> visitor) override;
    };
    class FuncFParam : public ASTNode {
    public:
        std::shared_ptr<BType> bType;
        Token ident;
        bool isArray;
        ASTNodeType nodeType() const override {return ASTNode::FUNCFPARAM;}
        void visit(std::shared_ptr<ASTVisitor> visitor) override;
    };
    class Block : public ASTNode {
    public:
        std::vector<std::shared_ptr<BlockItem>> items;
        Token rBrace;
        ASTNodeType nodeType() const override {return ASTNode::BLOCK;}
        void visit(std::shared_ptr<ASTVisitor> visitor) override;
    };
    class BlockItem : public ASTNode {
    public:
        std::variant<std::shared_ptr<Decl>, std::shared_ptr<Stmt>> item;
        ASTNodeType nodeType() const override {return ASTNode::BLOCKITEM;}
        void visit(std::shared_ptr<ASTVisitor> visitor) override;
    };
    class Stmt : public ASTNode {
    public:
        struct StmtAssign {
            std::shared_ptr<LVal> lVal;
            std::shared_ptr<Exp> exp;

            StmtAssign(std::shared_ptr<LVal> lVal, std::shared_ptr<Exp> exp) : lVal(lVal), exp(exp) {}
            StmtAssign() {}
        };
        struct StmtIf {
            std::shared_ptr<Cond> cond;
            std::shared_ptr<Stmt> stmt;
            std::shared_ptr<Stmt> elseStmt;

            StmtIf(std::shared_ptr<Cond> cond, std::shared_ptr<Stmt> stmt, std::shared_ptr<Stmt> elseStmt) : cond(cond), stmt(stmt), elseStmt(elseStmt) {}
            StmtIf() {}
        };
        struct StmtFor {
            std::shared_ptr<ForStmt> initStmt;
            std::shared_ptr<Cond> cond;
            std::shared_ptr<ForStmt> updateStmt;
            std::shared_ptr<Stmt> stmt;

            StmtFor(std::shared_ptr<ForStmt> initStmt, std::shared_ptr<Cond> cond, std::shared_ptr<ForStmt> updateStmt, std::shared_ptr<Stmt> stmt) : initStmt(initStmt), cond(cond), updateStmt(updateStmt), stmt(stmt) {}
            StmtFor() {}
        };
        enum BreakOrContinue {
            BREAK,
            CONTINUE
        };
        struct StmtReturn {
            std::shared_ptr<Exp> exp;

            StmtReturn(std::shared_ptr<Exp> exp) : exp(exp) {}
            StmtReturn() {}
        };
        struct StmtRead {
            std::shared_ptr<LVal> lVal;
            enum ReadType {
                INT,
                CHAR
            } type;

            StmtRead(std::shared_ptr<LVal> lVal, ReadType type) : lVal(lVal), type(type) {}
            StmtRead() {}
        };
        struct StmtPrintf {
            std::string fmt;
            std::vector<std::shared_ptr<Exp>> exps;
            Token printfToken;

            StmtPrintf(std::string const& fmt, std::vector<std::shared_ptr<Exp>>& exps, Token const& printfToken) : fmt(fmt), exps(exps), printfToken(printfToken) {}
            StmtPrintf() {}
        };


        std::variant<StmtAssign,
            std::shared_ptr<Exp>,
            std::shared_ptr<Block>,
            StmtIf,
            StmtFor,
            BreakOrContinue,
            StmtReturn,
            StmtRead,
            StmtPrintf> stmt;
        ASTNodeType nodeType() const override { return ASTNodeType::STMT; }
        void visit(std::shared_ptr<ASTVisitor> visitor) override;
    };
    class ForStmt : public ASTNode {
    public:
        std::shared_ptr<LVal> lVal;
        std::shared_ptr<Exp> exp;
        ASTNodeType nodeType() const override { return ASTNodeType::FORSTMT; }
        void visit(std::shared_ptr<ASTVisitor> visitor) override;
    };
    class Exp : public ASTNode {
    public:
        int len;
        std::shared_ptr<AddExp> addExp;
        bool isConst;
        int constVal;
        ASTNodeType nodeType() const override { return ASTNode::EXP; }
        void visit(std::shared_ptr<ASTVisitor> visitor) override;
        void evalConst(std::shared_ptr<SymbolTable> symbolTable);
    };
    class Cond : public ASTNode {
    public:
        std::shared_ptr<LOrExp> lOrExp;
        ASTNodeType nodeType() const override { return ASTNode::COND; }
        void visit(std::shared_ptr<ASTVisitor> visitor) override;
    };
    class LVal : public ASTNode {
    public:
        Token ident;
        std::shared_ptr<Exp> exp;
        bool isConst = false;
        int constVal = 0;

        ASTNodeType nodeType() const override { return ASTNode::LVAL; }
        void visit(std::shared_ptr<ASTVisitor> visitor) override;
        void evalConst(std::shared_ptr<SymbolTable> symbolTable);
    };
    class PrimaryExp : public ASTNode {
    public:
        std::variant<
            std::shared_ptr<Exp>,
            std::shared_ptr<LVal>,
            std::shared_ptr<Number>,
            std::shared_ptr<Character>> primaryExp;
        bool isConst = false;
        int constVal = 0;
        ASTNodeType nodeType() const override { return ASTNode::PRIMARYEXP; }
        void visit(std::shared_ptr<ASTVisitor> visitor) override;
        void evalConst();
    };
    class Number : public ASTNode {
    public:
        int num;
        ASTNodeType nodeType() const override { return ASTNode::NUMBER; }
        void visit(std::shared_ptr<ASTVisitor> visitor) override;
    };
    class Character : public ASTNode {
    public:
        char ch;
        ASTNodeType nodeType() const override { return ASTNode::CHARACTER; }
        void visit(std::shared_ptr<ASTVisitor> visitor) override;
    };
    class UnaryExp : public ASTNode {
    public:
        struct FuncExp {
            Token ident;
            std::shared_ptr<FuncRParams> params;

            FuncExp(Token const& ident, std::shared_ptr<FuncRParams> params) : ident(ident), params(params) {}
        };
        struct OpExp {
            std::shared_ptr<UnaryOp> op;
            std::shared_ptr<UnaryExp> exp;

            OpExp(std::shared_ptr<UnaryOp> op, std::shared_ptr<UnaryExp> exp) : op(op), exp(exp) {}
        };
        std::variant<std::shared_ptr<PrimaryExp>, FuncExp, OpExp> exp;
        bool isConst = false;
        int constVal = 0;

        ASTNodeType nodeType() const override { return ASTNode::UNARYEXP; }
        void visit(std::shared_ptr<ASTVisitor> visitor) override;
        void evalConst();
    };
    class UnaryOp : public ASTNode {
    public:
        enum Type {
            PLUS, MINUS, NOT
        } type;
        ASTNodeType nodeType() const override { return ASTNode::UNARYOP; }
        void visit(std::shared_ptr<ASTVisitor> visitor) override;
    };
    class FuncRParams : public ASTNode {
    public:
        std::vector<std::shared_ptr<Exp>> params;
        ASTNodeType nodeType() const override { return ASTNode::FUNCRPARAMS; }
        void visit(std::shared_ptr<ASTVisitor> visitor) override;
    };
    class MulExp : public ASTNode {
    public:
        struct OpExp {
            std::shared_ptr<MulExp> mulExp;
            enum Op {
                MUL, DIV, MOD
            } op;
            std::shared_ptr<UnaryExp> unaryExp;

            OpExp(std::shared_ptr<MulExp> mulExp, Op op, std::shared_ptr<UnaryExp> unaryExp) : mulExp(mulExp), op(op), unaryExp(unaryExp) {}
        };
        std::variant<std::shared_ptr<UnaryExp>, OpExp> exp;
        bool isConst = false;
        int constVal = 0;
        ASTNodeType nodeType() const override { return ASTNode::MULEXP; }
        void visit(std::shared_ptr<ASTVisitor> visitor) override;
        void evalConst();
    };
    class AddExp : public ASTNode {
    public:
        struct OpExp {
            std::shared_ptr<AddExp> addExp;
            enum Op {
                ADD, MINUS
            } op;
            std::shared_ptr<MulExp> mulExp;
            OpExp(std::shared_ptr<AddExp> addExp, Op op, std::shared_ptr<MulExp> mulExp) : addExp(addExp), op(op), mulExp(mulExp) {}
        };
        std::variant<std::shared_ptr<MulExp>, OpExp> exp;
        bool isConst = false;
        int constVal = 0;
        ASTNodeType nodeType() const override { return ASTNode::ADDEXP; }
        void visit(std::shared_ptr<ASTVisitor> visitor) override;
        void evalConst();
    };
    class RelExp : public ASTNode {
    public:
        struct OpExp {
            std::shared_ptr<RelExp> relExp;
            enum Op {
                GT, LT, GE, LE
            } op;
            std::shared_ptr<AddExp> addExp;

            OpExp(std::shared_ptr<RelExp> relExp, Op op, std::shared_ptr<AddExp> addExp) : relExp(relExp), op(op), addExp(addExp) {}
        };
        std::variant<std::shared_ptr<AddExp>, OpExp> exp;
        ASTNodeType nodeType() const override { return ASTNode::RELEXP; }
        void visit(std::shared_ptr<ASTVisitor> visitor) override;
    };
    class EqExp : public ASTNode {
    public:
        struct OpExp {
            std::shared_ptr<EqExp> eqExp;
            enum Op {
                EQ, NEQ
            } op;
            std::shared_ptr<RelExp> relExp;

            OpExp(std::shared_ptr<EqExp> eqExp, Op op, std::shared_ptr<RelExp> relExp) : eqExp(eqExp), op(op), relExp(relExp) {}
        };
        std::variant<std::shared_ptr<RelExp>, OpExp> exp;
        ASTNodeType nodeType() const override { return ASTNode::EQEXP; }
        void visit(std::shared_ptr<ASTVisitor> visitor) override;
    };
    class LAndExp : public ASTNode {
    public:
        struct OpExp {
            std::shared_ptr<LAndExp> lAndExp;
            std::shared_ptr<EqExp> eqExp;

            OpExp(std::shared_ptr<LAndExp> lAndExp, std::shared_ptr<EqExp> eqExp) : lAndExp(lAndExp), eqExp(eqExp) {}
        };
        std::variant<std::shared_ptr<EqExp>, OpExp> exp;
        ASTNodeType nodeType() const override { return ASTNode::LANDEXP; }
        void visit(std::shared_ptr<ASTVisitor> visitor) override;
    };
    class LOrExp : public ASTNode {
    public:
        struct OpExp {
            std::shared_ptr<LOrExp> lOrExp;
            std::shared_ptr<LAndExp> lAndExp;

            OpExp(std::shared_ptr<LOrExp> lOrExp, std::shared_ptr<LAndExp> lAndExp) : lOrExp(lOrExp), lAndExp(lAndExp) {}
        };
        std::variant<std::shared_ptr<LAndExp>, OpExp> exp;
        ASTNodeType nodeType() const override { return ASTNode::LOREXP; }
        void visit(std::shared_ptr<ASTVisitor> visitor) override;
    };
    class ConstExp : public ASTNode {
    public:
        std::shared_ptr<AddExp> addExp;
        int constVal;
        bool isConst = true;

        ASTNodeType nodeType() const override { return ASTNode::CONSTEXP; }
        void visit(std::shared_ptr<ASTVisitor> visitor) override;
        void evalConst(std::shared_ptr<SymbolTable> symbolTable);
    };
    class AbstractSyntaxTree {

    };

} // thm

#endif //ABSTRACTSYNTAXTREE_H
