#ifndef TINBACCC_ASM_BUILDER_H
#define TINBACCC_ASM_BUILDER_H

#include <ir/ir.h>
#include <asm_arm/instructions.h>

namespace asm_arm {
    class Builder {
    public:
        Module *module;
        Function *curFunction;
        BasicBlock *curBlock;

        Builder(Module *m);

        Function *createFunction(ir::Function *f);

        BasicBlock *createBlock();

        LDRInst *createLDR(std::string s);

        LDRInst *createLDR(int v);
    };
}
#endif //TINBACCC_BUILDER_H
