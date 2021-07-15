#include <ast/ast.h>
#include <ir/ir.h>

using std::ostream;
using std::endl;

namespace ir {
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
            case OpType::SUB:
            case OpType::MUL:
            case OpType::SDIV:
            case OpType::SREM:
            case OpType::SLT:
            case OpType::SLE:
            case OpType::SGE:
            case OpType::SGT:
            case OpType::EQ:
            case OpType::NE:
            case OpType::AND:
            case OpType::OR:
            case OpType::BRANCH:
            case OpType::JUMP:
            case OpType::RETURN:
            case OpType::LOAD:
            case OpType::STORE:
            case OpType::CALL:
            case OpType::ALLOCA:
            case OpType::PHI:
            case OpType::MEMOP:
            case OpType::CONST:
            case OpType::GLOBAL:
            case OpType::PARAM:
            case OpType::UNDEF:
            case OpType::GETELEMPTR:
            default:
                return "unknown";
        }
    }

    void Value::print(std::ostream &os) const {
        os << '\t' << op_to_str(optype) << endl;
    }

    void Function::print(std::ostream &os) const {
        os << (is_extern() ? "declare " : "define ")
            << (return_int ? "i32 " : "void ")
            << "@" << name << " (";
        bool is_first = true;
        for(auto &p:params){
            if(!is_first)
                os << ", ";
            is_first = false;
            os << "i32 ";
            if(p->decl->is_array())
                os << "* ";
            os << "%" << p->decl->name;
        }
        os << ")" << endl; //TODO: body
    }
}