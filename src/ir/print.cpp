#include <ast/ast.h>
#include <ir/ir.h>
#include <regex>
#include <sstream>

using std::ostream;
using std::endl;

namespace ir {
    std::unordered_map<Value *, std::string> nameOfValue;
    std::unordered_map<const BasicBlock *, std::string> nameOfBB;
    int seed;
    int bb_seed;

    static std::string generate_new_name() {
        return "x" + std::to_string(seed++);
    };

    static std::string generate_new_bb_name() {
        return "__bb" + std::to_string(bb_seed++);
    };

    static std::string get_valid_name(std::string str) {
        if(std::regex_match(str,std::regex("^x[0-9]+")))
        {
            return "_"+str;
        } else return str;
    }

    static std::string get_name_of_value(Value *val, const std::string &define_name = "") {
        if (!val && define_name.empty()) {
            throw std::runtime_error("print_body ir: requesting name of nullptr");
        }
        if (auto const_val = dynamic_cast<ConstValue *>(val)) {
            return std::to_string(const_val->value);
        }
        if (auto global_val = dynamic_cast<GlobalVar *>(val)) {
            return "@" + global_val->name;
        }
        if (nameOfValue.find(val) == nameOfValue.end()) {
            if (define_name.empty()) nameOfValue[val] = generate_new_name();
            else nameOfValue[val] = get_valid_name(define_name);
        }

        return "%" + nameOfValue[val];
    }

    static std::string get_name_of_BB(const BasicBlock *bb) {
        if (nameOfBB.find(bb) == nameOfBB.end())
            nameOfBB[bb] = generate_new_bb_name();
        return nameOfBB[bb];
    }
    bool has_llvm_arr_initval(ast::Decl* decl,int l,int r)
    {
        for(int i=l;i<=r;i++)
        {
            if(i>=decl->initval_expanded.size()) break;
            if(decl->initval_expanded[i] && decl->initval_expanded[i]->get_value()!=0) return true;
        }
        return false;
    }
    void print_llvm_arr_inner_decl(std::ostream &os, std::vector<ast::Exp *> &array_dims, int dim) {
        if (dim >= array_dims.size()) {
            os << "i32";
            return;
        }
        os << "[" << array_dims[dim]->get_value() << " x ";
        print_llvm_arr_inner_decl(os, array_dims, dim + 1);
        os << "]";
    }
    int get_initval_from_offset(ast::Decl *decl, int offset)
    {
        if(offset>=decl->initval_expanded.size()) return 0;
        else return decl->initval_expanded[offset]==nullptr ? 0 : decl->initval_expanded[offset]->get_value();
    }
    void print_llvm_arr_decl(std::ostream &os, ast::Decl *decl, std::vector<int> cur_index, int l, int r) {

        print_llvm_arr_inner_decl(os,decl->array_dims,cur_index.size());
        os<<" ";
        if(!has_llvm_arr_initval(decl,l,r))
        {
            os<<"zeroinitializer";
            return;
        }
        if(cur_index.size()!=decl->array_dims.size()-1)
        {
            os<<"[";
            int next_dim_size = decl->array_dims[cur_index.size()]->get_value();
            bool is_first = true;
            for(int i=0;i<next_dim_size;i++)
            {
                if(!is_first) os<<", ";
                is_first = false;
                std::vector<int> new_index(cur_index);
                new_index.push_back(i);
                print_llvm_arr_decl(os,decl,new_index,l+((r-l+1)/next_dim_size)*i,l+((r-l+1)/next_dim_size)*(i+1)-1);
            }

            os<<"]";
        } else
        {
            bool is_first = true;
            os<<"[";
            int offset = 0;
            for(int i=0;i<cur_index.size();i++)
            {
                offset+=decl->array_multipliers[i+1]*cur_index[i];
            }
            for(int i=0;i<decl->array_dims.back()->get_value();i++)
            {
                if(!is_first) os<<", ";
                is_first = false;
                os<<"i32 "<<get_initval_from_offset(decl,offset);
                offset++;
            }
            os<<"]";
        }
    }


    ostream &operator<<(ostream &os, const Module &m) {
        for (auto i:m.globalVarList) {
            i->print(os);
            os << std::endl;
        }

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
        seed = 0;
        bb_seed = 0;
        os << (is_extern() ? "declare " : "define ")
           << (return_int ? "i32 " : "void ")
           << "@" << name << " (";
        bool is_first = true;
        for (auto &p:params) {
            if (!is_first)
                os << ", ";
            is_first = false;
            if (p->decl->is_array()) {
                print_llvm_arr_inner_decl(os,p->decl->array_dims,1);
                os << "* ";
                os << get_name_of_value(p, p->decl->name);
            } else os <<"i32 "<< get_name_of_value(p, p->decl->name);

        }
        os << ")";

        if (is_extern()) {
            os << ";" << endl;
        } else {
            os << " {" << endl;
            //body
            for (auto &bb:bList) {
                bb->print(os);
            }
            os << "}" << endl;
        }
    }

    void BasicBlock::print(std::ostream &os) const {
        std::ostringstream comment;
        os << get_name_of_BB(this) << ":";
        if (!parentInsts.empty()) {
            comment << " preds = ";
            bool is_first = true;
            for (auto &i:parentInsts) {
                if (!is_first) comment << ", ";
                is_first = false;
                comment << "%" << get_name_of_BB(i->bb);
            }
        }
        if (idom)
            comment << " idom = %" << get_name_of_BB(idom);
        if (dom_tree_depth >= 0)
            comment << " dom_tree_depth = " << dom_tree_depth;
        if (!comment.str().empty())
            os << "\t;" << comment.str();

        os << "\t; loop_deep = " << loop_deep;

        os << std::endl;
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
            auto par = dynamic_cast<ir::GetElementPtrInst *>(p.value);
            if (par){
                par->print_llvm_type(os,par->dims.size());
                os<<"* ";
            }
            else os << "i32 ";
            os << get_name_of_value(p.value);
        }
        os << ")";
    }

    void BinaryInst::print(std::ostream &os) const {
        os << get_name_of_value((Value *) this) << " = ";
        Value::print(os);
        if (optype == OpType::ADD || optype == OpType::SUB || optype == OpType::MUL || optype == OpType::SDIV
            || optype == OpType::SREM || optype == OpType::SLT || optype == OpType::SLE || optype == OpType::SGT ||
            optype == OpType::SGE ||
            optype == OpType::EQ || optype == OpType::NE) {
            os << "i32 " << get_name_of_value(ValueL.value) << ", " << get_name_of_value(ValueR.value);
        }
    }

    void BranchInst::print(std::ostream &os) const {
        bool need_i1_convert = true;
        auto cond_val = dynamic_cast<BinaryInst *>(cond.value);
        Value *i1_val = cond.value;
        if (cond_val) {
            OpType op = cond_val->optype;
            if (op == OpType::SLT || op == OpType::SLE || op == OpType::SGT || op == OpType::SGE ||
                op == OpType::EQ || op == OpType::NE)
                need_i1_convert = false;
        }
        if (need_i1_convert) {
            i1_val = new BinaryInst(OpType::NE, cond.value, IRBuilder::getConstant(0));
            i1_val->print(os);
            os << std::endl << "\t";
        }
        Value::print(os);
        os << "i1 " << get_name_of_value(i1_val) << ", label %" << get_name_of_BB(true_block) << ", label %"
           << get_name_of_BB(false_block);
    }

    void JumpInst::print(std::ostream &os) const {
        Value::print(os);
        os << "label %" << get_name_of_BB(to);
    }

    void ReturnInst::print(std::ostream &os) const {
        Value::print(os);
        if (val.value)
            os << "i32 " << get_name_of_value(val.value);
        else os << "void";
    }

    void StoreInst::print(std::ostream &os) const {
        Value::print(os);
        os << "i32 " << get_name_of_value(val.value) << ", i32* " << get_name_of_value(ptr.value);
    }

    void LoadInst::print(std::ostream &os) const {
        os << get_name_of_value((Value *) this) << " = ";
        Value::print(os);
        os << "i32, i32* " << get_name_of_value(ptr.value);
    }

    void GlobalVar::print(std::ostream &os) const {
        os << get_name_of_value((Value *) this) << " = ";
        Value::print(os);
        if (decl->is_array()) {
            print_llvm_arr_decl(os, decl, {}, 0, decl->array_multipliers[0] - 1);
        } else {
            os << "i32 ";
            if (!initval.empty())
                os << initval[0];
            else os << "zeroinitializer";
        }
    }

    void AllocaInst::print(std::ostream &os) const {
        os << get_name_of_value((Value *) this) << " = ";
        Value::print(os);
        print_llvm_arr_inner_decl(os,decl->array_dims,0);
        //TODO: initialize imcomplete
    }
    void GetElementPtrInst::print_llvm_type(std::ostream &os, int start_dim) const {
        if (auto arr_val = dynamic_cast<AllocaInst *>(arr.value)) {
            print_llvm_arr_inner_decl(os,arr_val->decl->array_dims,start_dim);

        } else if (auto arr_val = dynamic_cast<GlobalVar *>(arr.value)) {
            print_llvm_arr_inner_decl(os,arr_val->decl->array_dims,start_dim);

        } else if(auto arr_val = dynamic_cast<FuncParam *>(arr.value)) {
            print_llvm_arr_inner_decl(os,arr_val->decl->array_dims,start_dim);
        }
    }
    void GetElementPtrInst::print(std::ostream &os) const {
        os << get_name_of_value((Value *) this) << " = ";
        Value::print(os);
        if (auto arr_val = dynamic_cast<AllocaInst *>(arr.value)) {
            print_llvm_type(os,0);
            os<<", ";
            print_llvm_type(os,0);
            os<<"* ";
            os<<get_name_of_value((Value*)arr_val)<<", ";
            os<<"i32 0";
            for(auto &i:dims){
                os<<", ";
                os<<"i32 "<<get_name_of_value(i.value);
            }
        } else if (auto arr_val = dynamic_cast<GlobalVar *>(arr.value)) {
            print_llvm_type(os,0);
            os<<", ";
            print_llvm_type(os,0);
            os<<"* ";
            os<<get_name_of_value((Value*)arr_val)<<", ";
            os<<"i32 0";
            for(auto &i:dims){
                os<<", ";
                os<<"i32 "<<get_name_of_value(i.value);
            }
        } else if(auto arr_val = dynamic_cast<FuncParam *>(arr.value)) {
            print_llvm_type(os,1);
            os<<", ";
            print_llvm_type(os,1);
            os<<"* ";
            os<<get_name_of_value((Value*)arr_val)<<", ";
            bool is_first = true;
            for(auto &i:dims){
                if(!is_first)
                    os<<", ";
                is_first = false;
                os<<"i32 "<<get_name_of_value(i.value);
            }
        }

    }

    void PhiInst::print(std::ostream &os) const {
        os << get_name_of_value((Value *) this) << " = ";
        Value::print(os);
        os << "i32 ";
        bool is_first = true;
        for (auto &i:phicont) {
            if (!is_first) os << ", ";
            is_first = false;
            os << "[ " << get_name_of_value(i.second->value) << ", %" << get_name_of_BB(i.first) << " ]";

        }
    }
}