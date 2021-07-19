#include <asm_arm/instructions.h>
#include <ir/ir.h>

#include <utility>
#include <stdexcept>

namespace asm_arm {

    Operand *Operand::newImm(int v) {
        auto ret = new Operand(Type::Imm);
        ret->val = v;
        return ret;
    }

    Operand *Operand::newReg(Reg r) {
        auto ret = new Operand(Type::Reg);
        ret->reg = r;
        return ret;
    }

    Operand *Operand::newVReg() {
        return new Operand(Type::VReg);
    }

    bool Operand::op2Imm(int i) {
        int lowbit;

        /* Fast return for 0 and small values.  We must do this for zero, since
           the code below can't handle that one case.  */
        if ((i & ~(unsigned int) 0xff) == 0)
            return true;

        /* Get the number of trailing zeros.  */
        lowbit = __builtin_ffs((int) i) - 1;

        /* Only even shifts are allowed in ARM mode so round down to the
           nearest even number.  */
        lowbit &= ~1;

        if ((i & ~(((unsigned int) 0xff) << lowbit)) == 0)
            return true;
        /* Allow rotated constants in ARM mode.  */
        if (lowbit <= 4
            && ((i & ~0xc000003f) == 0
                || (i & ~0xf000000f) == 0
                || (i & ~0xfc000003) == 0))
            return true;
        return false;
    }

    LDRInst::LDRInst(std::string l, Operand *d) : Inst(Inst::Op::LDR), type(Type::LABEL), label(std::move(l)), dst(d) {}

    LDRInst::LDRInst(int v, Operand *d) : Inst(Inst::Op::LDR), type(Type::IMM), value(v), dst(d) {}

    STRInst::STRInst(std::string l, Operand *d) : Inst(Inst::Op::STR), type(Type::LABEL), label(std::move(l)), dst(d) {}

    STRInst::STRInst(int v, Operand *d) : Inst(Inst::Op::STR), type(Type::IMM), value(v), dst(d) {}

    ADRInst::ADRInst(Operand *d, std::string lb) : Inst(Inst::Op::ADR), dst(d), label(lb) {}

    Inst2_1::Inst2_1(Op o, Operand *d, int s_imm) : Inst(o), dst(d), type_operand2(Type::Imm) {
        if (Operand::op2Imm(s_imm))
            src = Operand::newImm(s_imm);
        else {
            src = Operand::newVReg();
            src->val = s_imm;
        }
    }

    Inst2_1::Inst2_1(Op o, Operand *d, Operand *s) : Inst(o), dst(d), src(s), type_operand2(Type::Reg) {}

    MOVInst::MOVInst(Operand *d, int s_imm) : Inst2_1(Inst::Op::MOV, d, s_imm) {}

    MOVInst::MOVInst(Operand *d, Operand *s) : Inst2_1(Inst::Op::MOV, d, s) {}

    CMPInst::CMPInst(Operand *d, int s_imm) : Inst2_1(Inst::Op::CMP, d, s_imm) {}

    CMPInst::CMPInst(Operand *d, Operand *s) : Inst2_1(Inst::Op::CMP, d, s) {}

    TSTInst::TSTInst(Operand *d, int s_imm) : Inst2_1(Inst::Op::TST, d, s_imm) {}

    TSTInst::TSTInst(Operand *d, Operand *s) : Inst2_1(Inst::Op::TST, d, s) {}

    BInst::BInst(std::string lb) : Inst(Inst::Op::B), label(lb), has_suffix(false) {}

    BInst::BInst(std::string lb, Type sf) : Inst(Inst::Op::B), label(lb), has_suffix(true), suffix(sf) {}

    BinaryInst::BinaryInst(Op o, Operand *d, Operand *l, Operand *r) : Inst(o), dst(d), lhs(l), rhs(r) {
        if(o == Op::MUL || o == Op::SDIV)
            if(rhs->type == Operand::Type::Imm)
                throw std::runtime_error("IMM not allowed in MUL/SDIV");
    }

    TernaryInst::TernaryInst(Op o, Operand *d, Operand *o1, Operand *o2, Operand *o3) :
            Inst(o), dst(d), op1(o1), op2(o2), op3(o3) {}

    void BasicBlock::insertAtEnd(Inst *inst) {
        insts.push_back(inst);
        inst->bb = this;
    }

    void BasicBlock::markBranch() {
        it_branch = std::prev(insts.end());
    }

    void BasicBlock::insertBeforeBranch(Inst *inst) {
        insts.insert(it_branch, inst);
        inst->bb = this;
    }

    Function::Function(ir::Function *f) : func(f), name(f->name) {
        ret_block = new BasicBlock();
    }

    void Function::appendBlock(BasicBlock *block) {
        bList.push_back(block);
    }

    void Function::appendReturnBlock() {
        bList.push_back(ret_block);
    }
}