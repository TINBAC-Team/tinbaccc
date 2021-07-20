#ifndef TINBACCC_ASM_BUILDER_H
#define TINBACCC_ASM_BUILDER_H

#include <ir/ir.h>
#include <asm_arm/instructions.h>
#include <unordered_map>
#include <list>
#include <utility>

namespace asm_arm {
    class Builder {
    public:
        Module *module;
        Function *curFunction;
        BasicBlock *curBlock;
        std::unordered_map<ir::Value*, Operand*> value_map;
        std::unordered_map<ir::BasicBlock*, BasicBlock*> block_map;
        std::list<std::pair<BasicBlock**, ir::BasicBlock*>> block_fill_list;

        Builder(Module *m);

        Operand *getOperandOfValue(ir::Value* val);

        Operand *getOrCreateOperandOfValue(ir::Value* val);

        void setOperandOfValue(ir::Value* val, Operand *operand);

        BasicBlock* getASMBBfromIRBB(ir::BasicBlock* bb);

        void setASMBBtoIRBB(ir::BasicBlock *irbb, BasicBlock *bb);

        void addPendingBBPtr(BasicBlock **pbb, ir::BasicBlock *bb);

        void fillBBPtr();

        Function *createFunction(ir::Function *f);

        BasicBlock *createBlock();

        LDRInst *createLDR(std::string s);

        LDRInst *createLDR(int v);

        STRInst * createSTR(std::string& s);

        STRInst * createSTR(int v);

        ADRInst * createADR(std::string& lb);

        BInst * createBInst(ir::BasicBlock *bb, Inst::OpCond c = Inst::OpCond::NONE);

        BInst * createBInst(BasicBlock *bb, Inst::OpCond c = Inst::OpCond::NONE);

        /**
         * Create binary instruction with lhs and rhs as operand.
         * Destination operand is a virtual register.
         * @param op
         * @param lhs
         * @param rhs
         * @return created instruction
         */
        BinaryInst *createBinaryInst(Inst::Op op, Operand *lhs, Operand *rhs);

        TernaryInst *createTernaryInst(Inst::Op op, Operand *o1, Operand *o2, Operand *o3);
    };
}
#endif //TINBACCC_BUILDER_H
