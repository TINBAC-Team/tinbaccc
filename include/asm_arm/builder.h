#ifndef TINBACCC_ASM_BUILDER_H
#define TINBACCC_ASM_BUILDER_H

#include <ir/ir.h>
#include <asm_arm/instructions.h>
#include <unordered_map>

namespace asm_arm {
    class Builder {
    public:
        Module *module;
        Function *curFunction;
        BasicBlock *curBlock;
        std::unordered_map<ir::Value*, Operand*> value_map;

        Builder(Module *m);

        Operand *getOperandOfValue(ir::Value* val);

        Operand *getOrCreateOperandOfValue(ir::Value* val);

        void setOperandOfValue(ir::Value* val, Operand *operand);

        Function *createFunction(ir::Function *f);

        BasicBlock *createBlock();

        LDRInst *createLDR(std::string s);

        LDRInst *createLDR(int v);

        STRInst * createSTR(std::string& s);

        STRInst * createSTR(int v);

        ADRInst * createADR(std::string& lb);

        BInst * createBInst(std::string& lb);

        BInst * createBInst(std::string& lb, BInst::Type sf);

        /**
         * Create binary instruction with lhs and rhs as operand.
         * Destination operand is a virtual register.
         * @param op
         * @param lhs
         * @param rhs
         * @return created instruction
         */
        BinaryInst *createBinaryInst(Inst::Op op, Operand *lhs, Operand *rhs);
    };
}
#endif //TINBACCC_BUILDER_H
