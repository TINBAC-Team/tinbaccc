//
// Created by 28174 on 2021/7/24.
//

#include <asm_arm/instructions.h>
#include <ir/ir.h>
#include <asm_arm/builder.h>

namespace asm_arm {

    std::string getRegName(Reg &reg) {
        std::string ret;
        switch (reg) {
            case Reg::r0:
                ret = "r0";
                break;
            case Reg::r1:
                ret = "r1";
                break;
            case Reg::r2:
                ret = "r2";
                break;
            case Reg::r3:
                ret = "r3";
                break;
            case Reg::r4:
                ret = "r4";
                break;
            case Reg::r5:
                ret = "r5";
                break;
            case Reg::r6:
                ret = "r6";
                break;
            case Reg::r7:
                ret = "r7";
                break;
            case Reg::r8:
                ret = "r8";
                break;
            case Reg::r9:
                ret = "r9";
                break;
            case Reg::r10:
                ret = "r10";
                break;
            case Reg::r11:
                ret = "r11";
                break;
            case Reg::r12:
                ret = "r12";
                break;
            case Reg::sp:
                ret = "sp";
                break;
            case Reg::lr:
                ret = "lr";
                break;
            case Reg::pc:
                ret = "pc";
                break;
        }
        return ret;
    }

    static unsigned numVReg = 0;
    std::string Operand::getVRegName() {
        std::string ret = "vr";
        ret += std::to_string(numVReg++);
        return ret;
    }

    std::string Operand::getOperandName() {
        std::string name;
        switch (type) {
            case Type::Reg:
                name = getRegName(reg);
                break;
            case Type::VReg:
                name = getVRegName();
                break;
            case Type::Imm:
                name = "#" + std::to_string(val);
                break;
        }
        return name;
    }

    std::string Inst::Op_to_string() const {
        std::string ret_s;
        switch (op) {
            case Op::LDR:
                ret_s = "LDR";
                break;
            case Op::STR:
                ret_s = "STR";
                break;
            case Op::CMP:
                ret_s = "CMP";
                break;
            case Op::TST:
                ret_s = "TST";
                break;
            case Op::MOV:
                ret_s = "MOV";
                break;
            case Op::SDIV:
                ret_s = "SDIV";
                break;
            case Op::MUL:
                ret_s = "MUL";
                break;
            case Op::ADD:
                ret_s = "ADD";
                break;
            case Op::SUB:
                ret_s = "SUB";
                break;
            case Op::RSB:
                ret_s = "RSB";
                break;
            case Op::AND:
                ret_s = "AND";
                break;
            case Op::ORR:
                ret_s = "ORR";
                break;
            case Op::EOR:
                ret_s = "EOR";
                break;
            case Op::B:
                ret_s = "B";
                break;
            case Op::BL:
                ret_s = "BL";
                break;
            case Op::ADR:
                ret_s = "ADR";
                break;
            case Op::MLA:
                ret_s = "MLA";
                break;
            case Op::MLS:
                ret_s = "MLS";
                break;
            case Op::RETURN:
                // TODO
                break;
            default:
                ret_s = "???";
                break;
        }
        switch(cond) {
            case OpCond::NONE:
                break;
            case OpCond::EQ:
                ret_s += "EQ";
                break;
            case OpCond::NE:
                ret_s += "NE";
                break;
            case OpCond::GT:
                ret_s += "GT";
                break;
            case OpCond::GE:
                ret_s += "GE";
                break;
            case OpCond::LT:
                ret_s += "LT";
                break;
            case OpCond::LE:
                ret_s += "LE";
                break;
            default:
                // TODO
                break;
        }
        return ret_s;
    }


    void LDRInst::print(std::ostream &os) const {
        os << "\t" << Op_to_string() << " ";
        switch (type) {
            case Type::LABEL:
                os << dst->getOperandName() << ", " << label << std::endl;
                break;
            case Type::IMM:
                os << dst->getOperandName() << ", " << "=" + std::to_string(value) << std::endl;
                break;
            case Type::REGOFFS:
                os << dst->getOperandName() << ", [" << src->getOperandName();
                if (!offs)
                    os << "]" << std::endl;
                else
                    os << ", " << offs->getOperandName() << "]" << std::endl;
                break;
            default:
                std::cerr << "Error in LDR print!\n";
                break;
        }
    }

    void STRInst::print(std::ostream &os) const {
        os << "\t" << Op_to_string() << " " << val->getOperandName() << ", [" << addr->getOperandName();
        if (offset)
            os << ", " << offset->getOperandName() << "]" << std::endl;
        else
            os << "]" << std::endl;
    }

    void ADRInst::print(std::ostream &os) const {
        os << "\t" << Op_to_string() << " " << dst->getOperandName() << ", " << label << std::endl;
    }

    void MOVInst::print(std::ostream &os) const {
        os << "\t" << Op_to_string() << " " << dst->getOperandName() << ", " << src->getOperandName() << std::endl;
    }

    void CMPInst::print(std::ostream &os) const {
        os << "\t" << Op_to_string() << " " << lhs->getOperandName() << ", " << rhs->getOperandName() << std::endl;
    }

    void BInst::print(std::ostream &os) const {
        os << "\t" << Op_to_string() << " " << tgt->bb_label << std::endl;
    }

    void CallInst::print(std::ostream &os) const {
        os << "\t" << Op_to_string() << label << std::endl;
    }

    void BinaryInst::print(std::ostream &os) const {
        os << "\t" << Op_to_string() << " ";
        os << dst->getOperandName() << ", " << lhs->getOperandName() << ", " << rhs->getOperandName() << std::endl;
    }

    void TernaryInst::print(std::ostream &os) const {
        os << "\t" << Op_to_string() << " ";
        os << dst->getOperandName() << ", " << op1->getOperandName() << ", " << op2->getOperandName() << ", " << op3->getOperandName() << std::endl;
    }

    void ReturnInst::print(std::ostream &os) const {
        if (has_return_value)
            // TODO
            ;
        // TODO
        os << "\tMOV pc, lr\n";
    }

    void BasicBlock::print(std::ostream &os, bool single) const {
        if (!single)
            os << bb_label << ":\n";
        for (auto &x : insts)
            x->print(os);
    }

    void Function::print(std::ostream &os) const {
        os << name << ":\n";
        if (bList.size() == 1)
            (*bList.begin())->print(os, true);
        else {
            for (auto &x : bList)
                x->print(os, false);
        }
    }

    void Module::print(std::ostream &os) const {
        for (auto &x : functionList)
            x->print(os);
    }

    void Builder::print(std::ostream& os) {
        module->print(os);
    }
}