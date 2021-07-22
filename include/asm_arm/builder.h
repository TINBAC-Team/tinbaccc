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
        struct PhiMOV {
            ir::BasicBlock* irbb;
            ir::Value* irval;
            Operand *dst;
        };
        std::list<PhiMOV> phi_mov_list;

        Builder(Module *m);

        Operand *getOperandOfValue(ir::Value* val);

        Operand *getOrCreateOperandOfValue(ir::Value* val);

        void setOperandOfValue(ir::Value* val, Operand *operand);

        BasicBlock* getASMBBfromIRBB(ir::BasicBlock* bb);

        void setASMBBtoIRBB(ir::BasicBlock *irbb, BasicBlock *bb);

        void addPendingBBPtr(BasicBlock **pbb, ir::BasicBlock *bb);

        void fillBBPtr();

        void addPendingMOV(ir::BasicBlock* irbb, ir::Value* irval, Operand *dst);

        void fillMOV();

        Function *createFunction(ir::Function *f);

        BasicBlock *createBlock();

        LDRInst *createLDR(std::string s);

        LDRInst *createLDR(int v);

        LDRInst *createLDR(Operand *s, Operand *o);

        STRInst * createSTR(Operand *v, Operand *a, Operand *o);

        ADRInst * createADR(std::string& lb);

        BInst * createBInst(ir::BasicBlock *bb, Inst::OpCond c = Inst::OpCond::NONE);

        BInst * createBInst(BasicBlock *bb, Inst::OpCond c = Inst::OpCond::NONE);

        CallInst * createCall(std::string lb, int np, bool is_void);

        /**
         * Create binary instruction with lhs and rhs as operand.
         * Destination operand is a virtual register.
         * @param op
         * @param lhs
         * @param rhs
         * @return created instruction
         */
        BinaryInst *createBinaryInst(Inst::Op op, Operand *lhs, Operand *rhs);

        BinaryInst *moveSP(bool is_sub, int len);

        TernaryInst *createTernaryInst(Inst::Op op, Operand *o1, Operand *o2, Operand *o3);

        CMPInst *createCMPInst(Operand *lhs, Operand *rhs);

        MOVInst *createMOVInst(Operand *dst, Operand *src);

        Operand *allocate_stack(unsigned int ni32s);
    };
}
#endif //TINBACCC_BUILDER_H
