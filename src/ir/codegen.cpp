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

    Inst * Value::codegen(asm_arm::Builder &builder) {
        return nullptr;
    }
}