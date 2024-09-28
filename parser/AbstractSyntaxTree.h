//
// Created by slty5 on 24-9-27.
//

#ifndef ABSTRACTSYNTAXTREE_H
#define ABSTRACTSYNTAXTREE_H
#include <memory>
#include <variant>
#include <vector>

namespace thm {
    struct Token;
    class LAndExp;
    class RelExp;
    class MulExp;
    class UnaryExp;
    class PrimaryExp;
    class CompUnit;
    class FuncFParams;
    class EqExp;
    class FuncRParam;
    class UnaryOp;
    class FuncRParams;
    class Character;
    class Number;
    class LOrExp;
    class AddExp;
    class ForStmt;
    class Cond;
    class LVal;
    class Stmt;
    class BlockItem;
    class FuncFParam;
    class Block;
    class FuncType;
    class Exp;
    class InitVal;
    class VarDef;
    class ConstExp;
    class ConstInitVal;
    class ConstDef;
    class BType;
    class VarDecl;
    class ConstDecl;
    class MainFuncDef;
    class FuncDef;
    class Decl;

    std::ostream& operator<<(std::ostream& os, const CompUnit& a);
    std::ostream& operator<<(std::ostream& os, const Decl& a);
    std::ostream& operator<<(std::ostream& os, const ConstDecl& a);
    std::ostream& operator<<(std::ostream& os, const BType& a);
    std::ostream& operator<<(std::ostream& os, const ConstDef& a);
    std::ostream& operator<<(std::ostream& os, const ConstInitVal& a);
    std::ostream& operator<<(std::ostream& os, const VarDecl& a);
    std::ostream& operator<<(std::ostream& os, const VarDef& a);
    std::ostream& operator<<(std::ostream& os, const InitVal& a);
    std::ostream& operator<<(std::ostream& os, const FuncDef& a);
    std::ostream& operator<<(std::ostream& os, const MainFuncDef& a);
    std::ostream& operator<<(std::ostream& os, const FuncType& a);
    std::ostream& operator<<(std::ostream& os, const FuncFParams& a);
    std::ostream& operator<<(std::ostream& os, const FuncFParam& a);
    std::ostream& operator<<(std::ostream& os, const Block& a);
    std::ostream& operator<<(std::ostream& os, const BlockItem& a);
    std::ostream& operator<<(std::ostream& os, const Stmt& a);
    std::ostream& operator<<(std::ostream& os, const ForStmt& a);
    std::ostream& operator<<(std::ostream& os, const Cond& a);
    std::ostream& operator<<(std::ostream& os, const LVal& a);
    std::ostream& operator<<(std::ostream& os, const Exp& a);
    std::ostream& operator<<(std::ostream& os, const PrimaryExp& a);
    std::ostream& operator<<(std::ostream& os, const Number& a);
    std::ostream& operator<<(std::ostream& os, const Character& a);
    std::ostream& operator<<(std::ostream& os, const UnaryExp& a);
    std::ostream& operator<<(std::ostream& os, const UnaryOp& a);
    std::ostream& operator<<(std::ostream& os, const FuncRParams& a);
    std::ostream& operator<<(std::ostream& os, const MulExp& a);
    std::ostream& operator<<(std::ostream& os, const AddExp& a);
    std::ostream& operator<<(std::ostream& os, const RelExp& a);
    std::ostream& operator<<(std::ostream& os, const EqExp& a);
    std::ostream& operator<<(std::ostream& os, const LAndExp& a);
    std::ostream& operator<<(std::ostream& os, const LOrExp& a);
    std::ostream& operator<<(std::ostream& os, const ConstExp& a);

    class ASTNode {
    public:
        int lineno;
        std::vector<Token> tokens;

        void consume(std::vector<Token>& tokens);
        virtual ~ASTNode() = default;
    };
    class CompUnit : public ASTNode {
    public:
        std::vector<std::unique_ptr<Decl>> decls;
        std::vector<std::unique_ptr<FuncDef>> funcDefs;
        std::unique_ptr<MainFuncDef> mainFuncDef;
    };
    class Decl : public ASTNode {
    public:
        std::variant<std::unique_ptr<ConstDecl>, std::unique_ptr<VarDecl>> decl;
    };
    class ConstDecl : public ASTNode {
    public:
        std::unique_ptr<BType> bType;
        std::vector<std::unique_ptr<ConstDef>> constDefs;
    };
    class BType : public ASTNode {
    public:
        enum Type {
            INT,
            CHAR
        } type;
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
    };
    class VarDecl : public ASTNode {
    public:
        std::unique_ptr<BType> bType;
        std::vector<std::unique_ptr<VarDef>> varDefs;
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
    };
    class FuncDef : public ASTNode {
    public:
        std::unique_ptr<FuncType> funcType;
        std::string ident;
        std::unique_ptr<FuncFParams> params;
        std::unique_ptr<Block> block;
    };
    class MainFuncDef : public ASTNode {
    public:
        std::unique_ptr<Block> block;
    };
    class FuncType : public ASTNode {
    public:
        enum Type {
            VOID, INT, CHAR
        } type;
    };
    class FuncFParams : public ASTNode {
    public:
        std::vector<std::unique_ptr<FuncFParam>> params;
    };
    class FuncFParam : public ASTNode {
    public:
        std::unique_ptr<BType> bType;
        std::string ident;
        bool isArray;
    };
    class Block : public ASTNode {
    public:
        std::vector<std::unique_ptr<BlockItem>> items;
    };
    class BlockItem : public ASTNode {
    public:
        std::variant<std::unique_ptr<Decl>, std::unique_ptr<Stmt>> item;
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
    };
    class ForStmt : public ASTNode {
    public:
        std::unique_ptr<LVal> lVal;
        std::unique_ptr<Exp> exp;
    };
    class Exp : public ASTNode {
    public:
        std::unique_ptr<AddExp> addExp;
    };
    class Cond : public ASTNode {
    public:
        std::unique_ptr<LOrExp> lOrExp;
    };
    class LVal : public ASTNode {
    public:
        std::string ident;
        std::unique_ptr<Exp> exp;
    };
    class PrimaryExp : public ASTNode {
    public:
        std::variant<std::unique_ptr<Exp>, std::unique_ptr<LVal>, std::unique_ptr<Number>, std::unique_ptr<Character>> primaryExp;
    };
    class Number : public ASTNode {
    public:
        int num;
    };
    class Character : public ASTNode {
    public:
        char ch;
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
    };
    class UnaryOp : public ASTNode {
    public:
        enum Type {
            PLUS, MINUS, NOT
        } type;
    };
    class FuncRParams : public ASTNode {
    public:
        std::vector<std::unique_ptr<Exp>> params;
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
    };
    class LAndExp : public ASTNode {
    public:
        struct OpExp {
            std::unique_ptr<LAndExp> lAndExp;
            std::unique_ptr<EqExp> eqExp;
        };
        std::variant<std::unique_ptr<EqExp>, OpExp> exp;
    };
    class LOrExp : public ASTNode {
    public:
        struct OpExp {
            std::unique_ptr<LOrExp> lOrExp;
            std::unique_ptr<LAndExp> lAndExp;
        };
        std::variant<std::unique_ptr<LAndExp>, OpExp> exp;
    };
    class ConstExp : public ASTNode {
    public:
        std::unique_ptr<AddExp> addExp;
    };
    class AbstractSyntaxTree {

    };

} // thm

#endif //ABSTRACTSYNTAXTREE_H
