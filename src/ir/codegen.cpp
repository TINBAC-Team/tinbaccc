#include <ir/ir.h>
#include <asm_arm/builder.h>
#include <asm_arm/instructions.h>
#include <iostream>

namespace ir {
    void Module::codegen(asm_arm::Builder &builder) {
        builder.setIRModule(this);
        for (auto &i:functionList)
            i->codegen(builder);
    }

    void Function::codegen(asm_arm::Builder &builder) {
        // don't generate code for external functions
        if(bList.empty())
            return;
        builder.createFunction(this);
        for (auto &i:bList)
            i->codegen(builder);
        builder.curFunction->appendReturnBlock();
        builder.curBlock = builder.curFunction->ret_block;
        auto ret = new asm_arm::ReturnInst(return_int);
        builder.curBlock->insertAtEnd(ret);
        // fill incomplete BB pointers
        builder.fillBBPtr();
        builder.fillMOV();
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
            asm_arm::Operand *res = builder.createTernaryInst(asm_arm::Inst::Op::MLS, divres, rhs, lhs)->dst;
            builder.setOperandOfValue(this, res);
            return res;
        }

        asm_arm::Inst::Op op;
        bool lhs_const = ValueL.value->optype==OpType::CONST;
        bool rhs_const = ValueR.value->optype==OpType::CONST;

        switch (optype) {
            case OpType::ADD:
                op = asm_arm::Inst::Op::ADD;
                /* fall through */
            case OpType::SUB:
                if (optype == OpType::SUB)
                    op = asm_arm::Inst::Op::SUB;
                if (lhs_const) {
                    auto l_const_val = dynamic_cast<ConstValue *>(ValueL.value);
                    if (asm_arm::Operand::op2Imm(l_const_val->value)) {
                        if (optype == OpType::SUB)
                            op = asm_arm::Inst::Op::RSB;
                        rhs = l_const_val->genimm(builder);
                        lhs = builder.getOrCreateOperandOfValue(ValueR.value);
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

    asm_arm::Operand *BinaryInst::codegen_value(asm_arm::Builder &builder) {
        asm_arm::Operand *lhs = nullptr, *rhs = nullptr;
        asm_arm::Inst::OpCond asmcond;
        if(builder.is_OpCond(optype))
        {
            asmcond = builder.toOpCond(optype);
        } else
        {
            return this->codegen(builder);
        }
        lhs = builder.getOrCreateOperandOfValue(ValueL.value);
        if (ValueR.value->optype==OpType::CONST)
            rhs = dynamic_cast<ConstValue *>(ValueR.value)->genop2(builder);
        else
            rhs = builder.getOrCreateOperandOfValue(ValueR.value);
        builder.createCMPInst(lhs,rhs);
        auto ret = builder.createLDR(0);
        auto meet_cond = builder.createLDR(1,ret->dst);
        meet_cond->cond = asmcond;
        return ret->dst;

    }

    asm_arm::Operand * PhiInst::codegen(asm_arm::Builder &builder) {
        auto srcreg = asm_arm::Operand::newVReg();
        auto dstmov = builder.createMOVInst(asm_arm::Operand::newVReg(), srcreg);
        for (auto &i:phicont)
            builder.addPendingMOV(i.first, i.second->value, srcreg);
        builder.setOperandOfValue(this, dstmov->dst);
        return dstmov->dst;
    }

    asm_arm::Operand *CallInst::codegen(asm_arm::Builder &builder) {
        asm_arm::Operand *ret = nullptr;
        // SP should be 8-byte aligned
        int sp_move_len = 0;
        if (params.size() > 4) {
            sp_move_len = params.size() - 4;
            if (sp_move_len & 1)
                sp_move_len++;
            sp_move_len *= 4;
        }
        if (sp_move_len)
            builder.moveSP(true, sp_move_len);
        // setup parameters
        for (int i = params.size() - 1; i >= 0; i--) {
            if (i >= 4) {
                builder.createSTR(
                        builder.getOrCreateOperandOfValue(params[i].value),
                        asm_arm::Operand::getReg(asm_arm::Reg::sp),
                        asm_arm::Operand::newImm((i - 4) * 4)
                );
            } else {
                builder.createMOVInst(
                        asm_arm::Operand::getReg(static_cast<asm_arm::Reg>(i)),
                        builder.getOrCreateOperandOfValue(params[i].value)
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
        if (sp_move_len)
            builder.moveSP(false, sp_move_len);
        return ret;
    }

    asm_arm::Operand *BranchInst::codegen(asm_arm::Builder &builder) {
        asm_arm::Operand *lhs = nullptr, *rhs = nullptr;
        // Grab its cond value to generate cmp instruction
        asm_arm::Inst::OpCond asmcond;
        if(builder.is_OpCond(cond.value->optype))
        {
            asmcond = builder.toOpCond(cond.value->optype);
        } else
        {
            asmcond = asm_arm::Inst::OpCond::NE;
            lhs = builder.getOrCreateOperandOfValue(cond.value);
            rhs = asm_arm::Operand::newImm(0);
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
        builder.curBlock = nullptr;
        return nullptr;
    }

    asm_arm::Operand * JumpInst::codegen(asm_arm::Builder &builder) {
        builder.createBInst(to);
        builder.curBlock->markBranch();
        builder.curBlock = nullptr;
        return nullptr;
    }

    asm_arm::Operand *ReturnInst::codegen(asm_arm::Builder &builder) {
        if (val.value)
            builder.createMOVInst(
                    asm_arm::Operand::getReg(asm_arm::Reg::r0),
                    builder.getOrCreateOperandOfValue(val.value)
            );
        builder.createBInst(builder.curFunction->ret_block);
        builder.curBlock = nullptr;
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
        builder.setOperandOfValue(this, inst->dst);
        return inst->dst;
    }

    asm_arm::Operand * StoreInst::codegen(asm_arm::Builder &builder) {
        auto addrop = genptr(builder, ptr.value);
        auto valop = builder.getOrCreateOperandOfValue(val.value);
        // TODO: Handle offset
        builder.createSTR(valop, addrop, asm_arm::Operand::newImm(0));
        return nullptr;
    }

    asm_arm::Operand * AllocaInst::codegen(asm_arm::Builder &builder) {
        auto ret = builder.allocate_stack(size);
        builder.setOperandOfValue(this, ret);
        return ret;
    }

    asm_arm::Operand * GetElementPtrInst::codegen(asm_arm::Builder &builder) {
        // TODO: offset...
        auto mul4 = builder.createLDR(4);
        auto ptr = AccessInst::genptr(builder, arr.value);
        auto res = builder.createTernaryInst(asm_arm::Inst::Op::MLA, builder.getOrCreateOperandOfValue(offset.value), mul4->dst, ptr);
        builder.setOperandOfValue(this, res->dst);
        return res->dst;
    }




}
