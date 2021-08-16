#include <ir/ir.h>
#include <asm_arm/builder.h>
#include <iostream>
#include <ir/vectorization.h>

namespace ir {
    asm_arm::Operand *AdjacentMemory::codegen(asm_arm::Builder &builder) {
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
                if (pow == 0) {
                    offs_reg = dim_val;
                } else {
                    auto res = builder.createLSL(dim_val, pow);
                    offs_reg = res->dst;
                }
            } else {
                auto dim_mul = builder.createLDR(multipliers[i])->dst;
                auto res = builder.createBinaryInst(asm_arm::Inst::Op::MUL, dim_mul, dim_val);
                offs_reg = res->dst;
            }
        }
        std::unique_ptr<asm_arm::RegOffs> new_ro;
        if (offs_reg)
            new_ro = std::make_unique<asm_arm::RegOffs>(base, offs_reg);
        else
            new_ro = std::make_unique<asm_arm::RegOffs>(base, offs_const);

        auto val = builder.genValueFromRegOffs(new_ro.get());
        builder.setOperandOfValue(this, val);
        return val;
    }

    asm_arm::Operand *VDupInst::codegen(asm_arm::Builder &builder) {
        asm_arm::Inst *inst = nullptr;
        asm_arm::SIMDQReg dst = builder.getSIMDReg();
        if (value->optype == OpType::CONST) {
            int val_int = dynamic_cast<ConstValue *>(value)->value;
            if (asm_arm::VMOVInst::vmov_operand(val_int))
                inst = new asm_arm::VMOVInst(dst, val_int);
        }
        if (!inst)
            inst = new asm_arm::VDUPInst(dst, builder.getOrCreateOperandOfValue(value));
        builder.insertInst(inst);
        builder.setSIMDRegOfValue(this, dst);
        return nullptr;
    }

    asm_arm::Operand *VBinaryInst::codegen(asm_arm::Builder &builder) {
        asm_arm::SIMDQReg dst = builder.getSIMDReg();
        asm_arm::Inst::Op op;
        switch (optype) {
            case OpType::ADD:
                op = asm_arm::Inst::Op::VADD;
                break;
            case OpType::SUB:
                op = asm_arm::Inst::Op::VSUB;
                break;
            case OpType::MUL:
                op = asm_arm::Inst::Op::VMUL;
                break;
            default:
                throw std::runtime_error("unsupported ASIMD op");
        }
        builder.insertInst(new asm_arm::VBinaryInst(op, dst, builder.getSIMDRegOfValue(ValueL.value),
                                                    builder.getSIMDRegOfValue(ValueR.value)));
        builder.setSIMDRegOfValue(this, dst);
        return nullptr;
    }

    asm_arm::Operand *VLoadInst::codegen(asm_arm::Builder &builder) {
        asm_arm::SIMDQReg dst = builder.getSIMDReg();
        builder.insertInst(new asm_arm::VLDRInst(dst, builder.getOrCreateOperandOfValue(ptr.value)));
        builder.setSIMDRegOfValue(this, dst);
        return nullptr;
    }

    asm_arm::Operand *VStoreInst::codegen(asm_arm::Builder &builder) {
        builder.insertInst(new asm_arm::VSTRInst(builder.getSIMDRegOfValue(val.value),
                                                 builder.getOrCreateOperandOfValue(ptr.value)));
        return nullptr;
    }
}
