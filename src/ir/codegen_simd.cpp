#include <ir/ir.h>
#include <asm_arm/builder.h>
#include <iostream>
#include <ir/vectorization.h>

namespace ir {
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
