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
}