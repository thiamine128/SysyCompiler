//
// Created by slty5 on 24-10-27.
//

#include "LLVM.h"

#include <complex>
#include <functional>
#include <iostream>
#include <ostream>
#include <algorithm>

#include "../pass/AllocateRegisters.h"
#include "../pass/DeadCode.h"
#include "../pass/EliminatePhis.h"
#include "../pass/GCM.h"
#include "../pass/Mem2Reg.h"
#include "../pass/SaveArgument.h"
#include "../pass/AnalyzeSideEffect.h"
#include "../mips/Frame.h"
#include "../pass/AnalyzeDominance.h"

namespace thm {
    ValueType::Type ValueType::type() const {
        return BASIC;
    }

    void ValueType::print(std::ostream &os) const {

    }

    ValueType* ValueType::clone() const {
        return new ValueType();
    }

    ValueType * ValueType::fromVariableType(VariableType &variableType) {
        ValueType* t = new BasicValueType(variableType.type == VariableType::INT ? BasicValueType::I32 : BasicValueType::I8);
        if (variableType.isArray) {
            t = new ArrayValueType(t, variableType.arrayLen);
        }
        return t;
    }

    ValueType::Type BasicValueType::type() const {
        return BASIC;
    }

    void BasicValueType::print(std::ostream &os) const {
        switch (basicType) {
            case I32: os << "i32"; break;
            case I8: os << "i8"; break;
            case VOID: os << "void"; break;
            case I1: os << "i1"; break;
            case LABEL: os << "label"; break;
        }
    }

    ValueType * BasicValueType::clone() const {
        return new BasicValueType(basicType);
    }

    BasicValueType::BasicType BasicValueType::fromFunctionType(FunctionSymbol::Type functionType) {
        switch (functionType) {
            case FunctionSymbol::INT:
                return BasicValueType::I32;
            break;
            case FunctionSymbol::CHAR:
                return BasicValueType::I8;
            break;
            case FunctionSymbol::VOID:
                return BasicValueType::VOID;
            break;
        }
        return VOID;
    }

    ValueType::Type PtrValueType::type() const {
        return PTR;
    }

    void PtrValueType::print(std::ostream &os) const {
        value->print(os);
        os << "*";
    }

    ValueType * PtrValueType::clone() const {
        return new PtrValueType(value->clone());
    }

    ValueType::Type ArrayValueType::type() const {
        return ARRAY;
    }

    void ArrayValueType::print(std::ostream &os) const {
        os << "[" << arrayLen << " x ";
        value->print(os);
        os << "]";
    }

    ValueType * ArrayValueType::clone() const {
        return new ArrayValueType(value->clone(), arrayLen);
    }

    Value::~Value() {
        delete valueType;
    }

    LLVMType Value::type() const {
        return LLVMType::DEFAULT;
    }

    void Value::print(std::ostream &os) const {

    }

    void Value::printRef(std::ostream &os) const {
        os << "%" << slot;
    }

    bool Value::needsColor() const {
        return false;
    }

    Argument::Argument() {
        slot = 0;
    }

    LLVMType Argument::type() const {
        return LLVMType::ARGUMENT;
    }

    void Argument::print(std::ostream &os) const {
        valueType->print(os);
        os << " %" << slot;
    }

    bool Argument::needsColor() const {
        return true;
    }

    BasicBlock::BasicBlock(Function *function, int loopNest) : function(function), loopNest(loopNest) {
        valueType = new BasicValueType(BasicValueType::LABEL);
        slot = 0;
    }

    LLVMType BasicBlock::type() const {
        return LLVMType::BASIC_BLOCK;
    }

    void BasicBlock::print(std::ostream &os) const {
        os << slot << ":" << std::endl;
        // os << "; " << domDepth << std::endl;
        // if (iDom)
        //     os << "; " << iDom->slot << std::endl;
        // else
        //     os << ";" << std::endl;
        // os << "; ";
        // for (auto bb : doms) {
        //     os << bb->slot << " ";
        // }
        // os << std::endl;
        // os << "; froms: ";
        // for (auto bb : froms) {
        //     os << bb->slot << " ";
        // }
        // os << std::endl;
        for (Instruction* instruction : insts) {
            os << "\t";
            instruction->print(os);
        }
    }

    void BasicBlock::printRef(std::ostream &os) const {
        os << "%" << slot;
    }

    bool BasicBlock::isReachable() const {
        if (froms.size() == 0 || (froms.size() == 1 && froms[0] == this)) return false;
        return true;
    }

    void BasicBlock::calcDefUse() {
        def.clear();
        use.clear();
        in.clear();
        out.clear();
        for (auto inst : insts) {
            std::unordered_set<int> instDef, instUse;
            inst->getDefUse(instDef, instUse);
            for (auto elm : instUse) {
                if (def.find(elm) == def.end())
                    use.insert(elm);
            }
            for (auto elm : instDef) {
                if (use.find(elm) == use.end())
                    def.insert(elm);
            }
        }
    }

    void BasicBlock::addInstAhead(Instruction *instruction) {
        instruction->block = this;
        insts.insert(insts.begin(), instruction);
    }

    void BasicBlock::addInst(Instruction *instruction) {
        instruction->block = this;
        insts.push_back(instruction);
    }

    void BasicBlock::setupTransfer() {
        int len = 0;
        for (int i = 0; i < insts.size(); ++i) {
            len++;
            if (insts[i]->type() == LLVMType::RET_INST) {
                break;
            }
            if (insts[i]->type() == LLVMType::BRANCH_INST) {
                BranchInst *branchInst = static_cast<BranchInst *>(insts[i]);
                if (branchInst->ifTrue) {
                    tos.push_back(branchInst->ifTrue);
                    branchInst->ifTrue->froms.push_back(this);
                }
                if (branchInst->ifFalse) {
                    tos.push_back(branchInst->ifFalse);
                    branchInst->ifFalse->froms.push_back(this);
                }
                break;
            }
        }

        while (insts.size() > len) {
            insts.pop_back();
        }
    }

    bool BasicBlock::canMerge() {
        return tos.size() == 1 && tos[0]->froms.size() == 1;
    }

    void BasicBlock::merge(BasicBlock *block) {
        insts.pop_back();
        for (auto inst : block->insts) {
            addInst(inst);
        }
        tos = block->tos;
        for (auto to : tos) {
            for (int i = 0; i < to->froms.size(); ++i) {
                if (to->froms[i] == block) {
                    to->froms[i] = this;
                }
            }
        }
    }

    void BasicBlock::fillSlot() {
        for (Instruction *inst : insts) {
            if (inst->slot == 0) {
                if (MoveInst *move = dynamic_cast<MoveInst *>(inst)) {
                    if (move->dst->slot == 0) {
                        move->dst->slot = function->slotTracker.allocSlot();
                    }
                    move->slot = move->dst->slot;
                    continue;
                }
                inst->slot = function->slotTracker.allocSlot();
            }
        }
    }

    void BasicBlock::addInstLastSecond(Instruction *inst) {
        inst->block = this;
        insts.insert(insts.end() - 1, inst);
    }

    LLVMType Constant::type() const {
        return LLVMType::CONSTANT;
    }

    Undef::Undef(BasicValueType::BasicType basicType) {
        valueType = new BasicValueType(basicType);
    }

    LLVMType Undef::type() const {
        return LLVMType::UNDEF;
    }

    void Undef::print(std::ostream &os) const {
        os << "undef";
    }

    void Undef::printRef(std::ostream &os) const {
        os << "undef";
    }

    NumericLiteral::NumericLiteral(int value, BasicValueType::BasicType basic) : value(value) {
        valueType = new BasicValueType(basic);
    }

    LLVMType NumericLiteral::type() const {
        return LLVMType::NUMERIC_LITERAL;
    }

    void NumericLiteral::print(std::ostream &os) const {
        os << value;
    }

    void NumericLiteral::printRef(std::ostream &os) const {
        os << value;
    }

    StringLiteral::StringLiteral(const std::string &value) : value(value) {
        valueType = new PtrValueType(new ArrayValueType(new BasicValueType(BasicValueType::I8), value.length() + 1));
    }

    LLVMType StringLiteral::type() const {
        return LLVMType::STRING_LITERAL;
    }

    void StringLiteral::print(std::ostream &os) const {
        os << "@.str";
        if (refId) {
            os << "." << refId;
        }
        os << " = private unnamed_addr constant ";
        PtrValueType *ptr = static_cast<PtrValueType *>(valueType);
        ptr->value->print(os);
        os << " c\"";
        for (int i = 0; i < value.length(); i++) {
            int f = value[i] / 16;
            int s = value[i] % 16;
            os << "\\";
            if (f >= 10) {
                os << static_cast<char>('A' + f - 10);
            } else {
                os << f;
            }
            if (s >= 10) {
                os << static_cast<char>('A' + s - 10);
            } else {
                os << s;
            }
        }
        os << "\\00\", align 1" << std::endl;
    }

    void StringLiteral::printRef(std::ostream &os) const {
        os << "@.str";
        if (refId) {
            os << "." << refId;
        }
    }

    LLVMType GlobalValue::type() const {
        return LLVMType::GLOBAL_VALUE;
    }

    void GlobalValue::printRef(std::ostream &os) const {
        os << "@" << name;
    }

    Function::Function(std::string const &name, BasicValueType::BasicType retType): frame(new Frame(this)) {
        this->name = name;
        valueType = new BasicValueType(retType);
    }

    LLVMType Function::type() const {
        return LLVMType::FUNCTION;
    }

    void Function::print(std::ostream &os) const {
        os << "define dso_local ";
        valueType->print(os);
        os << " @" << name << "(";
        for (int i = 0; i < args.size(); i++) {
            args[i]->print(os);
            if (i + 1 < args.size()) os << ", ";
        }
        os << ") {" << std::endl;
        os << ";";
        for (auto arg : args) {
            os << static_cast<int>(arg->reg) << " ";
        }
        os << std::endl;
        for (BasicBlock* block : blocks) {
            block->print(os);
        }
        os << "}" << std::endl;
    }

    void Function::removeUnreachableBlocks() {
        std::unordered_map<BasicBlock *, bool> vis;
        std::vector<BasicBlock *> q;
        vis[blocks[0]] = true;
        q.push_back(blocks[0]);
        while (!q.empty()) {
            auto bb = q.front();
            q.erase(q.begin());
            for (auto to : bb->tos) {
                if (!vis[to]) {
                    vis[to] = true;
                    q.push_back(to);
                }
            }
        }
        for (auto it = blocks.begin(); it != blocks.end();) {
            if (!vis[*it]) {
                for (auto to = (*it)->tos.begin(); to != (*it)->tos.end(); ++to) {
                    for (auto from = (*to)->froms.begin(); from != (*to)->froms.end(); ) {
                        if (*from == *it) {
                            from = (*to)->froms.erase(from);
                        } else {
                            ++from;
                        }
                    }
                }
                it = blocks.erase(it);
            } else {
                ++it;
            }
        }
    }

    void Function::arrangeBlocks() {
        for (auto block : blocks) {
            block->setupTransfer();
        }
        removeUnreachableBlocks();
        std::unordered_set<BasicBlock*> merged;
        for (auto block : blocks) {
            while (block->canMerge() && merged.find(block) == merged.end()) {
                merged.insert(block->tos[0]);
                block->merge(block->tos[0]);
            }
        }
        for (auto iter = blocks.begin(); iter != blocks.end(); ) {
            if (merged.find(*iter) == merged.end()) {
                ++iter;
            } else {
                iter = blocks.erase(iter);
            }
        }
        root = blocks[0];
    }

    void Function::setAllocas() {
        for (BasicBlock* block : blocks) {
            for (auto inst = block->insts.begin(); inst != block->insts.end();) {
                if ((*inst)->type() == LLVMType::ALLOCA_INST) {
                    allocas.push_back(static_cast<AllocaInst*>(*inst));
                    inst = block->insts.erase(inst);
                } else {
                    ++inst;
                }
            }
        }
        for (auto alloc : allocas) {
            blocks[0]->addInstAhead(alloc);
        }
    }

    void Function::livenessAnalysis() {
        for (BasicBlock* block : blocks) {
            block->calcDefUse();
        }
        bool changed = true;
        while (changed) {
            changed = false;
            for (BasicBlock* block : blocks) {
                std::unordered_set<int> newIn;
                std::unordered_set<int> newOut;

                for (BasicBlock* to : block->tos) {
                    newOut.insert(to->in.begin(), to->in.end());
                }
                newIn = block->use;
                for (auto elm : newOut) {
                    if (block->def.find(elm) == block->def.end()) {
                        newIn.insert(elm);
                    }
                }
                if (newIn != block->in || newOut != block->out) {
                    changed = true;
                    block->out = newOut;
                    block->in = newIn;
                }
            }
        }
    }

    void Function::fillSlot() {
        int argIdx = 0;
        for (Argument* arg : args) {
            arg->slot = slotTracker.allocSlot();
            if (argIdx < 4) {
                arg->reg = regParams[argIdx];
            }
            argIdx++;
        }
        int idx = 0;
        for (BasicBlock* block : blocks) {
            block->slot = slotTracker.allocSlot();
            block->blockIdx = idx;
            idx++;
            block->fillSlot();
        }
    }

    int Function::getMaxArgs() {
        int num = 0;
        for (auto block : blocks) {
            for (auto inst : block->insts) {
                if (inst->type() == LLVMType::CALL_INST) {
                    CallInst *call = static_cast<CallInst *>(inst);
                    if (call->args.size() > num) {
                        num = call->args.size();
                    }
                }
            }
        }
        return num;
    }

    void Function::rebuildCFG() {
        for (auto bb : blocks) {
            bb->froms.clear();
            bb->tos.clear();
        }
        for (auto bb : blocks) {
            bb->setupTransfer();
        }
    }

    LLVMType GlobalVariable::type() const {
        return LLVMType::GLOBAL_VARIABLE;
    }

    void GlobalVariable::print(std::ostream &os) const {
        PtrValueType* ptr = static_cast<PtrValueType*>(valueType);
        os << "@" << name << " = " << "dso_local" << " " << "global ";
        ptr->value->print(os);
        os << " ";
        if (ptr->value->type() == ValueType::ARRAY) {
            ArrayValueType* arrayType = static_cast<ArrayValueType*>(ptr->value);
            if (zeroInit) {
                os << "zeroinitializer";
            } else {
                os << "[";
                for (int i = 0; i < arrayType->arrayLen; i++) {
                    arrayType->value->print(os);
                    os << " " << initVals[i];
                    if (i < arrayType->arrayLen - 1) {
                        os << ", ";
                    }
                }
                os << "]";
            }
            if (arrayType->value->type() == ValueType::BASIC) {
                BasicValueType* basicType = static_cast<BasicValueType*>(arrayType->value);
                if (basicType->basicType == BasicValueType::I8) {
                    os << ", align 1";
                }
            }
        } else {
            os << initVal;
        }
        os << std::endl;
    }

    LLVMType User::type() const {
        return LLVMType::USER;
    }

    void User::use(Value **value) {
        (*value)->usedBys.push_back(this);
        usings.push_back(value);
    }

    LLVMType Instruction::type() const {
        return LLVMType::INST;
    }

    void Instruction::getDefUse(std::unordered_set<int> &def, std::unordered_set<int> &use) {
    }

    void Instruction::onRemove() {
        for (auto use : usings) {
            (*use)->usedBys.erase(std::find((*use)->usedBys.begin(), (*use)->usedBys.end(), this));
        }
    }

    BinaryInst::BinaryInst(Op op, Value *l, Value *r) : op(op), l(l), r(r) {
        switch (op) {
            case EQ:
            case NE:
            case SLE:
            case SLT:
            case SGE:
            case SGT:
                valueType = new BasicValueType(BasicValueType::I1);
            break;
            default:
                valueType = l->valueType->clone();
            break;
        }
        slot = 0;
        use(&this->l);
        use(&this->r);
    }

    LLVMType BinaryInst::type() const {
        return LLVMType::BINARY_INST;
    }

    void BinaryInst::print(std::ostream &os) const {
        os << "%" << slot << " = ";
        switch (op) {
            case ADD:
                os << "add";
                break;
            case SUB:
                os << "sub";
                break;
            case MUL:
                os << "mul";
                break;
            case SDIV:
                os << "sdiv";
                break;
            case SREM:
                os << "srem";
                break;
            case AND:
                os << "and";
                break;
            case OR:
                os << "or";
                break;
            case EQ:
                os << "icmp eq";
                break;
            case NE:
                os << "icmp ne";
                break;
            case SGE:
                os << "icmp sge";
                break;
            case SGT:
                os << "icmp sgt";
                break;
            case SLE:
                os << "icmp sle";
                break;
            case SLT:
                os << "icmp slt";
                break;
        }
        os << " ";
        l->valueType->print(os);
        os << " ";
        l->printRef(os);
        os << ", ";
        r->printRef(os);
        os << "; " << static_cast<int>(reg);
        os << std::endl;
    }

    void BinaryInst::getDefUse(std::unordered_set<int> &def, std::unordered_set<int> &use) {
        def.insert(slot);
        if (l->needsColor())
            use.insert(l->slot);
        if (r->needsColor())
            use.insert(r->slot);
    }

    bool BinaryInst::needsColor() const {
        return true;
    }

    CallInst::CallInst(bool requireSlot, Function* function, std::vector<Value *> const &args) : function(function), name(function->name), args(args) {
        valueType = function->valueType->clone();
        if (requireSlot) {
            // TODO: Return Value not used ?
            slot = 0;
        }
        // TODO: determined if funtion is pinned
        pinned = true;
        use(reinterpret_cast<Value **>(&this->function));
        for (int i = 0; i < args.size(); i++) {
            use(&this->args[i]);
        }
    }

    LLVMType CallInst::type() const {
        return LLVMType::CALL_INST;
    }

    void CallInst::print(std::ostream &os) const {
        if (slot != -1) {
            os << "%" << slot << " = ";
        }
        os << "call ";
        valueType->print(os);
        os << " @" << name;
        os << "(";
        for (int i = 0; i < args.size(); i++) {
            args[i]->valueType->print(os);
            os << " ";
            args[i]->printRef(os);
            if (i + 1 < args.size()) {
                os << ", ";
            }
        }

        os << ")";
        os << "; " << static_cast<int>(reg);
        os << std::endl;
    }

    void CallInst::getDefUse(std::unordered_set<int> &def, std::unordered_set<int> &use) {
        Instruction::getDefUse(def, use);
        if (slot >= 0)
            def.insert(slot);
        for (auto arg : args) {
            if (arg->needsColor())
                use.insert(arg->slot);
        }
    }

    bool CallInst::needsColor() const {
        return slot >= 0;
    }

    AllocaInst::AllocaInst(VariableType& variableType) {
        slot = 0;
        allocType = ValueType::fromVariableType(variableType);
        valueType = allocType->clone();
        valueType = new PtrValueType(valueType);
    }

    AllocaInst::AllocaInst(ValueType* allocType) : allocType(allocType->clone()) {
        slot = 0;
        valueType = new PtrValueType(allocType->clone());
    }

    LLVMType AllocaInst::type() const {
        return LLVMType::ALLOCA_INST;
    }

    void AllocaInst::print(std::ostream &os) const {
        os << "%" << slot << " = alloca ";
        allocType->print(os);
        BasicValueType* basicType = nullptr;
        if (allocType->type() == ValueType::BASIC) {
            basicType = static_cast<BasicValueType*>(allocType);
        } else if (allocType->type() == ValueType::ARRAY) {
            ArrayValueType* arrayType = static_cast<ArrayValueType*>(allocType);
            if (arrayType->value->type() == ValueType::BASIC) {
                basicType = static_cast<BasicValueType*>(arrayType->value);
            }
        }
        os << "; " << static_cast<int>(reg);
        os << std::endl;
    }

    void AllocaInst::getDefUse(std::unordered_set<int> &def, std::unordered_set<int> &use) {

    }

    bool AllocaInst::isPromotable() const {
        for (auto used : usedBys) {
            if (used->type() != LLVMType::LOAD_INST && used->type() != LLVMType::STORE_INST) {
                return false;
            }
        }
        return true;
    }

    bool AllocaInst::needsColor() const {
        return false;
    }

    LoadInst::LoadInst(Value *ptr) : ptr(ptr) {
        pinned = true;
        slot = 0;
        PtrValueType* v = static_cast<PtrValueType*>(ptr->valueType);
        valueType = v->value->clone();
        use(&this->ptr);
    }

    LLVMType LoadInst::type() const {
        return LLVMType::LOAD_INST;
    }

    void LoadInst::print(std::ostream &os) const {
        os << "%" << slot << " = load ";
        valueType->print(os);
        os << ", ";
        ptr->valueType->print(os);
        os << " ";
        ptr->printRef(os);
        os << "; " << static_cast<int>(reg);
        os << std::endl;
    }

    void LoadInst::getDefUse(std::unordered_set<int> &def, std::unordered_set<int> &use) {
        def.insert(slot);
        if (ptr->needsColor())
            use.insert(ptr->slot);
    }

    bool LoadInst::needsColor() const {
        return true;
    }

    StoreInst::StoreInst(Value *value, Value *ptr) : value(value), ptr(ptr), isArgInit(false) {
        pinned = true;
        valueType = new BasicValueType(BasicValueType::VOID);
        use(&this->value);
        use(&this->ptr);
    }

    StoreInst::StoreInst(Value *value, Value *ptr, bool isArgInit) : value(value), ptr(ptr), isArgInit(isArgInit) {
        pinned = true;
        valueType = new BasicValueType(BasicValueType::VOID);
        use(&this->value);
        use(&this->ptr);
    }

    LLVMType StoreInst::type() const {
        return LLVMType::STORE_INST;
    }

    void StoreInst::print(std::ostream &os) const {
        os << "store ";
        value->valueType->print(os);
        os << " ";
        value->printRef(os);
        os << ", ";
        ptr->valueType->print(os);
        os << " ";
        ptr->printRef(os);
        os << std::endl;
    }

    void StoreInst::getDefUse(std::unordered_set<int> &def, std::unordered_set<int> &use) {
        if (value->needsColor())
            use.insert(value->slot);
        if (ptr->needsColor())
            use.insert(ptr->slot);
    }

    GetElementPtr::GetElementPtr(Value *ptr, Value* idx) : ptr(ptr), idx(idx) {
        slot = 0;
        ValueType* inner = static_cast<PtrValueType*>(ptr->valueType)->value;
        if (inner->type() == ValueType::ARRAY) {
            valueType = new PtrValueType(static_cast<ArrayValueType*>(inner)->value->clone());
        } else {
            valueType = ptr->valueType->clone();
        }
        use(&this->ptr);
        use(&this->idx);
    }

    LLVMType GetElementPtr::type() const {
        return LLVMType::GET_ELEMENT_PTR_INST;
    }

    void GetElementPtr::print(std::ostream &os) const {
        os << "%" << slot << " = getelementptr ";
        ValueType* inner = static_cast<PtrValueType*>(ptr->valueType)->value;
        inner->print(os);
        os << ", ";
        ptr->valueType->print(os);
        os << " ";
        ptr->printRef(os);
        os << ", ";
        if (inner->type() == ValueType::ARRAY) {
            os << "i32 0, ";
            idx->valueType->print(os);
            os << " ";
            idx->printRef(os);
        } else {
            idx->valueType->print(os);
            os << " ";
            idx->printRef(os);
        }
        os << "; " << static_cast<int>(reg);
        os << std::endl;
    }

    void GetElementPtr::getDefUse(std::unordered_set<int> &def, std::unordered_set<int> &use) {
        def.insert(slot);
        if (ptr->needsColor())
            use.insert(ptr->slot);
        if (idx->needsColor())
            use.insert(idx->slot);
    }

    bool GetElementPtr::needsColor() const {
        return true;
    }

    ZextInst::ZextInst(Value *v) : v(v) {
        slot = 0;
        valueType = new BasicValueType(BasicValueType::I32);
        use(&this->v);
    }

    LLVMType ZextInst::type() const {
        return LLVMType::ZEXT_INST;
    }

    void ZextInst::print(std::ostream &os) const {
        os << "%" << slot << " = zext ";
        v->valueType->print(os);
        os << " ";
        v->printRef(os);
        os << " to ";
        valueType->print(os);
        os << "; " << static_cast<int>(reg);
        os << std::endl;
    }

    void ZextInst::getDefUse(std::unordered_set<int> &def, std::unordered_set<int> &use) {
        def.insert(slot);
        if (v->needsColor())
            use.insert(v->slot);
    }

    bool ZextInst::needsColor() const {
        return true;
    }

    TruncInst::TruncInst(Value *v) : v(v) {
        slot = 0;
        valueType = new BasicValueType(BasicValueType::I8);
        use(&this->v);
    }

    LLVMType TruncInst::type() const {
        return LLVMType::TRUNC_INST;
    }

    void TruncInst::print(std::ostream &os) const {
        os << "%" << slot << " = trunc ";
        v->valueType->print(os);
        os << " ";
        v->printRef(os);
        os << " to ";
        valueType->print(os);
        os << "; " << static_cast<int>(reg);
        os << std::endl;
    }

    void TruncInst::getDefUse(std::unordered_set<int> &def, std::unordered_set<int> &use) {
        def.insert(slot);
        if (v->needsColor())
            use.insert(v->slot);
    }

    bool TruncInst::needsColor() const {
        return true;
    }

    RetInst::RetInst(Value *value) : value(value) {
        pinned = true;
        if (value != nullptr) {
            use(&this->value);
        }
    }

    LLVMType RetInst::type() const {
        return LLVMType::RET_INST;
    }

    void RetInst::print(std::ostream &os) const {
        os << "ret ";
        if (value != nullptr) {
            value->valueType->print(os);
            os << " ";
            value->printRef(os);
        } else {
            os << "void";
        }
        os << std::endl;
    }

    void RetInst::getDefUse(std::unordered_set<int> &def, std::unordered_set<int> &use) {
        if (value != nullptr && value->needsColor()) {
            use.insert(value->slot);
        }
    }

    BranchInst::BranchInst(BasicBlock *uncond) : cond(nullptr), ifTrue(uncond), ifFalse(nullptr) {
        pinned = true;
    }

    BranchInst::BranchInst(Value *cond, BasicBlock *ifTrue, BasicBlock *ifFalse) : cond(cond), ifTrue(ifTrue), ifFalse(ifFalse) {
        use(&this->cond);
        pinned = true;
    }

    LLVMType BranchInst::type() const {
        return LLVMType::BRANCH_INST;
    }

    void BranchInst::print(std::ostream &os) const {
        os << "br ";
        if (cond != nullptr) {
            cond->valueType->print(os);
            os << " ";
            cond->printRef(os);
            os << ", ";
            ifTrue->valueType->print(os);
            os << " ";
            ifTrue->printRef(os);
            os << ", ";
            ifFalse->valueType->print(os);
            os << " ";
            ifFalse->printRef(os);
        } else {
            ifTrue->valueType->print(os);
            os << " ";
            ifTrue->printRef(os);
        }
        os << std::endl;
    }

    void BranchInst::printRef(std::ostream &os) const {
        Instruction::printRef(os);
    }

    void BranchInst::getDefUse(std::unordered_set<int> &def, std::unordered_set<int> &use) {
        if (cond != nullptr && cond->needsColor()) {
            use.insert(cond->slot);
        }
    }

    PhiInst::PhiInst(AllocaInst *alloc) : alloc(alloc) {
        slot = 0;
        valueType = alloc->allocType->clone();
        pinned = true;
    }

    LLVMType PhiInst::type() const {
        return LLVMType::PHI_INST;
    }

    bool PhiInst::isNecessary() {
        if (opt.size() == 2) {
            for (auto ent : opt) {
                if (ent.second == this) return false;
            }
        }
        return true;
    }

    void PhiInst::print(std::ostream &os) const {
        os << "%" << slot << " = phi ";
        alloc->allocType->print(os);
        os << " ";
        int idx = 0;
        for (auto ent : opt) {
            os << "[ ";
            ent.second->printRef(os);
            os << ", %" << ent.first->slot << " ]";
            if (idx + 1 < opt.size()) {
                os << ", ";
            }
            idx++;
        }
        os << std::endl;
    }

    void PhiInst::addOpt(BasicBlock *bb, Value *val) {
        if (opt.find(bb) == opt.end()) {
            opt[bb] = val;
            use(&opt[bb]);
        } else {
            opt[bb] = val;
        }
    }

    bool PhiInst::needsColor() const {
        return true;
    }

    MoveInst::MoveInst(Value *dst, Value *src) : dst(dst), src(src) {
        slot = 0;
        valueType = src->valueType->clone();
    }

    LLVMType MoveInst::type() const {
        return LLVMType::MOVE;
    }

    void MoveInst::print(std::ostream &os) const {
        os << "%" << dst->slot << " = ";
        src->printRef(os);
        os << "; " << static_cast<int>(reg);
        os << std::endl;
    }

    void MoveInst::getDefUse(std::unordered_set<int> &def, std::unordered_set<int> &use) {
        def.insert(slot);
        if (src->needsColor()) {
            use.insert(src->slot);
        }
    }

    bool MoveInst::needsColor() const {
        return true;
    }

    BackupArg::BackupArg(int idx) : idx(idx) {
        slot = 0;
        valueType = new BasicValueType(BasicValueType::I32);
    }

    LLVMType BackupArg::type() const {
        return LLVMType::BACKUP_ARG;
    }

    void BackupArg::print(std::ostream &os) const {
        os << "backup " << idx;
        os << "; " << static_cast<int>(reg);
        os << std::endl;
    }

    void BackupArg::getDefUse(std::unordered_set<int> &def, std::unordered_set<int> &use) {
        def.insert(slot);
    }

    bool BackupArg::needsColor() const {
        return true;
    }

    RecoverArg::RecoverArg(BackupArg *backup) : backup(backup) {

    }

    LLVMType RecoverArg::type() const {
        return LLVMType::RECOVER_ARG;
    }

    void RecoverArg::print(std::ostream &os) const {
        os << "recover " << backup->idx;
        os << std::endl;
    }

    void RecoverArg::getDefUse(std::unordered_set<int> &def, std::unordered_set<int> &use) {
        use.insert(backup->slot);
    }

    MoveTmp::MoveTmp() {
        slot = 0;
    }

    LLVMType MoveTmp::type() const {
        return LLVMType::MOVE_TMP;
    }

    void MoveTmp::print(std::ostream &os) const {
        os << "%" << slot << std::endl;
    }

    void MoveTmp::printRef(std::ostream &os) const {
        os << "%" << slot;
    }

    ArgumentAddress::ArgumentAddress(Argument* arg) {
        valueType = new PtrValueType(arg->valueType->clone());
    }

    LLVMType ArgumentAddress::type() const {
        return LLVMType::ARG_ADDR;
    }

    void ArgumentAddress::print(std::ostream &os) const {

    }

    void ArgumentAddress::printRef(std::ostream &os) const {

    }

    Module::Module() {
        getInt = new Function("getint", BasicValueType::I32);
        getChar = new Function("getchar", BasicValueType::I32);
        putInt = new Function("putint", BasicValueType::VOID);
        Argument* putIntArg = new Argument();
        putIntArg->valueType = new BasicValueType(BasicValueType::I32);
        putInt->args.push_back(putIntArg);
        putChar = new Function("putch", BasicValueType::VOID);
        Argument* putCharArg = new Argument();
        putCharArg->valueType = new BasicValueType(BasicValueType::I32);
        putChar->args.push_back(putCharArg);
        putStr = new Function("putstr", BasicValueType::VOID);
        Argument* putStrArg = new Argument();
        putStrArg->valueType = new PtrValueType(new BasicValueType(BasicValueType::I8));
        putStr->args.push_back(putStrArg);
    }

    void Module::print(std::ostream &os) const {
        os << "declare i32 @getint()" << std::endl
            << "declare i32 @getchar()" << std::endl
            << "declare void @putint(i32)"<< std::endl
            << "declare void @putch(i8)" << std::endl
            << "declare void @putstr(i8*)" << std::endl;

        os << std::endl;
        for (auto var : variables) {
            var->print(os);
        }
        os << std::endl;
        for (auto stringLiteral : strings) {
            stringLiteral->print(os);
        }
        os << std::endl;
        for (auto func : functions) {
            func->print(os);
            os << std::endl;
        }

        main->print(os);
    }

    StringLiteral * Module::addStringLiteral(std::string const &string) {
        StringLiteral* stringLiteral = new StringLiteral(string);
        stringLiteral->refId = strings.size();
        strings.push_back(stringLiteral);
        return stringLiteral;
    }

    void Module::preprocess() {
        setAllocas();
        arrangeBlocks();

        AnalyzeDominance dominaceAnalyzer(this);
        dominaceAnalyzer.process();

        Mem2Reg mem2Reg(this);
        mem2Reg.process();

        AnalyzeSideEffect sideEffectAnalyzer(this);
        sideEffectAnalyzer.process();

        //DeadCode deadCode(this);
        //deadCode.process();
        GCM gcm(this);
        gcm.process();

        EliminatePhis eliminatePhis(this);
        eliminatePhis.process();

        SaveArgument saveArgument(this);
        saveArgument.process();

        for (Function *function : functions) {
            function->rebuildCFG();
        }
        main->rebuildCFG();

        for (Function* function : functions) {
            function->fillSlot();
        }
        main->fillSlot();

        AllocateRegisters allocateRegister(this);
        allocateRegister.process();
    }

    void Module::arrangeBlocks() {
        for (Function* function : functions) {
            function->arrangeBlocks();
        }
        main->arrangeBlocks();
    }

    void Module::setAllocas() {
        for (Function* function : functions) {
            function->setAllocas();
        }
        main->setAllocas();
    }

    void Module::calcSideEffects() {

    }

    bool isPtrAlias(Value *l, Value *r) {
        if (l->valueType->type() != ValueType::PTR || r->valueType->type() != ValueType::PTR) {
            return false;
        }
        if (GetElementPtr *lGEP = dynamic_cast<GetElementPtr*>(l)) {
            return isPtrAlias(lGEP->ptr, r);
        }
        if (GetElementPtr *rGEP = dynamic_cast<GetElementPtr *>(r)) {
            return isPtrAlias(l, rGEP->ptr);
        }
        return l == r;
    }
} // thm
