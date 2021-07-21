#include <ir/ir.h>
#include <asm_arm/builder.h>
#include <asm_arm/instructions.h>
#include <iostream>

namespace ir {
    void Module::codegen(asm_arm::Builder &builder) {
        for (auto &i:functionList)
            i->codegen(builder);
    }

    void Function::codegen(asm_arm::Builder &builder) {
        builder.createFunction(this);
        for (auto &i:bList)
            i->codegen(builder);
        // TODO: generate return instructions
        builder.curFunction->appendReturnBlock();
        // fill incomplete BB pointers
        builder.fillBBPtr();
    }

    void BasicBlock::codegen(asm_arm::Builder &builder) {
        auto asmbb = builder.createBlock();
        builder.setASMBBtoIRBB(this, asmbb);
        for (auto &i:iList)
            i->codegen(builder);
    }

    asm_arm::Operand *Value::codegen(asm_arm::Builder &builder) {
        return nullptr;
    }

    asm_arm::Operand *ConstValue::genimm(asm_arm::Builder &builder) {
        return asm_arm::Operand::newImm(value);
    }

    asm_arm::Operand *ConstValue::genreg(asm_arm::Builder &builder) { //all reg are vreg
        asm_arm::Operand *op = builder.getOperandOfValue(this);
        if (op)
            return op;
        auto inst = builder.createLDR(value);
        op = inst->dst;
        builder.setOperandOfValue(this, op);
        return op;
    }

    asm_arm::Operand* ConstValue::codegen(asm_arm::Builder &builder) {
        return genreg(builder);
    }

    asm_arm::Operand *ConstValue::genop2(asm_arm::Builder &builder) {
        if (asm_arm::Operand::op2Imm(value))
            return genimm(builder);
        return genreg(builder);
    }

    asm_arm::Operand* BinaryInst::codegen(asm_arm::Builder &builder) {
        asm_arm::Operand *lhs, *rhs;
        if(optype == OpType::SREM) {
            // a % b = a-(a/b)*b
            lhs= builder.getOrCreateOperandOfValue(ValueL.value);
            rhs= builder.getOrCreateOperandOfValue(ValueR.value);
            asm_arm::Operand *divres = builder.createBinaryInst(asm_arm::Inst::Op::SDIV, lhs, rhs)->dst;
            asm_arm::Operand *res = builder.createTernaryInst(asm_arm::Inst::Op::MLS, lhs, divres, rhs)->dst;
            builder.setOperandOfValue(this, res);
            return res;
        }

        asm_arm::Inst::Op op;
        bool lhs_const = ValueL.value->optype==OpType::CONST;
        bool rhs_const = ValueR.value->optype==OpType::CONST;

        switch (optype) {
            case OpType::ADD:
                op = asm_arm::Inst::Op::ADD;
                lhs = builder.getOrCreateOperandOfValue(ValueL.value);
                if (rhs_const)
                    rhs = dynamic_cast<ConstValue *>(ValueR.value)->genop2(builder);
                else
                    rhs = builder.getOrCreateOperandOfValue(ValueR.value);
                break;
            case OpType::SUB:
                op = asm_arm::Inst::Op::SUB;
                if (lhs_const) {
                    auto r_const_val = dynamic_cast<ConstValue *>(ValueR.value);
                    if (asm_arm::Operand::op2Imm(r_const_val->value)) {
                        op = asm_arm::Inst::Op::RSB;
                        lhs = r_const_val->genimm(builder);
                        rhs = builder.getOrCreateOperandOfValue(ValueL.value);
                        break;
                    }
                }
                lhs = builder.getOrCreateOperandOfValue(ValueL.value);
                if (rhs_const)
                    rhs = dynamic_cast<ConstValue *>(ValueR.value)->genop2(builder);
                else
                    rhs = builder.getOrCreateOperandOfValue(ValueR.value);
                break;
            case OpType::MUL:
                op=asm_arm::Inst::Op::MUL;
                lhs= builder.getOrCreateOperandOfValue(ValueL.value);
                rhs= builder.getOrCreateOperandOfValue(ValueR.value);
                break;
            case OpType::SDIV:
                op=asm_arm::Inst::Op::SDIV;
                lhs= builder.getOrCreateOperandOfValue(ValueL.value);
                rhs= builder.getOrCreateOperandOfValue(ValueR.value);
                break;
            case OpType::EQ:
            case OpType::NE:
            case OpType::SLT:
            case OpType::SLE:
            case OpType::SGT:
            case OpType::SGE:
                // result of conditional ops are used in branch instruction. Create it there.
                return nullptr;
            default:
                throw std::runtime_error("not implemented yet.");
        }
        asm_arm::Operand *res = builder.createBinaryInst(op, lhs, rhs)->dst;
        builder.setOperandOfValue(this, res);
        return res;
    }

    asm_arm::Operand * PhiInst::codegen(asm_arm::Builder &builder) {
        // TODO: implement it
        throw std::runtime_error("not implemented");
    }

    asm_arm::Operand *CallInst::codegen(asm_arm::Builder &builder) {
        asm_arm::Operand *ret = nullptr;
        // FIXME: SP should be 8-byte aligned
        if (params.size() > 4)
            builder.moveSP(true, (params.size() - 4) * 4);
        // setup parameters
        for (int i = params.size() - 1; i >= 0; i--) {
            if (i >= 4) {
                builder.createSTR(
                        builder.getOrCreateOperandOfValue(params[i]),
                        asm_arm::Operand::getReg(asm_arm::Reg::sp),
                        asm_arm::Operand::newImm((i - 4) * 4)
                );
            } else {
                builder.createMOVInst(
                        asm_arm::Operand::getReg(static_cast<asm_arm::Reg>(i)),
                        builder.getOrCreateOperandOfValue(params[i])
                );
            }
        }
        // call the function
        builder.createCall(fname, params.size(), is_void);
        if (!is_void) {
            builder.createMOVInst(
                    (ret = asm_arm::Operand::newVReg()),
                    asm_arm::Operand::getReg(asm_arm::Reg::r0)
            );
            builder.setOperandOfValue(this, ret);
        }
        // restore sp
        if (params.size() > 4)
            builder.moveSP(false, (params.size() - 4) * 4);
        return ret;
    }

    asm_arm::Operand *BranchInst::codegen(asm_arm::Builder &builder) {
        asm_arm::Operand *lhs = nullptr, *rhs = nullptr;
        // Grab its cond value to generate cmp instruction
        asm_arm::Inst::OpCond asmcond;
        switch (cond.value->optype) {
            case OpType::EQ:
                asmcond = asm_arm::Inst::OpCond::EQ;
                break;
            case OpType::NE:
                asmcond = asm_arm::Inst::OpCond::NE;
                break;
            case OpType::SLT:
                asmcond = asm_arm::Inst::OpCond::LT;
                break;
            case OpType::SLE:
                asmcond = asm_arm::Inst::OpCond::LE;
                break;
            case OpType::SGT:
                asmcond = asm_arm::Inst::OpCond::GT;
                break;
            case OpType::SGE:
                asmcond = asm_arm::Inst::OpCond::GE;
                break;
            default:
                asmcond = asm_arm::Inst::OpCond::NE;
                lhs = builder.getOrCreateOperandOfValue(cond.value);
                rhs = asm_arm::Operand::newImm(0);
                break;
        }
        if(!lhs) {
            auto binop = dynamic_cast<BinaryInst*>(cond.value);
            lhs = builder.getOrCreateOperandOfValue(binop->ValueL.value);
            if (binop->ValueR.value->optype==OpType::CONST)
                rhs = dynamic_cast<ConstValue *>(binop->ValueR.value)->genop2(builder);
            else
                rhs = builder.getOrCreateOperandOfValue(binop->ValueR.value);
        }
        builder.createCMPInst(lhs, rhs);
        builder.curBlock->markBranch();
        builder.createBInst(true_block, asmcond);
        // XXX: This kind-of breaks the concept of "BasicBlock"...
        builder.createBInst(false_block);
        builder.createBlock();
        return nullptr;
    }

    asm_arm::Operand * JumpInst::codegen(asm_arm::Builder &builder) {
        builder.createBInst(to);
        builder.curBlock->markBranch();
        builder.createBlock();
        return nullptr;
    }

    asm_arm::Operand *ReturnInst::codegen(asm_arm::Builder &builder) {
        if (val.value)
            builder.createMOVInst(
                    asm_arm::Operand::getReg(asm_arm::Reg::r0),
                    builder.getOrCreateOperandOfValue(val.value)
            );
        builder.createBInst(builder.curFunction->ret_block);
        builder.createBlock();
        return nullptr;
    }

    asm_arm::Operand * AccessInst::genptr(asm_arm::Builder &builder, Value *val) {
        if(val->optype==OpType::GLOBAL) {
            auto valglobal = dynamic_cast<GlobalVar*>(val);
            auto ldrinst = builder.createLDR(valglobal->name);
            return ldrinst->dst;
        } else {
            return builder.getOrCreateOperandOfValue(val);
        }
    }

    asm_arm::Operand * LoadInst::codegen(asm_arm::Builder &builder) {
        auto op = genptr(builder, ptr.value);
        // TODO: Handle offset
        auto inst = builder.createLDR(op, asm_arm::Operand::newImm(0));
        return inst->dst;
    }

    asm_arm::Operand * StoreInst::codegen(asm_arm::Builder &builder) {
        auto addrop = genptr(builder, ptr.value);
        auto valop = builder.getOrCreateOperandOfValue(val.value);
        // TODO: Handle offset
        builder.createSTR(addrop, valop, asm_arm::Operand::newImm(0));
        return nullptr;
    }

    asm_arm::Operand * AllocaInst::codegen(asm_arm::Builder &builder) {
        auto ret = builder.allocate_stack(size);
        builder.setOperandOfValue(this, ret);
    }

    asm_arm::Operand * GetElementPtrInst::codegen(asm_arm::Builder &builder) {
        // TODO: offset...
        auto mul4 = builder.createLDR(4);
        auto ptr = AccessInst::genptr(builder, arr.value);
        auto res = builder.createTernaryInst(asm_arm::Inst::Op::MLA, ptr, builder.getOrCreateOperandOfValue(offset.value), mul4->dst);
        builder.setOperandOfValue(this, res->dst);
        return res->dst;
    }
}
