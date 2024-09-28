//
// Created by slty5 on 24-9-27.
//

#ifndef ABSTRACTSYNTAXTREE_H
#define ABSTRACTSYNTAXTREE_H
#include <memory>
#include <variant>
#include <vector>

#include "../lexer/Token.h"

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
        std::vector<Token> tokens;

        void consume(std::vector<Token>& tokens);
        virtual ASTNodeType nodeType() const { return ASTNode::DEFAULT; }
        virtual ~ASTNode() = default;
    };

    std::ostream& operator<<(std::ostream& os, const ASTNode& node);

    std::string nodeTypeToString(ASTNode::ASTNodeType type);

    class CompUnit : public ASTNode {
    public:
        std::vector<std::unique_ptr<Decl>> decls;
        std::vector<std::unique_ptr<FuncDef>> funcDefs;
        std::unique_ptr<MainFuncDef> mainFuncDef;

        ASTNodeType nodeType() const override {return ASTNode::COMPUNIT;}
    };
    class Decl : public ASTNode {
    public:
        std::variant<std::unique_ptr<ConstDecl>, std::unique_ptr<VarDecl>> decl;

        ASTNodeType nodeType() const override {return ASTNode::DECL;}
    };
    class ConstDecl : public ASTNode {
    public:
        std::unique_ptr<BType> bType;
        std::vector<std::unique_ptr<ConstDef>> constDefs;
        ASTNodeType nodeType() const override {return ASTNode::CONSTDECL;}
    };
    class BType : public ASTNode {
    public:
        enum Type {
            INT,
            CHAR
        } type;
        ASTNodeType nodeType() const override {return ASTNode::BTYPE;}
    };
    class ConstDef : public ASTNode {
    public:
        struct ConstDefBasic {
            std::string ident;
        };
        struct ConstDefArray {
            std::string ident;
            std::unique_ptr<ConstExp> index;
        };
        std::variant<ConstDefBasic, ConstDefArray> def;
        std::unique_ptr<ConstInitVal> val;
        ASTNodeType nodeType() const override {return ASTNode::CONSTDEF;}
    };
    class ConstInitVal : public ASTNode {
    public:
        struct  ConstInitValBasic {
            std::unique_ptr<ConstExp> exp;
        };
        struct  ConstInitValArray {
            std::vector<std::unique_ptr<ConstExp>> exps;
        };
        std::variant<ConstInitValBasic, ConstInitValArray, std::string> val;
        ASTNodeType nodeType() const override {return ASTNode::CONSTINITVAL;}
    };
    class VarDecl : public ASTNode {
    public:
        std::unique_ptr<BType> bType;
        std::vector<std::unique_ptr<VarDef>> varDefs;
        ASTNodeType nodeType() const override {return ASTNode::VARDECL;}
    };
    class VarDef : public ASTNode {
    public:
        struct  VarDefBasic {
            std::string ident;
        };
        struct  VarDefArray {
            std::string ident;
            std::unique_ptr<ConstExp> size;
        };
        std::variant<VarDefBasic, VarDefArray> def;
        std::unique_ptr<InitVal> val;
        ASTNodeType nodeType() const override {return ASTNode::VARDEF;}
    };
    class InitVal : public ASTNode {
    public:
        struct  InitValBasic {
            std::unique_ptr<Exp> exp;
        };
        struct  InitValArray {
            std::vector<std::unique_ptr<Exp>> exps;
        };
        std::variant<InitValBasic, InitValArray, std::string> val;
        ASTNodeType nodeType() const override {return ASTNode::INITVAL;}
    };
    class FuncDef : public ASTNode {
    public:
        std::unique_ptr<FuncType> funcType;
        std::string ident;
        std::unique_ptr<FuncFParams> params;
        std::unique_ptr<Block> block;
        ASTNodeType nodeType() const override {return ASTNode::FUNCDEF;}
    };
    class MainFuncDef : public ASTNode {
    public:
        std::unique_ptr<Block> block;
        ASTNodeType nodeType() const override {return ASTNode::MAINFUNCDEF;}
    };
    class FuncType : public ASTNode {
    public:
        enum Type {
            VOID, INT, CHAR
        } type;
        ASTNodeType nodeType() const override {return ASTNode::FUNCTYPE;}
    };
    class FuncFParams : public ASTNode {
    public:
        std::vector<std::unique_ptr<FuncFParam>> params;
        ASTNodeType nodeType() const override {return ASTNode::FUNCFPARAMS;}
    };
    class FuncFParam : public ASTNode {
    public:
        std::unique_ptr<BType> bType;
        std::string ident;
        bool isArray;
        ASTNodeType nodeType() const override {return ASTNode::FUNCFPARAM;}
    };
    class Block : public ASTNode {
    public:
        std::vector<std::unique_ptr<BlockItem>> items;
        ASTNodeType nodeType() const override {return ASTNode::BLOCK;}
    };
    class BlockItem : public ASTNode {
    public:
        std::variant<std::unique_ptr<Decl>, std::unique_ptr<Stmt>> item;
        ASTNodeType nodeType() const override {return ASTNode::BLOCKITEM;}
    };
    class Stmt : public ASTNode {
    public:
        struct  StmtAssign {
            std::unique_ptr<LVal> lVal;
            std::unique_ptr<Exp> exp;
        };
        struct  StmtIf {
            std::unique_ptr<Cond> cond;
            std::unique_ptr<Stmt> stmt;
            std::unique_ptr<Stmt> elseStmt;
        };
        struct  StmtFor {
            std::unique_ptr<ForStmt> initStmt;
            std::unique_ptr<Cond> cond;
            std::unique_ptr<ForStmt> updateStmt;
            std::unique_ptr<Stmt> stmt;
        };
        enum BreakOrContinue {
            BREAK,
            CONTINUE
        };
        struct  StmtReturn {
            std::unique_ptr<Exp> exp;
        };
        struct  StmtRead {
            std::unique_ptr<LVal> lVal;
            enum ReadType {
                INT,
                CHAR
            } type;
        };
        struct  StmtPrintf {
            std::string fmt;
            std::vector<std::unique_ptr<Exp>> exps;
        };


        std::variant<StmtAssign,
            std::unique_ptr<Exp>,
            std::unique_ptr<Block>,
            StmtIf,
            StmtFor,
            BreakOrContinue,
            StmtReturn,
            StmtRead,
            StmtPrintf> stmt;
        ASTNodeType nodeType() const override { return ASTNodeType::STMT; }
    };
    class ForStmt : public ASTNode {
    public:
        std::unique_ptr<LVal> lVal;
        std::unique_ptr<Exp> exp;
        ASTNodeType nodeType() const override { return ASTNodeType::FORSTMT; }
    };
    class Exp : public ASTNode {
    public:
        std::unique_ptr<AddExp> addExp;
        ASTNodeType nodeType() const override { return ASTNode::EXP; }
    };
    class Cond : public ASTNode {
    public:
        std::unique_ptr<LOrExp> lOrExp;
        ASTNodeType nodeType() const override { return ASTNode::COND; }
    };
    class LVal : public ASTNode {
    public:
        std::string ident;
        std::unique_ptr<Exp> exp;
        ASTNodeType nodeType() const override { return ASTNode::LVAL; }
    };
    class PrimaryExp : public ASTNode {
    public:
        std::variant<std::unique_ptr<Exp>, std::unique_ptr<LVal>, std::unique_ptr<Number>, std::unique_ptr<Character>> primaryExp;
        ASTNodeType nodeType() const override { return ASTNode::PRIMARYEXP; }
    };
    class Number : public ASTNode {
    public:
        int num;
        ASTNodeType nodeType() const override { return ASTNode::NUMBER; }
    };
    class Character : public ASTNode {
    public:
        char ch;
        ASTNodeType nodeType() const override { return ASTNode::CHARACTER; }
    };
    class UnaryExp : public ASTNode {
    public:
        struct  FuncExp {
            std::string ident;
            std::unique_ptr<FuncRParams> params;
        };
        struct  OpExp {
            std::unique_ptr<UnaryOp> op;
            std::unique_ptr<UnaryExp> exp;
        };
        std::variant<std::unique_ptr<PrimaryExp>, FuncExp, OpExp> exp;
        ASTNodeType nodeType() const override { return ASTNode::UNARYEXP; }
    };
    class UnaryOp : public ASTNode {
    public:
        enum Type {
            PLUS, MINUS, NOT
        } type;
        ASTNodeType nodeType() const override { return ASTNode::UNARYOP; }
    };
    class FuncRParams : public ASTNode {
    public:
        std::vector<std::unique_ptr<Exp>> params;
        ASTNodeType nodeType() const override { return ASTNode::FUNCRPARAMS; }
    };
    class MulExp : public ASTNode {
    public:
        struct  OpExp {
            std::unique_ptr<MulExp> mulExp;
            enum Op {
                MUL, DIV, MOD
            } op;
            std::unique_ptr<UnaryExp> unaryExp;
        };
        std::variant<std::unique_ptr<UnaryExp>, OpExp> exp;
        ASTNodeType nodeType() const override { return ASTNode::MULEXP; }
    };
    class AddExp : public ASTNode {
    public:
        struct OpExp {
            std::unique_ptr<AddExp> addExp;
            enum Op {
                ADD, MINUS
            } op;
            std::unique_ptr<MulExp> mulExp;
        };
        std::variant<std::unique_ptr<MulExp>, OpExp> exp;
        ASTNodeType nodeType() const override { return ASTNode::ADDEXP; }
    };
    class RelExp : public ASTNode {
    public:
        struct OpExp {
            std::unique_ptr<RelExp> relExp;
            enum Op {
                GT, LT, GE, LE
            } op;
            std::unique_ptr<AddExp> addExp;
        };
        std::variant<std::unique_ptr<AddExp>, OpExp> exp;
        ASTNodeType nodeType() const override { return ASTNode::RELEXP; }
    };
    class EqExp : public ASTNode {
    public:
        struct OpExp {
            std::unique_ptr<EqExp> eqExp;
            enum Op {
                EQ, NEQ
            } op;
            std::unique_ptr<RelExp> relExp;
        };
        std::variant<std::unique_ptr<RelExp>, OpExp> exp;
        ASTNodeType nodeType() const override { return ASTNode::EQEXP; }
    };
    class LAndExp : public ASTNode {
    public:
        struct OpExp {
            std::unique_ptr<LAndExp> lAndExp;
            std::unique_ptr<EqExp> eqExp;
        };
        std::variant<std::unique_ptr<EqExp>, OpExp> exp;
        ASTNodeType nodeType() const override { return ASTNode::LANDEXP; }
    };
    class LOrExp : public ASTNode {
    public:
        struct OpExp {
            std::unique_ptr<LOrExp> lOrExp;
            std::unique_ptr<LAndExp> lAndExp;
        };
        std::variant<std::unique_ptr<LAndExp>, OpExp> exp;
        ASTNodeType nodeType() const override { return ASTNode::LOREXP; }
    };
    class ConstExp : public ASTNode {
    public:
        std::unique_ptr<AddExp> addExp;
        ASTNodeType nodeType() const override { return ASTNode::CONSTEXP; }
    };
    class AbstractSyntaxTree {

    };

} // thm

#endif //ABSTRACTSYNTAXTREE_H
