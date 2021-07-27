#include <asm_arm/instructions.h>

namespace asm_arm {
    void function_param_pointer_fixup(asm_arm::Module *module) {
        for (auto &func:module->functionList) {
            int stack_movement = func->stack_size;
            // lr
            stack_movement += 4;
            if (func->max_reg >= 12)
                stack_movement += 8 * 4;
            else if (func->max_reg >= 4)
                stack_movement += (func->max_reg - 4 + 1) * 4;

            for (auto &param:func->param_fixup_list)
                param->val += stack_movement;
        }
    }
}