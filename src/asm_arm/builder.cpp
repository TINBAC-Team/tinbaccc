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

    Function *Builder::createFunction(ir::Function *f) {
        auto *ret = new Function(f);
        module->functionList.push_back(ret);
        curFunction = ret;
        value_map.clear();
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

    STRInst * Builder::createSTR(std::string& s) {
        STRInst * ret = new STRInst(s, Operand::newVReg());
        curBlock->insertAtEnd(ret);
        return ret;
    }

    STRInst * Builder::createSTR(int v) {
        STRInst * ret = new STRInst(v, Operand::newVReg());
        curBlock->insertAtEnd(ret);
        return ret;
    }

    ADRInst * Builder::createADR(std::string& lb) {
        ADRInst * ret = new ADRInst(Operand::newVReg(), lb);
        curBlock->insertAtEnd(ret);
        return ret;
    }

    BInst * Builder::createBInst(std::string &lb) {
        BInst * ret = new BInst(lb);
        curBlock->insertAtEnd(ret);
        return ret;
    }

    BInst * Builder::createBInst(std::string &lb, BInst::Type sf) {
        BInst * ret = new BInst(lb, sf);
        curBlock->insertAtEnd(ret);
        return ret;
    }

    BinaryInst *Builder::createBinaryInst(Inst::Op op, Operand *lhs, Operand *rhs) {
        auto ret = new BinaryInst(op, Operand::newVReg(), lhs, rhs);
        curBlock->insertAtEnd(ret);
        return ret;
    }
}