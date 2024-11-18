//
// Created by slty5 on 24-11-7.
//

#ifndef MIPS_H
#define MIPS_H
#include <ostream>
#include <unordered_set>
#include <vector>

namespace thm {

enum class Register {
    NONE = -1,
    ZERO = 0,
    AT,
    V0,
    V1,
    A0,
    A1,
    A2,
    A3,
    T0,
    T1,
    T2,
    T3,
    T4,
    T5,
    T6,
    T7,
    S0,
    S1,
    S2,
    S3,
    S4,
    S5,
    S6,
    S7,
    T8,
    T9,
    K0,
    K1,
    GP,
    SP,
    FP,
    RA
};
    extern Register regParams[4];
    extern std::unordered_set<Register> generalRegs;
class MIPSText {
public:
    virtual void print(std::ostream& os) {}
};
class MIPSLabel : public MIPSText {
public:
    std::string label;

    MIPSLabel(std::string const& label);
    void print(std::ostream &os) override;
};
class MIPSInst : public MIPSText {
public:
    enum Type {
        ADD,
        ADDI,
        SUB,
        SUBI,
        MUL,
        MULI,
        DIV,
        DIVI,
        REM,
        REMI,
        ANDI,
        NEG,
        NOT,
        SW,
        LW,
        SB,
        LB,
        LI,
        LA,
        AND,
        OR,
        BEQ,
        BNE,
        BLE,
        BLT,
        BGE,
        BGT,
        SLT,
        SLTI,
        SLE,
        SLEI,
        SEQ,
        SEQI,
        SGT,
        SGTI,
        SGE,
        SGEI,
        SNE,
        SNEI,
        J,
        JR,
        JAL,
        SYSCALL,
    } type;
    Register rs, rt, rd;
    int imm;
    std::string label;

    MIPSInst(Type type, Register rs, Register rt, Register rd, int imm);
    MIPSInst(Type type, Register rs, Register rt, Register rd, int imm, std::string const& label);
    static MIPSInst *Add(Register dest, Register l, Register r);
    static MIPSInst *AddImm(Register dest, Register l, int r);
    static MIPSInst *Sub(Register dest, Register l, Register r);
    static MIPSInst *SubImm(Register dest, Register l, int r);
    static MIPSInst *Mul(Register dest, Register l, Register r);
    static MIPSInst *MulImm(Register dest, Register l, int r);
    static MIPSInst *Div(Register dest, Register l, Register r);
    static MIPSInst *DivImm(Register dest, Register l, int r);
    static MIPSInst *Rem(Register dest, Register l, Register r);
    static MIPSInst *RemImm(Register dest, Register l, int r);
    static MIPSInst *AndImm(Register dest, Register l, int r);
    static MIPSInst *Eq(Register dest, Register l, Register r);
    static MIPSInst *EqImm(Register dest, Register l, int r);
    static MIPSInst *Neq(Register dest, Register l, Register r);
    static MIPSInst *NeqImm(Register dest, Register l, int r);
    static MIPSInst *Sle(Register dest, Register l, Register r);
    static MIPSInst *SleImm(Register dest, Register l, int r);
    static MIPSInst *Slt(Register dest, Register l, Register r);
    static MIPSInst *SltImm(Register dest, Register l, int r);
    static MIPSInst *Sge(Register dest, Register l, Register r);
    static MIPSInst *SgeImm(Register dest, Register l, int r);
    static MIPSInst *Sgt(Register dest, Register l, Register r);
    static MIPSInst *SgtImm(Register dest, Register l, int r);
    static MIPSInst *LoadWord(Register dest, int offset, Register base);
    static MIPSInst *LoadByte(Register dest, int offset, Register base);
    static MIPSInst *LoadImm(Register dest, int imm);
    static MIPSInst *LoadAddr(Register dest, std::string const& symbol);
    static MIPSInst *SaveWord(Register val, int offset, Register base);
    static MIPSInst *SaveByte(Register val, int offset, Register base);
    static MIPSInst *Syscall();
    static MIPSInst *JumpAndLink(std::string const& name);
    static MIPSInst *JumpReg(Register reg);
    static MIPSInst *Jump(std::string const& label);
    static MIPSInst *BranchNE(Register cond, Register target, std::string const& label);

    void print(std::ostream &os) override;
};

} // thm

#endif //MIPS_H
