//
// Created by slty5 on 24-11-7.
//

#ifndef MIPSBUILDER_H
#define MIPSBUILDER_H
#include "MIPS.h"
#include "StackTracker.h"
#include "../llvm/LLVM.h"

namespace thm {

class MIPSBuilder {
public:
    Module *module;
    std::ostream& os;
    std::vector<MIPSText*> texts;
    StackTracker stackTracker;

    MIPSBuilder(Module *module, std::ostream& os) : module(module), os(os) {

    }

    void build();
    void addGlobalVariable(GlobalVariable *global);
    void addStringLiteral(StringLiteral *str);
    void translateFunction(Function *function);
    void translateBlock(BasicBlock *block);
    void translateBinaryInst(BinaryInst *binaryInst);

    void submitText(MIPSText* text);
    void loadSlot(int slot, Register reg);
};

} // thm

#endif //MIPSBUILDER_H
