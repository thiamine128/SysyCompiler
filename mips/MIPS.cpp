//
// Created by slty5 on 24-11-7.
//

#include "MIPS.h"

#include <unordered_set>


namespace thm {
    Register regParams[4] = {Register::A0, Register::A1, Register::A2, Register::A3};
    std::unordered_set<Register> generalRegs = {Register::T0, Register::T1, Register::T2, Register::T3, Register::T4, Register::T5, Register::T6, Register::T7, Register::S0, Register::S1, Register::S2, Register::S3, Register::S4, Register::S5, Register::S6, Register::S7, Register::T8, Register::T9, Register::K0, Register::K1};
    MIPSLabel::MIPSLabel(std::string const &label) {
        this->label = label;
    }

    void MIPSLabel::print(std::ostream &os) {
        os << label << ":" << std::endl;
    }

    MIPSInst::MIPSInst(Type type, Register rs, Register rt, Register rd, int imm) : type(type), rs(rs), rt(rt), rd(rd), imm(imm) {
    }

    MIPSInst::MIPSInst(Type type, Register rs, Register rt, Register rd, int imm, std::string const &label) : type(type), rs(rs), rt(rt), rd(rd), imm(imm), label(label) {

    }

    MIPSInst * MIPSInst::Add(Register dest, Register l, Register r) {
        return new MIPSInst(ADD, dest, l, r, 0);
    }

    MIPSInst * MIPSInst::AddImm(Register dest, Register l, int r) {
        return new MIPSInst(ADDI, dest, l, Register::ZERO, r);
    }

    MIPSInst * MIPSInst::Sub(Register dest, Register l, Register r) {
        return new MIPSInst(SUB, dest, l, r, 0);
    }

    MIPSInst * MIPSInst::SubImm(Register dest, Register l, int r) {
        return new MIPSInst(SUBI, dest, l, Register::ZERO, r);
    }

    MIPSInst * MIPSInst::Mul(Register dest, Register l, Register r) {
        return new MIPSInst(MUL, dest, l, r, 0);
    }

    MIPSInst * MIPSInst::MulImm(Register dest, Register l, int r) {
        return new MIPSInst(MULI, dest, l, Register::ZERO, r);
    }

    MIPSInst * MIPSInst::Div(Register dest, Register l, Register r) {
        return new MIPSInst(DIV, dest, l, r, 0);
    }

    MIPSInst * MIPSInst::DivImm(Register dest, Register l, int r) {
        return new MIPSInst(DIVI, dest, l, Register::ZERO, r);
    }

    MIPSInst * MIPSInst::Rem(Register dest, Register l, Register r) {
        return new MIPSInst(REM, dest, l, r, 0);
    }

    MIPSInst * MIPSInst::RemImm(Register dest, Register l, int r) {
        return new MIPSInst(REMI, dest, l, Register::ZERO, r);
    }

    MIPSInst * MIPSInst::AndImm(Register dest, Register l, int r) {
        return new MIPSInst(ANDI, dest, l, Register::ZERO, r);
    }

    MIPSInst * MIPSInst::Eq(Register dest, Register l, Register r) {
        return new MIPSInst(SEQ, dest, l, r, 0);
    }

    MIPSInst * MIPSInst::EqImm(Register dest, Register l, int r) {
        return new MIPSInst(SEQI, dest, l, Register::ZERO, r);
    }

    MIPSInst * MIPSInst::Neq(Register dest, Register l, Register r) {
        return new MIPSInst(SNE, dest, l, r, 0);
    }

    MIPSInst * MIPSInst::NeqImm(Register dest, Register l, int r) {
        return new MIPSInst(SNEI, dest, l, Register::ZERO, r);
    }

    MIPSInst * MIPSInst::Sle(Register dest, Register l, Register r) {
        return new MIPSInst(SLE, dest, l, r, 0);
    }

    MIPSInst * MIPSInst::SleImm(Register dest, Register l, int r) {
        return new MIPSInst(SLEI, dest, l, Register::ZERO, r);
    }

    MIPSInst * MIPSInst::Slt(Register dest, Register l, Register r) {
        return new MIPSInst(SLT, dest, l, r, 0);
    }

    MIPSInst * MIPSInst::SltImm(Register dest, Register l, int r) {
        return new MIPSInst(SLTI, dest, l, Register::ZERO, r);
    }

    MIPSInst * MIPSInst::Sge(Register dest, Register l, Register r) {
        return new MIPSInst(SGE, dest, l, r, 0);
    }

    MIPSInst * MIPSInst::SgeImm(Register dest, Register l, int r) {
        return new MIPSInst(SGEI, dest, l, Register::ZERO, r);
    }

    MIPSInst * MIPSInst::Sgt(Register dest, Register l, Register r) {
        return new MIPSInst(SGT, dest, l, r, 0);
    }

    MIPSInst * MIPSInst::SgtImm(Register dest, Register l, int r) {
        return new MIPSInst(SGTI, dest, l, Register::ZERO, r);
    }

    MIPSInst * MIPSInst::LoadWord(Register dest, int offset, Register base) {
        return new MIPSInst(Type::LW, base, dest, Register::ZERO, offset);
    }

    MIPSInst * MIPSInst::LoadByte(Register dest, int offset, Register base) {
        return new MIPSInst(Type::LB, base, dest, Register::ZERO, offset);
    }

    MIPSInst * MIPSInst::LoadImm(Register dest, int imm) {
        return new MIPSInst(LI, Register::ZERO, dest, Register::ZERO, imm);
    }

    MIPSInst * MIPSInst::LoadAddr(Register dest, std::string const &symbol) {
        return new MIPSInst(LA, Register::ZERO, dest, Register::ZERO, 0, symbol);
    }

    MIPSInst * MIPSInst::SaveWord(Register val, int offset, Register base) {
        return new MIPSInst(Type::SW, base, val, Register::ZERO, offset);
    }

    MIPSInst * MIPSInst::SaveByte(Register val, int offset, Register base) {
        return new MIPSInst(Type::SB, base, val, Register::ZERO, offset);
    }

    MIPSInst * MIPSInst::Syscall() {
        return new MIPSInst(SYSCALL, Register::ZERO, Register::ZERO, Register::ZERO, 0);
    }

    MIPSInst * MIPSInst::JumpAndLink(std::string const &name) {
        return new MIPSInst(JAL, Register::ZERO, Register::ZERO, Register::ZERO, 0, name);
    }

    MIPSInst * MIPSInst::JumpReg(Register reg) {
        return new MIPSInst(JR, reg, Register::ZERO, Register::ZERO, 0);
    }

    MIPSInst * MIPSInst::Jump(std::string const &label) {
        return new MIPSInst(J, Register::ZERO, Register::ZERO, Register::ZERO, 0, label);
    }

    MIPSInst * MIPSInst::BranchNE(Register cond, Register target, std::string const &label) {
        return new MIPSInst(BNE, cond, target, Register::ZERO, 0, label);
    }

    void MIPSInst::print(std::ostream &os) {
        os << "\t";
        switch (type) {
            case ADD:
                os << "addu $" << static_cast<int>(rs) << ", $" << static_cast<int>(rt) << ", $" << static_cast<int>(rd);
                break;
            case ADDI:
                os << "addiu $" << static_cast<int>(rs) << ", $" << static_cast<int>(rt) << ", " << imm;
            break;
            case SUB:
                os << "subu $" << static_cast<int>(rs) << ", $" << static_cast<int>(rt) << ", $" << static_cast<int>(rd);
            break;
            case SUBI:
                os << "subiu $" << static_cast<int>(rs) << ", $" << static_cast<int>(rt) << ", " << imm;
            break;
            case MUL:
                os << "mul $" << static_cast<int>(rs) << ", $" << static_cast<int>(rt) << ", $" << static_cast<int>(rd);
            break;
            case MULI:
                os << "mul $" << static_cast<int>(rs) << ", $" << static_cast<int>(rt) << ", " << imm;
            break;
            case DIV:
                os << "div $" << static_cast<int>(rs) << ", $" << static_cast<int>(rt) << ", $" << static_cast<int>(rd);
            break;
            case DIVI:
                os << "div $" << static_cast<int>(rs) << ", $" << static_cast<int>(rt) << ", " << imm;
            break;
            case REM:
                os << "rem $" << static_cast<int>(rs) << ", $" << static_cast<int>(rt) << ", $" << static_cast<int>(rd);
            break;
            case REMI:
                os << "rem $" << static_cast<int>(rs) << ", $" << static_cast<int>(rt) << ", " << imm;
            break;
            case SEQ:
                os << "seq $" << static_cast<int>(rs) << ", $" << static_cast<int>(rt) << ", $" << static_cast<int>(rd);
            break;
            case SEQI:
                os << "seq $" << static_cast<int>(rs) << ", $" << static_cast<int>(rt) << ", " << imm;
            break;
            case SNE:
                os << "sne $" << static_cast<int>(rs) << ", $" << static_cast<int>(rt) << ", $" << static_cast<int>(rd);
            break;
            case SNEI:
                os << "sne $" << static_cast<int>(rs) << ", $" << static_cast<int>(rt) << ", " << imm;
            break;
            case SLT:
                os << "slt $" << static_cast<int>(rs) << ", $" << static_cast<int>(rt) << ", $" << static_cast<int>(rd);
            break;
            case SLTI:
                os << "slti $" << static_cast<int>(rs) << ", $" << static_cast<int>(rt) << ", " << imm;
            break;
            case SGT:
                os << "sgt $" << static_cast<int>(rs) << ", $" << static_cast<int>(rt) << ", $" << static_cast<int>(rd);
            break;
            case SGTI:
                os << "sgt $" << static_cast<int>(rs) << ", $" << static_cast<int>(rt) << ", " << imm;
            break;
            case SLE:
                os << "sle $" << static_cast<int>(rs) << ", $" << static_cast<int>(rt) << ", $" << static_cast<int>(rd);
            break;
            case SLEI:
                os << "sle $" << static_cast<int>(rs) << ", $" << static_cast<int>(rt) << ", " << imm;
            break;
            case SGE:
                os << "sge $" << static_cast<int>(rs) << ", $" << static_cast<int>(rt) << ", $" << static_cast<int>(rd);
            break;
            case SGEI:
                os << "sge $" << static_cast<int>(rs) << ", $" << static_cast<int>(rt) << ", " << imm;
            break;
            case LW:
                os << "lw $" << static_cast<int>(rt) << ", " << imm << "($" << static_cast<int>(rs) << ")";
                break;
            case LB:
                os << "lbu $" << static_cast<int>(rt) << ", " << imm << "($" << static_cast<int>(rs) << ")";
            break;
            case LI:
                os << "li $" << static_cast<int>(rt) << ", " << imm;
                break;
            case SW:
                os << "sw $" << static_cast<int>(rt) << ", " << imm << "($" << static_cast<int>(rs) << ")";
            break;
            case SB:
                os << "sb $" << static_cast<int>(rt) << ", " << imm << "($" << static_cast<int>(rs) << ")";
            break;
            case SYSCALL:
                os << "syscall";
            break;
            case LA:
                os << "la $" << static_cast<int>(rt) << " " << label;
            break;
            case JAL:
                os << "jal " << label;
            break;
            case JR:
                os << "jr $" << static_cast<int>(rs);
            break;
            case BNE:
                os << "bne $" << static_cast<int>(rs) << ", $" << static_cast<int>(rt) << ", " << label;
            break;
            case J:
                os << "j " << label;
            break;
            case ANDI:
                os << "and $" << static_cast<int>(rs) << ", $" << static_cast<int>(rt) << ", " << imm;
            break;
            default:
                break;
        }
        os << std::endl;
    }
} // thm