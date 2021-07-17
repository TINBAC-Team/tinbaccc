#include <ir/ir.h>
#include <asm_arm/builder.h>
#include <asm_arm/instructions.h>

namespace ir {
    void Module::codegen(asm_arm::Builder &builder) {
        for (auto i:functionList)
            i->codegen(builder);
    }

    void Function::codegen(asm_arm::Builder &builder) {
    }

    asm_arm::Operand *Value::codegen(asm_arm::Builder &builder) {
        return nullptr;
    }

    asm_arm::Operand *ConstValue::codegen(asm_arm::Builder &builder) {
        return asm_arm::Operand::newImm(value);
    }

    asm_arm::Operand *ConstValue::genreg(asm_arm::Builder &builder) {
        builder.createLDR(value);
        return nullptr;//TODO: REGISTER!!!
    }

    asm_arm::Operand *ConstValue::genop2(asm_arm::Builder &builder) {
        if (asm_arm::Operand::op2Imm(value))
            return (codegen(builder));
        return genreg(builder);
    }
}