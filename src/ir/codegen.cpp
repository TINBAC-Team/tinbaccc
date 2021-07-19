#include <ir/ir.h>
#include <asm_arm/builder.h>
#include <asm_arm/instructions.h>
#include <iostream>

namespace ir {
    void Module::codegen(asm_arm::Builder &builder) {
        for (auto &i:functionList)
            i->codegen(builder);
    }

    void Function::codegen(asm_arm::Builder &builder) {
        builder.createFunction(this);
        for (auto &i:bList)
            i->codegen(builder);
        // TODO: generate return instructions
        builder.curFunction->appendReturnBlock();
    }

    void BasicBlock::codegen(asm_arm::Builder &builder) {
        builder.createBlock();
        for (auto &i:iList)
            i->codegen(builder);
    }

    asm_arm::Operand *Value::codegen(asm_arm::Builder &builder) {
        return nullptr;
    }

    asm_arm::Operand *ConstValue::codegen(asm_arm::Builder &builder) {
        return asm_arm::Operand::newImm(value);
    }

    asm_arm::Operand *ConstValue::genreg(asm_arm::Builder &builder) { //all reg are vreg
        auto inst = builder.createLDR(value);
        return inst->dst;
    }

    asm_arm::Operand *ConstValue::genop2(asm_arm::Builder &builder) {
        if (asm_arm::Operand::op2Imm(value))
            return (codegen(builder));
        return genreg(builder);
    }

    asm_arm::Operand* BinaryInst::codegen(asm_arm::Builder &builder) {
        ConstValue * ConstL = dynamic_cast<ConstValue *>(ValueL.value);
        ConstValue * ConstR = dynamic_cast<ConstValue *>(ValueR.value);
        asm_arm::Operand * s_mov = ConstL->genop2(builder);
        asm_arm::Operand * s2 = ConstR->genop2(builder);
        auto last = builder.createInst2_1_<asm_arm::MOVInst>(asm_arm::Inst::Op::MOV, asm_arm::Operand::newVReg(), s_mov); // MOV Instruction
        switch (BinaryInst::optype) {
            case OpType::ADD: {
                auto ret = builder.createInst1_<asm_arm::ADDInst>(asm_arm::Inst::Op::ADD, last->dst, last->dst, s2); // ADD Instruction
                return ret->dst;
            }
            case OpType::SUB: {
                auto ret = builder.createInst1_<asm_arm::SUBInst>(asm_arm::Inst::Op::SUB, last->dst, last->dst, s2); // SUB Instruction
                return ret->dst;
            }
            case OpType::AND: {
                auto ret = builder.createInst1_<asm_arm::ANDInst>(asm_arm::Inst::Op::AND, last->dst, last->dst, s2); // AND Instruction
                return ret->dst;
            }
            case OpType::OR: {
                auto ret = builder.createInst1_<asm_arm::ORRInst>(asm_arm::Inst::Op::ORR, last->dst, last->dst, s2); // ORR Instruction
                return ret->dst;
            }
            case OpType::MUL: {
                auto last2 = builder.createInst2_1_<asm_arm::MOVInst>(asm_arm::Inst::Op::MOV, asm_arm::Operand::newVReg(), s2); // MOV Instruction
                auto ret = builder.createInst2<asm_arm::MULInst>(asm_arm::Inst::Op::MUL, last->dst, last->dst, last2->dst); // MUL Instruction
                return ret->dst;
            }
            case OpType::SDIV: {
                auto last2 = builder.createInst2_1_<asm_arm::MOVInst>(asm_arm::Inst::Op::MOV, asm_arm::Operand::newVReg(), s2); // MOV Instruction
                auto ret = builder.createInst2<asm_arm::SDIVInst>(asm_arm::Inst::Op::SDIV, last->dst, last->dst, last2->dst); // SDIV Instruction
                return ret->dst;
            }
            case OpType::SREM: {
                auto last2 = builder.createInst2_1_<asm_arm::MOVInst>(asm_arm::Inst::Op::MOV, asm_arm::Operand::newVReg(), s2); //MOV Instruction
                auto last3 = builder.createInst2<asm_arm::SDIVInst>(asm_arm::Inst::Op::SDIV, asm_arm::Operand::newVReg(), last->dst, last2->dst);
                builder.createInst2<asm_arm::MULInst>(asm_arm::Inst::Op::MUL, last3->dst, last3->dst, last2->dst);
                auto ret = builder.createInst1_<asm_arm::SUBInst>(asm_arm::Inst::Op::SUB, last3->dst, last->dst, last3->dst); //SUB Instruction
                return ret->dst;
            }
            case OpType::EQ:
            case OpType::NE:
            case OpType::SLT:
            case OpType::SLE:
            case OpType::SGT:
            case OpType::SGE: {
                builder.createInst2_1_<asm_arm::CMPInst>(asm_arm::Inst::Op::CMP, last->dst, s2);
                std::string st = "True";
                std::string sf = "False";
                switch (BinaryInst::optype) {
                    case OpType::EQ:
                        builder.createBInst(st, asm_arm::BInst::Type::EQ);
                    case OpType::NE:
                        builder.createBInst(st, asm_arm::BInst::Type::NE);
                    case OpType::SLT:
                        builder.createBInst(st, asm_arm::BInst::Type::LT);
                    case OpType::SLE:
                        builder.createBInst(st, asm_arm::BInst::Type::LE);
                    case OpType::SGT:
                        builder.createBInst(st, asm_arm::BInst::Type::GT);
                    case OpType::SGE:
                        builder.createBInst(st, asm_arm::BInst::Type::GE);
                    default:
                        std::cerr << "Error in codegen(suffix)!\n";
                }
                builder.createBInst(sf);
                builder.createLABELofInst(st);
                auto ret = builder.createInst2_1<asm_arm::MOVInst>(asm_arm::Inst::Op::MOV, asm_arm::Operand::newVReg(), 1);
                builder.createLABELofInst(sf);
                builder.createInst1_<asm_arm::EORInst>(asm_arm::Inst::Op::EOR, ret->dst, ret->dst, ret->dst);
                return ret->dst;
            }
            default:
                std::cerr << "Error in codegen!\n";
                return nullptr;
        }
    }
}