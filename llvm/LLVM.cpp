//
// Created by slty5 on 24-10-27.
//

#include "LLVM.h"

#include <complex>
#include <functional>
#include <iostream>
#include <ostream>
#include <bits/locale_facets_nonio.h>

#include "Mem2Reg.h"

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

    BasicBlock::BasicBlock(Function *function) : function(function) {
        valueType = new BasicValueType(BasicValueType::LABEL);
        slot = 0;
    }

    LLVMType BasicBlock::type() const {
        return LLVMType::BASIC_BLOCK;
    }

    void BasicBlock::print(std::ostream &os) const {
        os << slot << ":" << std::endl;
        // os << "; doms=";
        // for (auto bb : doms) {
        //     os << bb->slot << " ";
        // }
        // os << std::endl;
        // os << "; df=";
        // for (auto bb : df) {
        //     os << bb->slot << " ";
        // }
        // os << std::endl;
        // os << std::endl;
        // if (iDom != nullptr)
        //     os << "; idom=" << iDom->slot << std::endl;
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
        for (auto inst : insts) {
            std::unordered_set<Value *> instDef, instUse;
            inst->getDefUse(instDef, instUse);
            for (auto elm : instDef) {
                if (def.find(elm) == def.end()) {
                    def.insert(elm);
                }
            }
            for (auto elm : instUse) {
                if (use.find(elm) == use.end()) {
                    use.insert(elm);
                }
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
            insts.push_back(inst);
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
                inst->slot = function->slotTracker.allocSlot();
                if (inst->type() == LLVMType::ALLOCA_INST) {
                    AllocaInst *allocInst = static_cast<AllocaInst *>(inst);
                    if (allocInst->argIdx == -1) {
                        PtrValueType *ptr = static_cast<PtrValueType *>(inst->valueType);
                        int size = 0;
                        if (ptr->value->type() == ValueType::ARRAY) {
                            ArrayValueType *arrayType = static_cast<ArrayValueType *>(ptr->value);
                            BasicValueType *basicType = static_cast<BasicValueType *>(arrayType->value);
                            if (basicType->type() == BasicValueType::I8) {
                                size = arrayType->arrayLen;
                            } else {
                                size = 4 * arrayType->arrayLen;
                            }
                            function->slotTracker.useArray(inst->slot, size);
                        } else {
                            function->slotTracker.useStack(inst->slot);
                        }
                    } else {
                        function->slotTracker.allocArgs[allocInst->argIdx] = allocInst;
                    }
                } else {
                    function->slotTracker.useStack(inst->slot);
                }
            }
        }
    }

    LLVMType Constant::type() const {
        return LLVMType::CONSTANT;
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

    Function::Function(std::string const &name, BasicValueType::BasicType retType): frame(nullptr) {
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
    }

    void Function::calcDominators() {
        blocks[0]->doms.insert(blocks[0]);
        bool changed = true;
        while (changed) {
            changed = false;
            for (int i = 1; i < blocks.size(); i++) {
                std::unordered_set<BasicBlock *> newDom;
                newDom.insert(blocks[i]);
                if (blocks[i]->froms.empty()) continue;
                for (auto candidate : blocks[i]->froms[0]->doms) {
                    bool valid = true;
                    for (auto from : blocks[i]->froms) {
                        if (from->doms.find(candidate) == from->doms.end()) {
                            valid = false;
                            break;
                        }
                    }
                    if (valid) {
                        newDom.insert(candidate);
                    }
                }
                if (newDom != blocks[i]->doms) {
                    changed = true;
                    blocks[i]->doms = newDom;
                }
            }
        }
        for (int i = 1; i < blocks.size(); ++i) {
            for (auto dom : blocks[i]->doms) {
                // dom is strict dom of i
                if (dom != blocks[i]) {
                    bool valid = true;
                    for (auto d : blocks[i]->doms) {
                        // d is strict dom of i
                        if (d != blocks[i] && d != dom) {
                            if (d->doms.find(dom) != d->doms.end()) {
                                valid = false;
                                break;
                            }
                        }
                    }
                    if (valid) {
                        blocks[i]->iDom = dom;
                        break;
                    }
                }
            }
        }
        for (int i = 0; i < blocks.size(); ++i) {
            for (auto to : blocks[i]->tos) {
                BasicBlock *x = blocks[i];
                while (!(x != to && to->doms.find(x) != to->doms.end())) {
                    x->df.insert(to);
                    x = x->iDom;
                    if (x == nullptr) break;
                }
            }
        }
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
                std::unordered_set<Value *> newOut;
                for (BasicBlock* to : block->tos) {
                    newOut.insert(to->in.begin(), to->in.end());
                }
                if (newOut != block->out) {
                    changed = true;
                    block->out = newOut;
                    block->in = block->use;
                    for (auto elm : block->out) {
                        if (block->def.find(elm) == block->def.end()) {
                            block->in.insert(elm);
                        }
                    }
                }
            }
        }
    }

    void Function::fillSlot() {
        for (Argument* arg : args) {
            arg->slot = slotTracker.allocSlot();
        }
        slotTracker.allocArgs.resize(args.size());
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

    void Instruction::getDefUse(std::unordered_set<Value *> &def, std::unordered_set<Value *> &use) {
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
        color = 0;
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
        os << std::endl;
    }

    void BinaryInst::getDefUse(std::unordered_set<Value *> &def, std::unordered_set<Value *> &use) {
        Instruction::getDefUse(def, use);
        def.insert(this);
        if (l->color >= 0)
            use.insert(l);
        if (r->color >= 0)
            use.insert(r);
    }

    CallInst::CallInst(bool requireSlot, Function* function, std::vector<Value *> const &args) : function(function), name(function->name), args(args) {
        valueType = function->valueType->clone();
        if (requireSlot) {
            // TODO: Return Value not used ?
            slot = 0;
            color = 0;
        }
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
        os << ")" << std::endl;
    }

    void CallInst::getDefUse(std::unordered_set<Value *> &def, std::unordered_set<Value *> &use) {
        Instruction::getDefUse(def, use);
        if (slot >= 0)
            def.insert(this);
        for (auto arg : args) {
            if (arg->color >= 0)
                use.insert(arg);
        }
    }

    AllocaInst::AllocaInst(VariableType& variableType) {
        slot = 0;
        allocType = ValueType::fromVariableType(variableType);
        valueType = allocType->clone();
        valueType = new PtrValueType(valueType);
    }

    AllocaInst::AllocaInst(ValueType* allocType, int argIdx) : allocType(allocType->clone()), argIdx(argIdx) {
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
        os << std::endl;
    }

    void AllocaInst::getDefUse(std::unordered_set<Value *> &def, std::unordered_set<Value *> &use) {
        Instruction::getDefUse(def, use);
    }

    bool AllocaInst::isPromotable() const {
        for (auto used : usedBys) {
            if (used->type() != LLVMType::LOAD_INST && used->type() != LLVMType::STORE_INST) {
                return false;
            }
        }
        return true;
    }

    LoadInst::LoadInst(Value *ptr) : ptr(ptr) {
        slot = 0;
        color = 0;
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
        os << std::endl;
    }

    void LoadInst::getDefUse(std::unordered_set<Value *> &def, std::unordered_set<Value *> &use) {
        Instruction::getDefUse(def, use);
        def.insert(this);
        if (ptr->color >= 0)
            use.insert(ptr);
    }

    StoreInst::StoreInst(Value *value, Value *ptr) : value(value), ptr(ptr), isArgInit(false) {
        valueType = new BasicValueType(BasicValueType::VOID);
        use(&this->value);
        use(&this->ptr);
    }

    StoreInst::StoreInst(Value *value, Value *ptr, bool isArgInit) : value(value), ptr(ptr), isArgInit(isArgInit) {
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

    void StoreInst::getDefUse(std::unordered_set<Value *> &def, std::unordered_set<Value *> &use) {
        Instruction::getDefUse(def, use);
        use.insert(value);
        if (ptr->color >= 0)
            use.insert(ptr);
    }

    GetElementPtr::GetElementPtr(Value *ptr, Value* idx) : ptr(ptr), idx(idx) {
        slot = 0;
        color = 0;
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
        os << std::endl;
    }

    void GetElementPtr::getDefUse(std::unordered_set<Value *> &def, std::unordered_set<Value *> &use) {
        Instruction::getDefUse(def, use);
        def.insert(this);
        use.insert(ptr);
        use.insert(idx);
    }

    ZextInst::ZextInst(Value *v) : v(v) {
        slot = 0;
        color = 0;
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
        os << std::endl;
    }

    void ZextInst::getDefUse(std::unordered_set<Value *> &def, std::unordered_set<Value *> &use) {
        Instruction::getDefUse(def, use);
        def.insert(this);
        use.insert(v);
    }

    TruncInst::TruncInst(Value *v) : v(v) {
        slot = 0;
        color = 0;
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
        os << std::endl;
    }

    void TruncInst::getDefUse(std::unordered_set<Value *> &def, std::unordered_set<Value *> &use) {
        Instruction::getDefUse(def, use);
        def.insert(this);
        use.insert(v);
    }

    RetInst::RetInst(Value *value) : value(value) {
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

    void RetInst::getDefUse(std::unordered_set<Value *> &def, std::unordered_set<Value *> &use) {
        Instruction::getDefUse(def, use);
        if (value != nullptr) {
            use.insert(value);
        }
    }

    BranchInst::BranchInst(BasicBlock *uncond) : cond(nullptr), ifTrue(uncond), ifFalse(nullptr) {

    }

    BranchInst::BranchInst(Value *cond, BasicBlock *ifTrue, BasicBlock *ifFalse) : cond(cond), ifTrue(ifTrue), ifFalse(ifFalse) {
        use(&this->cond);
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

    void BranchInst::getDefUse(std::unordered_set<Value *> &def, std::unordered_set<Value *> &use) {
        Instruction::getDefUse(def, use);
        if (cond != nullptr) {
            use.insert(cond);
        }
    }

    PhiInst::PhiInst(AllocaInst *alloc) : alloc(alloc) {
        slot = 0;
        valueType = alloc->allocType->clone();
    }

    LLVMType PhiInst::type() const {
        return LLVMType::PHI_INST;
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
        for (Function* function : functions) {
            function->arrangeBlocks();
            function->calcDominators();
            function->setAllocas();
            function->livenessAnalysis();
        }
        main->arrangeBlocks();
        main->calcDominators();
        main->setAllocas();
        Mem2Reg mem2Reg(this);
        mem2Reg.process();

        for (Function* function : functions) {
            function->fillSlot();
        }
        main->fillSlot();
    }
} // thm
