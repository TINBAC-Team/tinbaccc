#include <ast/ast.h>
#include <ir/ir.h>

using std::ostream;
using std::endl;

namespace ir {
    std::unordered_map<Value *, std::string> nameOfValue;
    std::unordered_map<const BasicBlock * ,std::string> nameOfBB;

    static std::string generate_new_name() {
        static int seed = 0;
        return std::to_string(seed++);
    };

    static std::string get_name_of_value(Value *val) {
        if (auto const_val = dynamic_cast<ConstValue *>(val)) {
            return std::to_string(const_val->value);
        }
        if (nameOfValue.find(val) == nameOfValue.end())
            nameOfValue[val] = generate_new_name();
        return "%" + nameOfValue[val];
    }

    static std::string get_name_of_BB(const BasicBlock *bb) {
        if(nameOfBB.find(bb) == nameOfBB.end())
            nameOfBB[bb] = generate_new_name();
        return nameOfBB[bb];
    }

    ostream &operator<<(ostream &os, const Module &m) {
        for (auto i:m.globalVarList)
            i->print(os);
        for (auto i:m.functionList)
            i->print(os);
        return os;
    }

    static const char *op_to_str(OpType op) {
        switch (op) {
            case OpType::ADD:
                return "add";
            case OpType::SUB:
                return "sub";
            case OpType::MUL:
                return "mul";
            case OpType::SDIV:
                return "sdiv";
            case OpType::SREM:
                return "srem";
            case OpType::SLT:
                return "icmp slt";
            case OpType::SLE:
                return "icmp sle";
            case OpType::SGE:
                return "icmp sge";
            case OpType::SGT:
                return "icmp sgt";
            case OpType::EQ:
                return "icmp eq";
            case OpType::NE:
                return "icmp ne";
            case OpType::AND:
                return "and";
            case OpType::OR:
                return "or";
            case OpType::BRANCH:
                return "br";
            case OpType::JUMP:
                return "br";
            case OpType::RETURN:
                return "ret";
            case OpType::LOAD:
                return "load";
            case OpType::STORE:
                return "store";
            case OpType::CALL:
                return "call";
            case OpType::ALLOCA:
                return "alloca";
            case OpType::PHI:
                return "phi";
            case OpType::CONST:
                return "TBD--const"; //TODO:const optype
            case OpType::GLOBAL:
                return "global";
            case OpType::PARAM:
                return "TBD--param"; //TODO:param optype
            case OpType::GETELEMPTR:
                return "getelementptr";
            default:
                return "unknown";
        }
    }

    void Value::print(std::ostream &os) const {
        os << op_to_str(optype) << " ";
    }

    void Function::print(std::ostream &os) const {
        os << (is_extern() ? "declare " : "define ")
           << (return_int ? "i32 " : "void ")
           << "@" << name << " (";
        bool is_first = true;
        for (auto &p:params) {
            if (!is_first)
                os << ", ";
            is_first = false;
            os << "i32 ";
            if (p->decl->is_array())
                os << "* ";
            os << "%" << p->decl->name;
        }
        os << ")";
        if(!is_extern())
            os << " #"<<get_name_of_BB(bList.front()) ;
        os <<" {" << endl;
        //body
        for (auto &bb:bList) {
                bb->print(os, bb == bList.front());
        }
        os << "}" << endl;

    }

    void BasicBlock::print(std::ostream &os, bool is_first) const {
        if (!is_first) os << "; <label>:"<<get_name_of_BB(this)<<":" << std::endl;
        for (auto &inst:iList) {
            os << "\t";
            inst->print(os);
            os << std::endl;
        }
    }

    void CallInst::print(std::ostream &os) const {
        if (!is_void) {
            os << get_name_of_value((Value *) this) << " = ";
        }
        Value::print(os);
        if (is_void) os << "void ";
        else os << "i32 ";
        os << "@" << fname << "(";
        bool is_first = true;
        for (auto &p:params) {
            if (!is_first)
                os << ", ";
            is_first = false;
            os << "i32 ";
            os << get_name_of_value(p);
        }
        os << ")";
    }

    void BinaryInst::print(std::ostream &os) const {
        os << get_name_of_value((Value *) this) << " = ";
        Value::print(os);
        if (optype == OpType::ADD || optype == OpType::SUB || optype == OpType::MUL || optype == OpType::SDIV
            || optype == OpType::SLT || optype == OpType::SLE || optype == OpType::SGT || optype == OpType::SGE) {
            os << "i32 " << get_name_of_value(ValueL.value) << ", " << get_name_of_value(ValueR.value);
        }
    }

    void BranchInst::print(std::ostream &os) const{
        Value::print(os);
        os<<"i1 "<<get_name_of_value(cond.value)<<", label %"<<get_name_of_BB(true_block)<<", label %"<<get_name_of_BB(false_block);
    }
    void JumpInst::print(std::ostream &os) const{
        Value::print(os);
        os<<"label %"<<get_name_of_BB(to);
    }
}