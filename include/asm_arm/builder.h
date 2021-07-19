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

        STRInst * createSTR(std::string& s);

        STRInst * createSTR(int v);

        ADRInst * createADR(std::string& lb);

        template<typename RT> // 使用时需要显示指定类型！(?)
        RT * createInst2_1(Inst::Op o, Operand *d, int s_imm);

        template<typename RT>
        RT * createInst2_1_(Inst::Op o, Operand *d, Operand *s);

        BInst * createBInst(std::string& lb);

        BInst * createBInst(std::string& lb, BInst::Type sf);

        template<typename RT>
        RT * createInst1(Inst::Op o, Operand *d, Operand *s1, int s2_imm);

        template<typename RT>
        RT * createInst1_(Inst::Op o, Operand *d, Operand *s1, Operand *s2);

        template<typename RT>
        RT * createInst2(Inst::Op o, Operand *d, Operand *s1, Operand *s2);

        LABELofInst * createLABELofInst(std::string& lb);
    };
}
#endif //TINBACCC_BUILDER_H
