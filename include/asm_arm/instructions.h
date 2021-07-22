#ifndef TINBACCC_INSTRUCTIONS_H
#define TINBACCC_INSTRUCTIONS_H

#include <string>
#include <list>
#include <unordered_map>

namespace ir {
    class Function;
}

namespace asm_arm {
    class BasicBlock;

    enum class Reg {
        r0 = 0,
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
        static std::unordered_map<Reg, Operand *> precolored_reg_map;

        Operand(Type t) : type(t) {};

        static Operand *newImm(int v);

        static Operand *getReg(Reg r);

        static void resetRegMap();

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
            STR,
            ADR,
            MOV,
            CMP,
            TST,
            B,
            BL,
            AND,
            ORR,
            EOR,
            ADD,
            SUB,
            RSB,
            MLA,
            MLS,
            MUL,
            SDIV,
            RETURN, // not an actual instruction
        } op;

        enum class OpCond {
            EQ = 0,
            NE,
            CS,
            CC,
            MI,
            PL,
            VS,
            VC,
            HI,
            LS,
            GE,
            LT,
            GT,
            LE,
            NONE,
            UNDEF
        } cond;

        Inst(Op o, OpCond c = OpCond::NONE) : op(o), cond(c) {}

        virtual void print() {};

        virtual ~Inst() {};
    };

    class LDRInst : public Inst {
    public:
        enum class Type {
            LABEL,
            IMM,
            REGOFFS,
        } type;
        std::string label;
        int value;
        Operand *dst;
        Operand *src, *offs;

        LDRInst(std::string l, Operand *d);

        LDRInst(int v, Operand *d);

        LDRInst(Operand *d, Operand *s, Operand *o);
    };

    /**
     * STR Rt, Rn, Offset
     * *(Rn+Offset) = Rt
     * Offset can be imm12
     */
    class STRInst : public Inst {
    public:
        Operand *val, *addr, *offset;

        STRInst(Operand *v, Operand *a, Operand *o);
    };

    class ADRInst : public Inst {
    public:
        Operand *dst;
        std::string label;

        ADRInst(Operand *d, std::string lb);
    };

    class Inst2_1 : public Inst { // 2 operands, including 1 Reg and 1 <Operand2>
    public:
        enum class Type {
            Imm,
            Reg,
        } type_operand2;
        Operand *dst, *src;

        Inst2_1(Op o, Operand *d, int s_imm);

        Inst2_1(Op o, Operand *d, Operand *s);
    };

    class MOVInst : public Inst {
    public:
        Operand *dst, *src;

        MOVInst(Operand *d, Operand *s);
    };

    class CMPInst : public Inst {
        Operand *lhs, *rhs;
    public:
        CMPInst(Operand *l, Operand *r);
    };

    class TSTInst : public Inst2_1 {
    public:
        TSTInst(Operand *d, int s_imm);

        TSTInst(Operand *d, Operand *s);
    };

    class BInst : public Inst {
    public:
        BasicBlock *tgt;

        BInst(OpCond c = OpCond::NONE);
    };

    /**
     * BL <label>
     * Rename it to indicate that we are specifically using it for
     * function call. It returns weird use/def for caller-preserved registers
     */
    class CallInst : public Inst {
    public:
        int nparams;
        bool is_void;
        std::string label;
        CallInst(int np, std::string l, bool _is_void);
    };

    class BinaryInst : public Inst { // 3 operands, including 2 registers and 1 <Operand2>
    public:
        Operand *dst, *lhs, *rhs;

        BinaryInst(Op o, Operand *d, Operand *l, Operand *r);
    };

    class TernaryInst : public Inst {
    public:
        Operand *dst, *op1, *op2, *op3;

        TernaryInst(Op o, Operand *d, Operand *o1, Operand *o2, Operand *o3);
    };

    class ReturnInst : public Inst {
    public:
        bool has_return_value;

        ReturnInst(bool ret) : Inst(Op::RETURN), has_return_value(ret) {}
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
        unsigned int stack_size;

        Function(ir::Function *f);

        void appendBlock(BasicBlock *block);

        void appendReturnBlock();

        unsigned int allocate_stack(unsigned int ni32s);
    };

    class Module {
    public:
        std::list<Function *> functionList;
    };

}
#endif //TINBACCC_INSTRUCTIONS_H
