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
		STR,
		ADR,
		MOV,
		CMP,
		TST,
		B,
		LABEL,
		AND,
		ORR,
		EOR,
		ADD,
		SUB,
		MUL,
		SDIV,
	} op;

	Inst(Op o) : op(o) {}

	virtual void print();

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

class STRInst : public Inst {
  public:
	enum class Type {
		LABEL,
		IMM
	} type;
	std::string label;
	int value;
	Operand *dst;

	STRInst(std::string l, Operand *d);

	STRInst(int v, Operand *d);
};

class ADRInst : public Inst {
  public:
	Operand *dst;
	std::string label;

	ADRInst(Operand *d, std::string lb);
};

//暂未考虑每条指令可能带有的的后缀S，即设置标志位
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

class MOVInst : public Inst2_1 {
  public:
	MOVInst(Operand *d, int s_imm);

	MOVInst(Operand *d, Operand *s);
};

class CMPInst : public Inst2_1 {
  public:
	CMPInst(Operand *d, int s_imm);

	CMPInst(Operand *d, Operand *s);
};

class TSTInst : public Inst2_1 {
  public:
	TSTInst(Operand *d, int s_imm);

	TSTInst(Operand *d, Operand *s);
};

class BInst : public Inst {
  public:
	std::string label;
	bool has_suffix;
	enum class Type {
		EQ,
		NE,
		GT,
		GE,
		LT,
		LE
	} suffix;

	BInst(std::string lb);

	BInst(std::string lb, Type sf);
};

class LABELofInst : public Inst {
  public:
	std::string label;

	LABELofInst(std::string lb);
};


class Inst1 : public Inst { // 3 operands, including 2 registers and 1 <Operand2>
  public:
	enum class Type {
		Imm,
		Reg,
	} type_operand2;
//        enum class Type_shift {
//            ASR,
//            LSL,
//            LSR,
//            ROR
//        } type_shift;
//        unsigned num_shift;
	Operand *dst, *src1, *src2;

	Inst1(Op o, Operand *d, Operand *s1, int s2_imm); // 2 registers and 1 immediate number

	Inst1(Op o, Operand *d, Operand *s1, Operand *s2); // 3 registers without shift
};

class Inst2 : public Inst { // 3 operands, including 3 registers
  public:
	Operand *dst, *src1, *src2;

	Inst2(Op o, Operand *d, Operand *s1, Operand *s2);
};

class ADDInst : public Inst1 {
  public:
	ADDInst(Operand *d, Operand *s1, int s2_imm);

	ADDInst(Operand *d, Operand *s1, Operand *s2);
};

class SUBInst : public Inst1 {
  public:
	SUBInst(Operand *d, Operand *s1, int s2_imm);

	SUBInst(Operand *d, Operand *s1, Operand *s2);
};

class MULInst : public Inst2 {
  public:
	MULInst(Operand *d, Operand *s1, Operand *s2);
};

class SDIVInst : public Inst2 {
  public:
	SDIVInst(Operand *d, Operand *s1, Operand *s2);
};

class ANDInst : public Inst1 {
  public:
	ANDInst(Operand *d, Operand *s1, int s2_imm);

	ANDInst(Operand *d, Operand *s1, Operand *s2);
};

class ORRInst : public Inst1 {
  public:
	ORRInst(Operand *d, Operand *s1, int s2_imm);

	ORRInst(Operand *d, Operand *s1, Operand *s2);

};

class EORInst : public Inst1 {
  public:
	EORInst(Operand *d, Operand *s1, int s2_imm);

	EORInst(Operand *d, Operand *s1, Operand *s2);
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
