#ifndef TINBACCC_INSTRUCTIONS_H
#define TINBACCC_INSTRUCTIONS_H

#include <string>
#include <list>

namespace ir {
    class Function;
}

namespace asm_arm {
    class BasicBlock;

    enum class Reg {
        r0,
        r1,
        r2,
        r3,
        r4,
        r5,
        r6,
        r7,
        r8,
        r9,
        r10,
        r11,
        r12,
        sp,
        lr,
        pc
    };

    class Operand {
    public:
        enum class Type {
            Reg,
            VReg,
            Imm
        } type;
        Reg reg;
        int val;

        Operand(Type t) : type(t) {};

        static Operand *newImm(int v);

        static Operand *newReg(Reg r);

        static Operand *newVReg();

        /**
         * OP2 of binary operations in ARM can be a immediate value encoded
         * with 8-bit number with 4-bit rotation.
         * This function checks if a number can be encoded this way.
         * code are taken from GCC
         *
         * @param val to be checked
         * @return whether the value can be encoded this way
         */
        static bool op2Imm(int val);
    };

    class Inst {
    public:
        BasicBlock *bb;
        enum class Op {
            LDR,
            MOV,
            // Binary arithmetic ops
            ADD,
            SUB,
            MUL,
            SDIV,
        } op;

        Inst(Op o) : op(o) {}

        virtual ~Inst() {};
    };

    class LDRInst : public Inst {
    public:
        enum class Type {
            LABEL,
            IMM
        } type;
        std::string label;
        int value;
        Operand *dst;

        LDRInst(std::string l, Operand *d);

        LDRInst(int v, Operand *d);
    };

    class BasicBlock {
    public:
        std::list<Inst *> insts;
        std::list<Inst *>::const_iterator it_branch;

        void insertAtEnd(Inst *inst);

        void markBranch();

        void insertBeforeBranch(Inst *inst);

    };

    class Function {
    public:
        std::string name;
        ir::Function *func;
        std::list<BasicBlock *> bList;
        BasicBlock *ret_block;

        Function(ir::Function *f);

        void appendBlock(BasicBlock *block);

        void appendReturnBlock();
    };

    class Module {
    public:
        std::list<Function *> functionList;
    };

}
#endif //TINBACCC_INSTRUCTIONS_H
