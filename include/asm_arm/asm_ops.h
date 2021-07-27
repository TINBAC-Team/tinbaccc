#ifndef TINBACCC_ASM_OPS_H
#define TINBACCC_ASM_OPS_H
namespace asm_arm {
    void allocate_register(asm_arm::Module *module);
    void function_param_pointer_fixup(asm_arm::Module *module);
}
#endif //TINBACCC_ASM_OPS_H
