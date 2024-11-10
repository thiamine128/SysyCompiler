//
// Created by slty5 on 24-11-7.
//

#ifndef MIPSBUILDER_H
#define MIPSBUILDER_H
#include "MIPS.h"
#include "../llvm/LLVM.h"

namespace thm {

class MIPSBuilder {
public:
    Module *module;
    std::ostream& os;
    std::vector<MIPSText*> texts;

    MIPSBuilder(Module *module, std::ostream& os) : module(module), os(os) {

    }

    void build();
    void addGlobalVariable(GlobalVariable *global);
    void addStringLiteral(StringLiteral *str);
    void translateFunction(Function *function);
    void translateBlock(BasicBlock *block);
    void translateBinaryInst(Function *function, BinaryInst *binaryInst);
    void translateCallInst(Function *function, CallInst *callInst);

    void submitText(MIPSText* text);
    void loadSlot(Function *function, int slot, Register reg);
};

} // thm

#endif //MIPSBUILDER_H
