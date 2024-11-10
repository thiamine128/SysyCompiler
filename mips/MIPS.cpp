//
// Created by slty5 on 24-11-7.
//

#include "MIPS.h"

namespace thm {
    MIPSLabel::MIPSLabel(std::string const &label) {
        this->label = label;
    }

    void MIPSLabel::print(std::ostream &os) {
        os << label << ":" << std::endl;
    }

    MIPSInst::MIPSInst(Type type, Register rs, Register rt, Register rd, int imm) : type(type), rs(rs), rt(rt), rd(rd), imm(imm) {
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

    MIPSInst * MIPSInst::SaveWord(Register dest, int offset, Register base) {
        return new MIPSInst(Type::SW, base, dest, Register::ZERO, offset);
    }

    MIPSInst * MIPSInst::SaveByte(Register dest, int offset, Register base) {
        return new MIPSInst(Type::SB, base, dest, Register::ZERO, offset);
    }

    MIPSInst * MIPSInst::Syscall() {
        return new MIPSInst(SYSCALL, Register::ZERO, Register::ZERO, Register::ZERO, 0);
    }

    void MIPSInst::print(std::ostream &os) {
        os << "\t";
        switch (type) {
            case ADD:
                os << "add $" << static_cast<int>(rs) << ", $" << static_cast<int>(rt) << ", $" << static_cast<int>(rd);
                break;
            case ADDI:
                os << "addi $" << static_cast<int>(rs) << ", $" << static_cast<int>(rt) << ", " << imm;
            break;
            case SUB:
                os << "sub $" << static_cast<int>(rs) << ", $" << static_cast<int>(rt) << ", $" << static_cast<int>(rd);
            break;
            case SUBI:
                os << "subi $" << static_cast<int>(rs) << ", $" << static_cast<int>(rt) << ", " << imm;
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
                os << "seq $" << static_cast<int>(rs) << ", $" << static_cast<int>(rd) << ", $" << static_cast<int>(rd);
            break;
            case SEQI:
                os << "seq $" << static_cast<int>(rs) << ", $" << static_cast<int>(rt) << ", " << imm;
            break;
            case SNE:
                os << "sne $" << static_cast<int>(rs) << ", $" << static_cast<int>(rd) << ", $" << static_cast<int>(rd);
            break;
            case SNEI:
                os << "sne $" << static_cast<int>(rs) << ", $" << static_cast<int>(rt) << ", " << imm;
            break;
            case LW:
                os << "lw $" << static_cast<int>(rt) << " " << imm << "(" << static_cast<int>(rs) << ")" << std::endl;
                break;
            case LB:
                os << "lb $" << static_cast<int>(rt) << " " << imm << "(" << static_cast<int>(rs) << ")" << std::endl;
            break;
            case LI:
                os << "li $" << static_cast<int>(rt) << " " << imm << std::endl;
                break;
            case SW:
                os << "sw $" << static_cast<int>(rt) << " " << imm << "(" << static_cast<int>(rs) << ")" << std::endl;
            break;
            case SB:
                os << "sb $" << static_cast<int>(rt) << " " << imm << "(" << static_cast<int>(rs) << ")" << std::endl;
            break;
            default:
                break;
        }
        os << std::endl;
    }
} // thm