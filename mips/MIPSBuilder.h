//
// Created by slty5 on 24-11-7.
//

#ifndef MIPSBUILDER_H
#define MIPSBUILDER_H
#include "MIPS.h"
#include "../llvm/LLVM.h"
#include "../pass/Pass.h"
namespace thm {

class MIPSBuilder : public Pass {
public:
    std::ostream& os;
    std::vector<MIPSText*> texts;

    MIPSBuilder(Module *module, std::ostream& os) : Pass(module), os(os) {

    }

    void process() override;
    void addGlobalVariable(GlobalVariable *global);
    void addStringLiteral(StringLiteral *str);
    void processFunction(Function *function);

    void translateBackupInst(Function * function, BackupArg * inst);

    void translateRecoverInst(Function * function, RecoverArg * inst);

    void translateBlock(BasicBlock *block);
    void translateBinaryInst(Function *function, BinaryInst *binaryInst);
    void translateCallInst(Function *function, CallInst *callInst);
    void translateLoadInst(Function *function, LoadInst *loadInst);
    void translateStoreInst(Function *function, StoreInst *storeInst);
    void translateGetElementPtrInst(Function *function, GetElementPtr *getElementPtr);
    void translateBranchInst(Function *function, BranchInst *branchInst);
    void translateRetInst(Function *function, RetInst *retInst);
    void translateZextInst(Function *function, ZextInst *zextInst);
    void translateTruncInst(Function *function, TruncInst *truncInst);
    void translateMoveInst(Function *function, MoveInst *moveInst);

    void submitText(MIPSText* text);
    Register loadValue(Value *value);
    void loadValue(Value *value, Register reg);
    void loadConstant(Value *value, Register reg);
    void move(Register dst, Register src);
    void sequenceMoves(std::vector<std::pair<Register, Register>> &moves);

    void debugReturn();

    void debugBreak();

    void debugTLE();

    void alignCheck(Register reg, int offset);
};

} // thm

#endif //MIPSBUILDER_H
