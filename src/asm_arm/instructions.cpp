#include <asm_arm/instructions.h>
#include <ir/ir.h>

#include <utility>

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