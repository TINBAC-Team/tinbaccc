#include <asm_arm/builder.h>
#include <iostream>

namespace asm_arm {
    Builder::Builder(Module *m) : module(m) {}

    Function *Builder::createFunction(ir::Function *f) {
        auto *ret = new Function(f);
        module->functionList.push_back(ret);
        curFunction = ret;
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

    template<typename RT>
    RT * Builder::createInst2_1(Inst::Op o, Operand *d, int s_imm) {
        switch (o) {
            case Inst::Op::MOV: {
                MOVInst *ret = new MOVInst(d, s_imm);
                curBlock->insertAtEnd(ret);
                return ret;
            }
            case Inst::Op::CMP: {
                CMPInst *ret = new CMPInst(d, s_imm);
                curBlock->insertAtEnd(ret);
                return ret;
            }
            case Inst::Op::TST: {
                TSTInst *ret = new TSTInst(d, s_imm);
                curBlock->insertAtEnd(ret);
                return ret;
            }
            default:
                std::cerr << "Error in Builder!\n";
                return nullptr;
        }
    }

    template<typename RT>
    RT * Builder::createInst2_1_(Inst::Op o, Operand *d, Operand *s) {
        switch (o) {
            case Inst::Op::MOV: {
                MOVInst *ret = new MOVInst(d, s);
                curBlock->insertAtEnd(ret);
                return ret;
            }
            case Inst::Op::CMP: {
                CMPInst *ret = new CMPInst(d, s);
                curBlock->insertAtEnd(ret);
                return ret;
            }
            case Inst::Op::TST: {
                TSTInst *ret = new TSTInst(d, s);
                curBlock->insertAtEnd(ret);
                return ret;
            }
            default:
                std::cerr << "Error in Builder!\n";
                return nullptr;
        }
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

    template <typename RT>
    RT * Builder::createInst1(Inst::Op o, Operand *d, Operand * s1, int s2_imm) {
        switch (o) {
            case Inst::Op::ADD: {
                ADDInst * ret = new ADDInst(d, s1, s2_imm);
                curBlock->insertAtEnd(ret);
                return ret;
            }
            case Inst::Op::SUB: {
                SUBInst * ret = new SUBInst(d, s1, s2_imm);
                curBlock->insertAtEnd(ret);
                return ret;
            }
            case Inst::Op::AND: {
                ANDInst * ret = new ANDInst(d, s1, s2_imm);
                curBlock->insertAtEnd(ret);
                return ret;
            }
            case Inst::Op::ORR: {
                ORRInst * ret = new ORRInst(d, s1, s2_imm);
                curBlock->insertAtEnd(ret);
                return ret;
            }
            case Inst::Op::EOR: {
                EORInst * ret = new EORInst(d, s1, s2_imm);
                curBlock->insertAtEnd(ret);
                return ret;
            }
            default:
                std::cerr << "Error in Builder!\n";
                return nullptr;
        }
    }

    template <typename RT>
    RT * Builder::createInst1_(Inst::Op o, Operand * d, Operand * s1, Operand* s2) {
        switch (o) {
            case Inst::Op::ADD: {
                ADDInst * ret = new ADDInst(d, s1, s2);
                curBlock->insertAtEnd(ret);
                return ret;
            }
            case Inst::Op::SUB: {
                SUBInst * ret = new SUBInst(d, s1, s2);
                curBlock->insertAtEnd(ret);
                return ret;
            }
            case Inst::Op::AND: {
                ANDInst * ret = new ANDInst(d, s1, s2);
                curBlock->insertAtEnd(ret);
                return ret;
            }
            case Inst::Op::ORR: {
                ORRInst * ret = new ORRInst(d, s1, s2);
                curBlock->insertAtEnd(ret);
                return ret;
            }
            case Inst::Op::EOR: {
                EORInst * ret = new EORInst(d, s1, s2);
                curBlock->insertAtEnd(ret);
                return ret;
            }
            default:
                std::cerr << "Error in Builder!\n";
                return nullptr;
        }
    }

    template <typename RT>
    RT * Builder::createInst2(Inst::Op o, Operand *d, Operand *s1, Operand *s2) {
        switch (o) {
            case Inst::Op::MUL: {
                MULInst *ret = new MULInst(d, s1, s2);
                curBlock->insertAtEnd(ret);
                return ret;
            }
            case Inst::Op::SDIV: {
                SDIVInst *ret = new SDIVInst(d, s1, s2);
                curBlock->insertAtEnd(ret);
                return ret;
            }
            default:
                std::cerr << "Error in Builder!\n";
                return nullptr;
        }
    }

    LABELofInst * Builder::createLABELofInst(std::string& lb) {
        LABELofInst *ret = new LABELofInst(lb);
        curBlock->insertAtEnd(ret);
        return ret;
    }
}