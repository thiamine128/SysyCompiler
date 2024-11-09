//
// Created by slty5 on 24-11-7.
//

#ifndef MIPS_H
#define MIPS_H
#include <ostream>

namespace thm {

enum class Register {
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
        SLE,
        SEQ,
        SGT,
        SGE,
        SNE,
        JR,
        JAL
    } type;
    Register rs, rt, rd;
    int imm;

    MIPSInst(Type type, Register rs, Register rt, Register rd, int imm);
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
    static MIPSInst *LoadWord(Register dest, int offset, Register base);
    static MIPSInst *LoadImm(Register dest, int imm);

    void print(std::ostream &os) override;
};

} // thm

#endif //MIPS_H
