#include <asm_arm/instructions.h>
#include <ir/ir.h>

#include <utility>
#include <stdexcept>
#include <algorithm>

namespace asm_arm {
    Operand *Operand::precolored_reg_map[static_cast<int>(Reg::MAX)] = {};

    Operand *Operand::newImm(int v) {
        auto ret = new Operand(Type::Imm);
        ret->val = v;
        return ret;
    }

    Operand *Operand::getReg(Reg r) {
        if (precolored_reg_map[static_cast<int>(r)])
            return precolored_reg_map[static_cast<int>(r)];
        auto ret = new Operand(Type::Reg);
        ret->reg = r;
        precolored_reg_map[static_cast<int>(r)] = ret;
        return ret;
    }

    void Operand::resetRegMap() {
        for (auto &i:precolored_reg_map) {
            if (i) {
                delete i;
                i = nullptr;
            }
        }
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



    void Inst::add_use(Operand *op) {
        use.insert(op);
    }

    void Inst::add_def(Operand *op) {
        def.insert(op);
    }

    bool Inst::replace_use(Operand *orig, Operand *newop) {
        return false;
    }

    bool Inst::replace_def(Operand *orig, Operand *newop) {
        return false;
    }


    LDRInst::LDRInst(std::string l, Operand *d) : Inst(Inst::Op::LDR), type(Type::LABEL), label(std::move(l)), dst(d) {
        add_def(d);
    }

    LDRInst::LDRInst(int v, Operand *d) : Inst(Inst::Op::LDR), type(Type::IMM), value(v), dst(d) {
        add_def(d);
    }

    LDRInst::LDRInst(Operand *d, Operand *s, Operand *o) : Inst(Inst::Op::LDR), type(Type::REGOFFS), dst(d), src(s),
                                                           offs(o) {
        add_use(s);
        add_use(o);
        add_def(d);
    }

    bool LDRInst::replace_def(Operand *orig, Operand *newop) {
        if (dst != orig)
            return false;
        dst = newop;
        def.erase(orig);
        add_def(newop);
        return true;
    }

    bool LDRInst::replace_use(Operand *orig, Operand *newop) {
        if (type != Type::REGOFFS)
            return false;
        if (use.find(orig) == use.end())
            return false;
        if (src == orig)
            src = newop;
        if (offs == orig)
            offs = newop;
        use.erase(orig);
        add_use(newop);
        return true;
    }

    STRInst::STRInst(Operand *v, Operand *a, Operand *o) : Inst(Inst::Op::STR), val(v), addr(a), offset(o) {
        add_use(v);
        add_use(a);
        add_use(o);
    }

    bool STRInst::replace_use(Operand *orig, Operand *newop) {
        if (use.find(orig) == use.end())
            return false;
        if (val == orig)
            val = newop;
        if (addr == orig)
            addr = newop;
        if (offset == orig)
            offset = newop;
        use.erase(orig);
        add_use(newop);
        return true;
    }

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

    MOVInst::MOVInst(Operand *d, Operand *s) : Inst(Inst::Op::MOV), dst(d), src(s) {
        add_use(s);
        add_def(d);
    }

    bool MOVInst::replace_def(Operand *orig, Operand *newop) {
        if (dst != orig)
            return false;
        dst = newop;
        def.erase(orig);
        add_def(newop);
        return true;
    }

    bool MOVInst::replace_use(Operand *orig, Operand *newop) {
        if (src != orig)
            return false;
        src = newop;
        use.erase(orig);
        add_use(newop);
        return true;
    }

    CMPInst::CMPInst(Operand *l, Operand *r) : Inst(Inst::Op::CMP), lhs(l), rhs(r) {
        add_use(l);
        add_use(r);
    }

    bool CMPInst::replace_use(Operand *orig, Operand *newop) {
        if (use.find(orig) == use.end())
            return false;
        if (lhs == orig)
            lhs = newop;
        if (rhs == orig)
            rhs = newop;
        use.erase(orig);
        add_use(newop);
        return true;
    }

    TSTInst::TSTInst(Operand *d, int s_imm) : Inst2_1(Inst::Op::TST, d, s_imm) {}

    TSTInst::TSTInst(Operand *d, Operand *s) : Inst2_1(Inst::Op::TST, d, s) {}

    BInst::BInst(OpCond c) : Inst(Op::B, c) {}

    CallInst::CallInst(int np, std::string l, bool _is_void) :
            Inst(Inst::Op::BL), nparams(np), label(std::move(l)), is_void(_is_void) {

        // r0-r3 are caller-preserved regs. set unused ones to def for register allocation.
        if (np >= 1)
            add_use(Operand::getReg(Reg::r0));
        if (np >= 2)
            add_use(Operand::getReg(Reg::r1));
        if (np >= 3)
            add_use(Operand::getReg(Reg::r2));
        if (np >= 4)
            add_use(Operand::getReg(Reg::r3));
        add_def(Operand::getReg(Reg::r0));
        add_def(Operand::getReg(Reg::r1));
        add_def(Operand::getReg(Reg::r2));
        add_def(Operand::getReg(Reg::r3));
        add_def(Operand::getReg(Reg::r12));
        add_def(Operand::getReg(Reg::lr));
    }

    bool CallInst::replace_use(Operand *orig, Operand *newop) {
        if (use.find(orig) == use.end())
            return false;
        throw std::runtime_error("Call shouldn't be spilled?");
    }

    bool CallInst::replace_def(Operand *orig, Operand *newop) {
        if (def.find(orig) == def.end())
            return false;
        throw std::runtime_error("Call shouldn't be spilled?");
    }

    BinaryInst::BinaryInst(Op o, Operand *d, Operand *l, Operand *r) : Inst(o), dst(d), lhs(l), rhs(r) {
        if (o == Op::MUL || o == Op::SDIV)
            if (rhs->type == Operand::Type::Imm)
                throw std::runtime_error("IMM not allowed in MUL/SDIV");

        add_use(l);
        add_use(r);
        add_def(d);
    }

    bool BinaryInst::replace_use(Operand *orig, Operand *newop) {
        if (use.find(orig) == use.end())
            return false;
        if (lhs == orig)
            lhs = newop;
        if (rhs == orig)
            rhs = newop;
        use.erase(orig);
        add_use(newop);
        return true;
    }

    bool BinaryInst::replace_def(Operand *orig, Operand *newop) {
        if (dst != orig)
            return false;
        dst = newop;
        def.erase(orig);
        add_def(newop);
        return true;
    }

    TernaryInst::TernaryInst(Op o, Operand *d, Operand *o1, Operand *o2, Operand *o3) :
            Inst(o), dst(d), op1(o1), op2(o2), op3(o3) {
        add_use(o1);
        add_use(o2);
        add_use(o3);
        add_def(d);
    }

    bool TernaryInst::replace_def(Operand *orig, Operand *newop) {
        if (dst != orig)
            return false;
        dst = newop;
        def.erase(orig);
        add_def(newop);
        return true;
    }

    bool TernaryInst::replace_use(Operand *orig, Operand *newop) {
        if (use.find(orig) == use.end())
            return false;
        if (op1 == orig)
            op1 = newop;
        if (op2 == orig)
            op2 = newop;
        if (op3 == orig)
            op3 = newop;
        use.erase(orig);
        add_use(newop);
        return true;
    }

    ReturnInst::ReturnInst(bool ret) : Inst(Op::RETURN), has_return_value(ret) {
        if (ret)
            add_use(Operand::getReg(Reg::r0));
    }

    bool ReturnInst::replace_use(Operand *orig, Operand *newop) {
        if (use.find(orig) == use.end())
            return false;
        throw std::runtime_error("Call shouldn't be spilled?");
    }

    static int bb_seed2 = 0;
    BasicBlock::BasicBlock() {
        bb_label = ".L";
        bb_label += std::to_string(bb_seed2++);
    }

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

    void BasicBlock::insertBefore(Inst *inst, Inst *before) {
        if (*it_insert != before) {
            it_insert = std::find(insts.begin(), insts.end(), before);
            if (it_insert == insts.end())
                throw std::runtime_error("instruction not found");
        }
        insts.insert(it_insert, inst);
    }

    Function::Function(ir::Function *f) : func(f), name(f->name), stack_size(0) {
        ret_block = new BasicBlock();
    }

    void Function::appendBlock(BasicBlock *block) {
        bList.push_back(block);
    }

    void Function::appendReturnBlock() {
        bList.push_back(ret_block);
    }

    unsigned int Function::allocate_stack(unsigned int ni32s) {
        unsigned int retoffs = stack_size;
        stack_size += ni32s * 4;
        return retoffs;
    }
}
