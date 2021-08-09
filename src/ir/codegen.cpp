#include <ir/ir.h>
#include <asm_arm/builder.h>
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
        auto asmbb = builder.createBlock(this->loop_depth);
        asmbb->comment << "ir: " << name;
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

    /**
     * check if a Value is a constant of pow(2,n)
     * @param val pointer to Value*
     * @return -1 if it isn't. otherwise return n
     */
    static int isValPow2(Value *val) {
        auto constVal = dynamic_cast<ConstValue *>(val);
        if (!constVal)
            return -1;
        if (constVal->value <= 0)
            return -1;
        int pow = __builtin_ffs(constVal->value) - 1;
        return (1 << pow == constVal->value) ? pow : -1;
    }

    asm_arm::Operand* BinaryInst::codegen(asm_arm::Builder &builder) {
        asm_arm::Operand *lhs, *rhs;
        if(optype == OpType::SREM)
            return codegen_mod(builder);
        if(optype == OpType::MUL)
            return codegen_mul(builder);

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
        meet_cond->add_use(ret->dst);
        return ret->dst;

    }

    asm_arm::Operand *BinaryInst::codegen_mod(asm_arm::Builder &builder) {
        asm_arm::Operand *lhs, *rhs;
        // a % b = a-(a/b)*b
        lhs = builder.getOrCreateOperandOfValue(ValueL.value);
        // a % 2^n can be converted to a & ((1<<n)-1)
        int pow2 = isValPow2(ValueR.value);
        if (pow2 >= 0) {
            int and_val = ((1 << pow2) - 1);
            if (asm_arm::Operand::op2Imm(and_val))
                rhs = asm_arm::Operand::newImm(and_val);
            else
                rhs = builder.createLDR(and_val)->dst;
            // To deal with negative numbers, if a is negative, we need res = res - 2^n if res > 0
            auto andInst = builder.createBinaryInst(asm_arm::Inst::Op::AND, lhs, rhs);
            asm_arm::Operand *res = andInst->dst;
            // set flags to test for res==0
            andInst->set_flags = true;
            // Only execute if the AND result is positive.
            asm_arm::Inst *negInst =builder.createCMPInst(lhs, asm_arm::Operand::newImm(0));
            // For flag Z == 0. It has nothing to do with a "not equal" test.
            negInst->cond=asm_arm::Inst::OpCond::NE;
            // The result of the previous AND can't be negative so flag N can only be set by CMP.
            auto subInst = new asm_arm::BinaryInst(asm_arm::Inst::Op::SUB, res, res, asm_arm::Operand::newImm((1 << pow2)));
            subInst->cond=asm_arm::Inst::OpCond::MI;
            subInst->add_use(res);
            builder.curBlock->insertAtEnd(subInst);
            builder.setOperandOfValue(this, res);
            return res;
        }
        rhs = builder.getOrCreateOperandOfValue(ValueR.value);
        asm_arm::Operand *divres = builder.createBinaryInst(asm_arm::Inst::Op::SDIV, lhs, rhs)->dst;
        asm_arm::Operand *res = builder.createTernaryInst(asm_arm::Inst::Op::MLS, divres, rhs, lhs)->dst;
        builder.setOperandOfValue(this, res);
        return res;
    }

    asm_arm::Operand *BinaryInst::codegen_mul(asm_arm::Builder &builder) {
        asm_arm::Operand *lhs = builder.getOrCreateOperandOfValue(ValueL.value);
        asm_arm::Operand *res;
        int pow2 = isValPow2(ValueR.value);
        if (pow2 >= 0) {
            res = builder.createLSL(lhs, pow2)->dst;
        } else {
            asm_arm::Inst::Op op = asm_arm::Inst::Op::MUL;
            asm_arm::Operand *rhs = builder.getOrCreateOperandOfValue(ValueR.value);
            auto mulInst = builder.createBinaryInst(op, lhs, rhs);
            mulInst->dst->inst = mulInst;
            res = mulInst->dst;

        }
        builder.setOperandOfValue(this, res);
        return res;
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
        asm_arm::LDRInst *inst;
        auto ro = builder.getRegOffsOfValue(ptr.value);
        if (ro) {
            asm_arm::Operand *offs;
            if (ro->offs)
                offs = ro->offs;
            else if (ro->const_offs < 1024)
                offs = asm_arm::Operand::newImm(ro->const_offs * 4);
            else
                offs = builder.createLDR(ro->const_offs * 4)->dst;
            inst = builder.createLDR(ro->reg, offs);
            if (ro->offs)
                inst->lsl = 2;
        } else {
            auto op = genptr(builder, ptr.value);
            inst = builder.createLDR(op, asm_arm::Operand::newImm(0));
        }
        builder.setOperandOfValue(this, inst->dst);
        return inst->dst;
    }

    asm_arm::Operand * StoreInst::codegen(asm_arm::Builder &builder) {
        auto valop = builder.getOrCreateOperandOfValue(val.value);
        auto ro = builder.getRegOffsOfValue(ptr.value);
        if (ro) {
            asm_arm::Operand *offs;
            if (ro->offs)
                offs = ro->offs;
            else if (ro->const_offs < 1024)
                offs = asm_arm::Operand::newImm(ro->const_offs * 4);
            else
                offs = builder.createLDR(ro->const_offs * 4)->dst;
            auto inst = builder.createSTR(valop, ro->reg, offs);
            if (ro->offs)
                inst->lsl = 2;
        } else {
            auto addrop = genptr(builder, ptr.value);
            builder.createSTR(valop, addrop, asm_arm::Operand::newImm(0));
        }
        return nullptr;
    }

    asm_arm::Operand * AllocaInst::codegen(asm_arm::Builder &builder) {
        auto ret = builder.allocate_stack(size);
        builder.setOperandOfValue(this, ret);
        return ret;
    }

    asm_arm::Operand *GetElementPtrInst::codegen(asm_arm::Builder &builder) {
        auto ro = builder.getRegOffsOfValue(arr.value);
        asm_arm::Operand *base;
        asm_arm::Operand *offs_reg;
        int offs_const;
        if (ro) {
            base = ro->reg;
            offs_reg = ro->offs;
            offs_const = ro->const_offs;
        } else {
            base = AccessInst::genptr(builder, arr.value);
            offs_const = 0;
            offs_reg = nullptr;
        }
        for (int i = 0; i < dims.size(); i++) {
            // skip zero offset
            auto val_const = dynamic_cast<ConstValue *>(dims[i].value);
            if (val_const) {
                if (!val_const->value)
                    continue;
                if (!offs_reg) {
                    offs_const += multipliers[i] * val_const->value;
                    continue;
                }
            }
            int pow = __builtin_ffs(multipliers[i]) - 1;
            auto dim_val = builder.getOrCreateOperandOfValue(dims[i].value);
            if (!offs_reg && offs_const)
                offs_reg = builder.createLDR(offs_const)->dst;
            if (offs_reg) {
                if (1 << pow == multipliers[i]) {
                    auto res = builder.createBinaryInst(asm_arm::Inst::Op::ADD, offs_reg, dim_val);
                    res->lsl = pow;
                    offs_reg = res->dst;
                } else {
                    auto dim_mul = builder.createLDR(multipliers[i])->dst;
                    auto res = builder.createTernaryInst(asm_arm::Inst::Op::MLA, dim_mul, dim_val, offs_reg);
                    offs_reg = res->dst;
                }
            } else if (1 << pow == multipliers[i]) {
                auto res = builder.createLSL(dim_val, pow);
                offs_reg = res->dst;
            } else {
                auto dim_mul = builder.createLDR(multipliers[i])->dst;
                auto res = builder.createBinaryInst(asm_arm::Inst::Op::MUL, dim_mul, dim_val);
                offs_reg = res->dst;
            }
        }
        if (offs_reg)
            builder.setRegOffsOfValue(this, std::make_unique<asm_arm::RegOffs>(base, offs_reg));
        else
            builder.setRegOffsOfValue(this, std::make_unique<asm_arm::RegOffs>(base, offs_const));
        return nullptr;
    }
}
