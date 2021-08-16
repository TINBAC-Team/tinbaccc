//
// Created by 28174 on 2021/7/24.
//

#include <asm_arm/instructions.h>
#include <ir/ir.h>
#include <asm_arm/builder.h>
#include <sstream>

namespace asm_arm {
    static std::unordered_map<Operand *, std::string> nameOfVReg;

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
            default:
                ret = "??";
                break;
        }
        return ret;
    }

    static unsigned numVReg = 0;

    std::string Operand::getVRegName() {
        auto it = nameOfVReg.find(this);
        if (it != nameOfVReg.end())
            return it->second;
        return nameOfVReg[this] = "vr" + std::to_string(numVReg++);
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
            case Op::LSL:
                ret_s = "LSL";
                break;
            case Op::ASR:
                ret_s = "ASR";
                break;
            case Op::LSR:
                ret_s = "LSR";
                break;
            case Op::SMMLA:
                ret_s = "SMMLA";
                break;
            case Op::SMMUL:
                ret_s = "SMMUL";
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
            case Op::VDUP:
                ret_s = "VDUP.32";
                break;
            case Op::VADD:
                ret_s = "VADD.I32";
                break;
            case Op::VSUB:
                ret_s = "VSUB.I32";
                break;
            case Op::VMUL:
                ret_s = "VMUL.I32";
                break;
            case Op::VMOV:
                ret_s = "VMOV.I32";
                break;
            case Op::VMLA:
                ret_s = "VMLA.I32";
                break;
            case Op::VLD1:
                ret_s = "VLD1.32";
                break;
            case Op::VST1:
                ret_s = "VST1.32";
                break;
            default:
                ret_s = "???";
                break;
        }
        if (nop())
            ret_s = "@ nop: " + ret_s;
        if (set_flags)
            ret_s += "S";
        return ret_s + OpCond_to_string();
    }

    const char *Inst::OpCond_to_string() const {
        switch (cond) {
            case OpCond::NONE:
                return "";
            case OpCond::EQ:
                return "EQ";
            case OpCond::NE:
                return "NE";
            case OpCond::GT:
                return "GT";
            case OpCond::GE:
                return "GE";
            case OpCond::LT:
                return "LT";
            case OpCond::LE:
                return "LE";
            case OpCond::CS:
                return "CS";
            case OpCond::CC:
                return "CC";
            case OpCond::MI:
                return "MI";
            case OpCond::PL:
                return "PL";
            case OpCond::VS:
                return "VS";
            case OpCond::VC:
                return "VC";
            case OpCond::HI:
                return "HI";
            case OpCond::LS:
                return "LS";
            default:
                break;
        }
        return "??";
    }

    void Inst::print(std::ostream &os) {
        os << "\t" << Op_to_string() << " ";
        print_body(os);
        if (!comment.str().empty())
            os << "  @" << comment.str();
        os << std::endl;
    }

    void LDRInst::print(std::ostream &os) {
        if (type == Type::IMM) {
            if (Operand::op2Imm(value) || !(value & 0xffff0000)) {
                os << "\tMOV" << OpCond_to_string() << " " << dst->getOperandName() << ", #" << value;
            } else if (Operand::op2Imm(~value)) {
                os << "\tMVN" << OpCond_to_string() << " " << dst->getOperandName() << ", #" << (~value);
            } else {
                os << "\tMOVW" << OpCond_to_string() << " " << dst->getOperandName()
                   << ", #" << (value & 0xffff) << std::endl;
                os << "\tMOVT" << OpCond_to_string() << " " << dst->getOperandName()
                   << ", #" << ((uint32_t)value >> 16);
            }
            if (!comment.str().empty())
                os << "  @" << comment.str();
            os << std::endl;
        } else {
            Inst::print(os);
        }
    }

    void LDRInst::print_body(std::ostream &os) const {
        switch (type) {

            case Type::LABEL:
                os << dst->getOperandName() << ", =" << label;
                break;
#if 0
            case Type::IMM:
                os << dst->getOperandName() << ", " << "=" + std::to_string(value);
                break;
#endif
            case Type::REGOFFS:
                os << dst->getOperandName() << ", [" << src->getOperandName();
                if (!offs) {
                    os << "]";
                } else {
                    os << ", " << offs->getOperandName();
                    if (lsl)
                        os << ", LSL #" << lsl;
                    os << "]";
                }
                break;
            default:
                std::cerr << "Error in LDR print_body!\n";
                break;
        }
    }

    void ShiftInst::print_body(std::ostream &os) const {
        os << dst->getOperandName() << ", " << src->getOperandName() << ", #" << shift;
    }

    void STRInst::print_body(std::ostream &os) const {
        os << val->getOperandName() << ", [" << addr->getOperandName();
        if (offset) {
            os << ", " << offset->getOperandName();
            if (lsl)
                os << ", LSL #" << lsl;
            os << "]";
        } else {
            os << "]";
        }
    }

    void ADRInst::print_body(std::ostream &os) const {
        os << dst->getOperandName() << ", " << label;
    }

    void MOVInst::print_body(std::ostream &os) const {
        os << dst->getOperandName() << ", " << src->getOperandName();
    }

    void CMPInst::print_body(std::ostream &os) const {
        os << lhs->getOperandName() << ", " << rhs->getOperandName();
    }

    void BInst::print_body(std::ostream &os) const {
        os << tgt->bb_label;
        if(append_pool)
        {
            os<<std::endl<<'\t'<<".pool";
        }
    }

    void CallInst::print_body(std::ostream &os) const {
        os << label;
    }

    void BinaryInst::print_body(std::ostream &os) const {
        os << dst->getOperandName() << ", " << lhs->getOperandName() << ", " << rhs->getOperandName();
        if (lsl > 0)
            os << ", LSL #" << lsl;
        else if (lsl < 0)
            os << ", LSR #" << -lsl;
    }

    void TernaryInst::print_body(std::ostream &os) const {
        os << dst->getOperandName() << ", " << op1->getOperandName() << ", " << op2->getOperandName() << ", "
           << op3->getOperandName();
    }

    void ReturnInst::print_body(std::ostream &os) const {
        // do nothing here. actual return code will be handled in function print
        os << "@END OF FUNCTION";
    }

    // ASIMD instructions
    static std::string get_qreg_name(SIMDQReg r) {
        return "q" + std::to_string(static_cast<int>(r));
    }

    static std::string get_dreg_group(SIMDQReg r) {
        std::ostringstream os;
        int qval = static_cast<int>(r);
        os << "{d" << qval * 2 << ", d" << qval * 2 + 1 << "}";
        return os.str();
    }

    void VDUPInst::print_body(std::ostream &os) const {
        os << get_qreg_name(dst) << ", " << src->getOperandName();
    }

    void VMOVInst::print_body(std::ostream &os) const {
        os << get_qreg_name(dst) << ", #" << val;
    }

    void VBinaryInst::print_body(std::ostream &os) const {
        os << get_qreg_name(dst) << ", " << get_qreg_name(lhs) << ", " << get_qreg_name(rhs);
    }

    void VLDRInst::print_body(std::ostream &os) const {
        os << get_dreg_group(dst) << ", [" << src->getOperandName() << "]";
    }

    void VSTRInst::print_body(std::ostream &os) const {
        os << get_dreg_group(src) << ", [" << dst->getOperandName() << "]";
    }

    void BasicBlock::print(std::ostream &os, bool single) const {
        std::ostringstream bbcomment;
        bbcomment << comment.str();
        auto bbsucc = succ();
        if (!bbsucc.empty()) {
            bbcomment << " succ:";
            for (auto &i:bbsucc)
                bbcomment << " " << (i->bb_label);
        }
        if (!single)
            os << bb_label << ":\n";
        if (!bbcomment.str().empty())
            os << "\t@" << bbcomment.str() << std::endl;
        for (auto &x : insts)
            x->print(os);
    }

    void Function::print(std::ostream &os) const {
        nameOfVReg.clear();
        os << "\t.align 2" << std::endl;
        os << "\t.global " << name << std::endl;
        os << "\t.arch armv8-a" << std::endl;
        os << "\t.arch_extension crc" << std::endl;
        os << "\t.syntax unified" << std::endl;
        os << "\t.arm" << std::endl;
        os << "\t.fpu crypto-neon-fp-armv8" << std::endl;
        os << "\t.type " << name << ", %function" << std::endl;
        os << name << ":\n";
        // TODO: optionally push lr
        os << "\tPUSH {";
        // preserve r4-r11
        for (int i = 4; i <= max_reg; i++) {
            if (i == 12)
                break;
            Reg reg = static_cast<Reg>(i);
            os << getRegName(reg) << ",";
        }
        os << "lr}" << std::endl;
        if (stack_size) {
            if (Operand::op2Imm(stack_size)) {
                os << "\tSUB sp, sp, #" << stack_size << std::endl;
            } else {
                os << "\tMOV r12, #" << (stack_size & 0xffff) << std::endl;
                if (stack_size & 0xffff0000)
                    os << "\tMOVT r12, #" << ((unsigned int) stack_size >> 16) << std::endl;
                os << "\tSUB sp, sp, r12" << std::endl;
            }
        }

        if (bList.size() == 1)
            (*bList.begin())->print(os, true);
        else {
            for (auto &x : bList)
                x->print(os, false);
        }
        if (stack_size) {
            if (Operand::op2Imm(stack_size)) {
                os << "\tADD sp, sp, #" << stack_size << std::endl;
            } else {
                os << "\tLDR r12, =" << stack_size << std::endl;
                os << "\tADD sp, sp, r12" << std::endl;
            }
        }
        // TODO: optionally pop lr->pc
        // restore r4-r11
        os << "\tPOP {";
        for (int i = 4; i <= max_reg; i++) {
            if (i == 12)
                break;
            Reg reg = static_cast<Reg>(i);
            os << getRegName(reg) << ",";
        }
        os << "pc}" << std::endl;
        os << "\t.pool" << std::endl;
        os << "\t.size " << name << ", .-" << name << std::endl;
    }

    void Module::printGlobalVar(std::ostream &os, ir::GlobalVar *v) {
        os << "\t.global " << v->name << std::endl;
        os << "\t." << (v->initval.empty() ? "bss" : "data") << std::endl;
        /**
         * quote from binutil doc:
         * For other systems, including ppc, i386 using a.out format,
         * arm and strongarm, it is the number of low-order zero bits
         * the location counter must have after advancement. For example
         * ‘.align 3’ advances the location counter until it is a multiple of 8.
         */
        os << "\t.align 2" << std::endl;
        os << "\t.type " << v->name << ", %object" << std::endl;
        os << "\t.size " << v->name << ", " << v->len * 4 << std::endl;
        os << v->name << ":" << std::endl;
        int gen_space = 0;
        int fold_zero = 0;
        for (const auto &i:v->initval) {
            if (i) {
                if (fold_zero) {
                    os << "\t.space " << fold_zero * 4 << std::endl;
                    gen_space += fold_zero;
                    fold_zero = 0;
                }
                os << "\t.word " << i << std::endl;
                gen_space++;
            } else {
                fold_zero++;
            }
        }
        if (gen_space < v->len)
            os << "\t.space " << (v->len - gen_space) * 4 << std::endl;
    }

    void Module::print(std::ostream &os) const {
        os << "\t.arch armv8-a" << std::endl;
        os << "\t.arch_extension crc" << std::endl;
        for (auto &x:irModule->globalVarList)
            printGlobalVar(os, x);
        os << "\t.text" << std::endl;
        for (auto &x : functionList)
            x->print(os);
        // sylib generates additional outputs and we have to link it.
        // throw an arbitrary branch here. It should be unreachable.
        os << "\tb getint" << std::endl;
    }

    void Builder::print(std::ostream &os) {
        module->print(os);
    }

    void PoolInst::print(std::ostream &os) {
        os<<"B .L_AFTER_POOL"<<number<<std::endl;
        os<<"\t"<<".pool"<<std::endl;
        os<<".L_AFTER_POOL"<<number<<":"<<std::endl;
    }

    void PoolInst::print_body(std::ostream &os) const {}
}