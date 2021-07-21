#include <asm_arm/builder.h>
#include <iostream>

namespace asm_arm {
    Builder::Builder(Module *m) : module(m) {}

    Operand * Builder::getOperandOfValue(ir::Value *val) {
        auto got = value_map.find(val);
        if(got != value_map.end())
            return got->second;
        return nullptr;
    }

    void Builder::setOperandOfValue(ir::Value *val, Operand *operand) {
        value_map[val] = operand;
    }

    Operand *Builder::getOrCreateOperandOfValue(ir::Value *val) {
        Operand *ret = getOperandOfValue(val);
        if(ret)
            return ret;
        // safety check for a hack: This function is designed specifically for constants.
        // It's used to reduce some constant checks in BinaryInst generation.
        if(val->optype!=ir::OpType::CONST)
            throw std::runtime_error("Non-const value should have been created!");
        ret = val->codegen(*this);
        setOperandOfValue(val, ret);
        return ret;
    }

    BasicBlock *Builder::getASMBBfromIRBB(ir::BasicBlock *bb) {
        auto got = block_map.find(bb);
        if (got != block_map.end())
            return got->second;
        return nullptr;
    }

    void Builder::setASMBBtoIRBB(ir::BasicBlock *irbb, BasicBlock *bb) {
        block_map[irbb] = bb;
    }

    void Builder::addPendingBBPtr(BasicBlock **pbb, ir::BasicBlock *bb) {
        block_fill_list.emplace_back(pbb, bb);
    }

    void Builder::fillBBPtr() {
        for (auto &i:block_fill_list) {
            BasicBlock *tgt_bb = getASMBBfromIRBB(i.second);
            if(!tgt_bb)
                throw std::runtime_error("No ASM BasicBlock for IR BasicBlock.");
            *i.first = tgt_bb;
        }
    }

    Function *Builder::createFunction(ir::Function *f) {
        Operand::resetRegMap();
        auto *ret = new Function(f);
        module->functionList.push_back(ret);
        curFunction = ret;
        value_map.clear();
        block_map.clear();
        block_fill_list.clear();
        return ret;
    }

    BasicBlock *Builder::createBlock() {
        auto *bb = new BasicBlock();
        curFunction->appendBlock(bb);
        curBlock = bb;
        return bb;
    }

    LDRInst *Builder::createLDR(int v) {
        auto ret = new LDRInst(v, Operand::newVReg());
        curBlock->insertAtEnd(ret);
        return ret;
    }

    LDRInst *Builder::createLDR(std::string s) {
        auto ret = new LDRInst(s, Operand::newVReg());
        curBlock->insertAtEnd(ret);
        return ret;
    }

    LDRInst *Builder::createLDR(Operand *s, Operand *o) {
        auto ret = new LDRInst(Operand::newVReg(), s, o);
        curBlock->insertAtEnd(ret);
        return ret;
    }

    STRInst * Builder::createSTR(Operand *v, Operand *a, Operand *o)  {
        STRInst * ret = new STRInst(v, a, o);
        curBlock->insertAtEnd(ret);
        return ret;
    }

    ADRInst * Builder::createADR(std::string& lb) {
        ADRInst * ret = new ADRInst(Operand::newVReg(), lb);
        curBlock->insertAtEnd(ret);
        return ret;
    }

    BInst *Builder::createBInst(ir::BasicBlock *bb, Inst::OpCond c) {
        auto ret = new BInst(c);
        curBlock->insertAtEnd(ret);
        addPendingBBPtr(&(ret->tgt), bb);
        return ret;
    }

    BInst *Builder::createBInst(BasicBlock *bb, Inst::OpCond c) {
        auto ret = new BInst(c);
        curBlock->insertAtEnd(ret);
        ret->tgt = bb;
        return ret;
    }

    CallInst *Builder::createCall(std::string lb, int np, bool is_void) {
        auto ret = new CallInst(np, std::move(lb), is_void);
        curBlock->insertAtEnd(ret);
        return ret;
    }

    BinaryInst *Builder::createBinaryInst(Inst::Op op, Operand *lhs, Operand *rhs) {
        auto ret = new BinaryInst(op, Operand::newVReg(), lhs, rhs);
        curBlock->insertAtEnd(ret);
        return ret;
    }

    BinaryInst *Builder::moveSP(bool is_sub, int len) {
        auto ret = new BinaryInst(
                is_sub ? Inst::Op::SUB : Inst::Op::ADD,
                Operand::getReg(Reg::sp),
                Operand::getReg(Reg::sp),
                Operand::newImm(len));
        curBlock->insertAtEnd(ret);
        return ret;
    }

    TernaryInst *Builder::createTernaryInst(Inst::Op op, Operand *o1, Operand *o2, Operand *o3) {
        auto ret = new TernaryInst(op, Operand::newVReg(), o1, o2, o3);
        curBlock->insertAtEnd(ret);
        return ret;
    }

    CMPInst *Builder::createCMPInst(Operand *lhs, Operand *rhs) {
        auto ret = new CMPInst(lhs, rhs);
        curBlock->insertAtEnd(ret);
        return ret;
    }

    MOVInst *Builder::createMOVInst(Operand *dst, Operand *src) {
        auto ret = new MOVInst(dst, src);
        curBlock->insertAtEnd(ret);
        return ret;
    }
}