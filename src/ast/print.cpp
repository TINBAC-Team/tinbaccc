#include <ast/ast.h>
#include <fstream>

namespace ast {
    static int count;

    void CompUnit::print(std::ofstream &ofd) {
        count = 0;
        ofd << "digraph g{\n";
        int count_compunit = count;
        ofd << "\tnode" << count_compunit << "[label=\"CompUnit\"];\n";
        Decl* enn = nullptr;
        Function* enn_ = nullptr;
        for (auto en = entries.begin(); en != entries.end(); en++) {
            enn = dynamic_cast<Decl*>(*en);
            if (enn) {
                ofd << "\tnode" << ++count << "[label=\"Decl\"];\n";
                ofd << "\tnode" << count_compunit << "->node" << count << ";\n";
                enn->print(ofd);
            }
            else {
                enn_ = dynamic_cast<Function*>(*en);
                ofd << "\tnode" << ++count << "[label=\"Function\"];\n";
                ofd << "\tnode" << count_compunit << "->node" << count << ";\n";
                enn_->print(ofd);
            }
        }
        ofd << "}\n";
    }

    void InitVal::print(std::ofstream &ofd) {
        int count_initval = count;
        ofd << "\tnode" << count_initval << "[label=\"InitVal\"];\n";
        if (exp != nullptr) {
            ofd << "\tnode" << ++count << "[label=\"Exp\"];\n";
            ofd << "\tnode" << count_initval << "->node" << count << ";\n";
            exp->print(ofd);
        }
        else {
            for (auto it = vals.cbegin(); it != vals.cend(); it++) {
                ofd << "\tnode" << ++count << "[label=\"InitVal\"];\n";
                ofd << "\tnode" << count_initval << "->node" << count << ";\n";
                (*it)->print(ofd);
            }
        }
    }

    void LVal::print(std::ofstream &ofd) {
        int count_l = count;
        ofd << "\tnode" << count_l << "[label=\"LVal\"];\n";
        ofd << "\tnode" << ++count << "[label=\"" << name << "\"];\n";
        ofd << "\tnode" << count_l << "->node" << count << ";\n";
        for (auto it = array_dims.cbegin(); it !=array_dims.cend(); it++) {
            ofd << "\tnode" << ++count << "[label=\"Exp\"];\n";
            ofd << "\tnode" << count_l << "->node" << count << ";\n";
            (*it)->print(ofd);
        }
    }

    void Decl::print(std::ofstream &ofd) {
        int count_decl = count;
        ofd << "\tnode" << count_decl << "[label=\"Decl\"];\n";
        if (is_const) {
            ofd << "\tnode" << ++count << "[label=\"const\"];\n";
            ofd << "\tnode" << count_decl << "->node" << count << ";\n";
        }
        ofd << "\tnode" << ++count << "[label=\"int\"];\n";
        ofd << "\tnode" << count_decl << "->node" << count << ";\n";
        ofd << "\tnode" << ++count << "[label=\"" << name << "\"];\n";
        ofd << "\tnode" << count_decl << "->node" << count << ";\n";
        if (is_array()) {
            for (auto it = array_dims.cbegin(); it != array_dims.cend(); it++) {
                ofd << "\tnode" << ++count << "[label=\"[\"];\n";
                ofd << "\tnode" << count_decl << "->node" << count << ";\n";
                ofd << "\tnode" << ++count << "[label=\"Exp\"];\n";
                ofd << "\tnode" << count_decl << "->node" << count << ";\n";
                (*it)->print(ofd);
                ofd << "\tnode" << ++count << "[label=\"]\"];\n";
                ofd << "\tnode" << count_decl << "->node" << count << ";\n";
            }
        }
        if (initval) {
            ofd << "\tnode" << ++count << "[label=\"=\"];\n";
            ofd << "\tnode" << count_decl << "->node" << count << ";\n";
            ofd << "\tnode" << ++count << "[label=\"InitVal\"];\n";
            ofd << "\tnode" << count_decl << "->node" << count << ";\n";
            initval->print(ofd);
        }
        else {

        }
    }

    std::string Exp::op_real() {
        switch(op) {
            case Op::MUL:
                return "*";
            case Op::DIV:
                return "/";
            case Op::EQ:
                return "==";
            case Op::LESS_EQ:
                return "<=";
            case Op::LESS_THAN:
                return "<";
            case Op::MOD:
                return "%";
            case Op::GREATER_EQ:
                return ">=";
            case Op::GREATER_THAN:
                return ">";
            case Op::MINUS:
                return "-";
            case Op::PLUS:
                return "+";
            case Op::LOGIC_OR:
                return "||";
            case Op::LOGIC_NOT:
                return "!";
            case Op::INEQ:
                return "!=";
            case Op::LOGIC_AND:
                return "&&";
            default:
                return "???";
        }
    }

    void Exp::print(std::ofstream &ofd) {
        int count_exp = count;
        ofd << "\tnode" << count_exp << "[label=\"Exp\"];\n";
        switch (op) {
            case Op::CONST_VAL:
                ofd << "\tnode" << ++count << "[label=\"" << const_val << "\"];\n";
                ofd << "\tnode" << count_exp << "->node" << count << ";\n";
                break;
            case Op::LVAL:
                ofd << "\tnode" << ++count << "[label=\"LVal\"];\n";
                ofd << "\tnode" << count_exp << "->node" << count << ";\n";
                lval->print(ofd);
                break;
            case Op::FuncCall:
                ofd << "\tnode" << ++count << "[label=\"FuncCall\"];\n";
                ofd << "\tnode" << count_exp << "->node" << count << ";\n";
                funccall->print(ofd);
                break;
            default:
                ofd << "\tnode" << ++count << "[label=\"Exp\"];\n";
                ofd << "\tnode" << count_exp << "->node" << count << ";\n";
                lhs->print(ofd);
                ofd << "\tnode" << ++count << "[label=\"" << op_real() << "\"];\n";
                ofd << "\tnode" << count_exp << "->node" << count << ";\n";
                ofd << "\tnode" << ++count << "[label=\"Exp\"];\n";
                ofd << "\tnode" << count_exp << "->node" << count << ";\n";
                rhs->print(ofd);
                break;
        }
    }

    void Cond::print(std::ofstream &ofd) {
        int count_cond = count;
        ofd << "\tnode" << count_cond << "[label=\"Cond\"];\n";
        ofd << "\tnode" << ++count << "[label=\"Exp\"];\n";
        ofd << "\tnode" << count_cond << "->node" << count << ";\n";
        exp->print(ofd);
    }

    void FuncCall::print(std::ofstream &ofd) {
        int count_funcall = count;
        ofd << "\tnode" << count_funcall << "[label=\"FuncCall\"];\n";
        ofd << "\tnode" << ++count << "[label=\"" << name << "\"];\n";
        ofd << "\tnode" << count_funcall << "->node" << count << ";\n";
        if (!params.empty()) {
            for (auto it = params.cbegin(); it != params.cend(); it++) {
                ofd << "\tnode" << ++count << "[label=\"FuncFParam\"];\n";
                ofd << "\tnode" << count_funcall << "->node" << count << ";\n";
                (*it)->print(ofd);
            }
        }
    }

    void FuncFParam::print(std::ofstream &ofd) {
        int count_fpm = count;
        ofd << "\tnode" << count_fpm << "[label=\"FuncFParam\"];\n";
        if (type == Decl::VarType::INT)
            ofd << "\tnode" << ++count << "[label=\"int\"];\n";
        else
            ofd << "\tnode" << ++count << "[label=\"char\"];\n";
        ofd << "\tnode" << count_fpm << "->node" << count << ";\n";
        ofd << "\tnode" << ++count << "[label=\"LVal\"];\n";
        ofd << "\tnode" << count_fpm << "->node" << count << ";\n";
        signature->print(ofd);
    }

    void Function::print(std::ofstream &ofd) {
        int count_func = count;
        ofd << "\tnode" << count_func << "[label=\"Function\"];\n";
        if (type == Type::VOID)
            ofd << "\tnode" << ++count << "[label=\"void\"];\n";
        else
            ofd << "\tnode" << ++count << "[label=\"int\"];\n";
        ofd << "\tnode" << count_func << "->node" << count << ";\n";
        ofd << "\tnode" << ++count << "[label=\"" << name << "\"];\n";
        ofd << "\tnode" << count_func << "->node" << count << ";\n";
        if (!params.empty()) {
            for (auto it = params.cbegin(); it != params.cend(); it++) {
                ofd << "\tnode" << ++count << "[label=\"FuncFParam\"];\n";
                ofd << "\tnode" << count_func << "->node" << count << ";\n";
                (*it)->print(ofd);
            }
        }
        ofd << "\tnode" << ++count << "[label=\"Block\"];\n";
        ofd << "\tnode" << count_func << "->node" << count << ";\n";
        block->print(ofd);
    }

    void Block::print(std::ofstream &ofd) {
        int count_block = count;
        ofd << "\tnode" << count_block << "[label=\"Block\"];\n";
        Decl* enn = nullptr;
        Stmt* enn_ = nullptr;
        for (auto en = entries.begin(); en != entries.end(); en++) {
            enn = dynamic_cast<Decl*>(*en);
            if (enn) {
                ofd << "\tnode" << ++count << "[label=\"Decl\"];\n";
                ofd << "\tnode" << count_block << "->node" << count << ";\n";
                enn->print(ofd);
            }
            else {
                enn_ = dynamic_cast<Stmt*>(*en);
                ofd << "\tnode" << ++count << "[label=\"Stmt\"];\n";
                ofd << "\tnode" << count_block << "->node" << count << ";\n";
                enn_->print(ofd);
            }
        }
    }

    void AssignmentStmt::print(std::ofstream &ofd) {
        int count_asgn = count;
        ofd << "\tnode" << count_asgn << "[label=\"Stmt\"];\n";
        ofd << "\tnode" << ++count << "[label=\"LVal\"];\n";
        ofd << "\tnode" << count_asgn << "->node" << count << ";\n";
        lval->print(ofd);
        ofd << "\tnode" << ++count << "[label=\"=\"];\n";
        ofd << "\tnode" << count_asgn << "->node" << count << ";\n";
        ofd << "\tnode" << ++count << "[label=\"Exp\"];\n";
        ofd << "\tnode" << count_asgn << "->node" << count << ";\n";
        exp->print(ofd);
    }

    void EvalStmt::print(std::ofstream &ofd) {
        if (exp) {
            ofd << "\tnode" << ++count << "[label=\"Stmt\"];\n";
            ofd << "\tnode" << ++count << "[label=\"Exp\"];\n";
            ofd << "\tnode" << count - 1 << "->node" << count << ";\n";
            exp->print(ofd);
        }
    }

    void IfStmt::print(std::ofstream &ofd) {
        int count_if = count;
        ofd << "\tnode" << count_if << "[label=\"Stmt\"];\n";
        ofd << "\tnode" << ++count << "[label=\"if\"];\n";
        ofd << "\tnode" << count_if << "->node" << count << ";\n";
        ofd << "\tnode" << ++count << "[label=\"Cond\"];\n";
        ofd << "\tnode" << count_if << "->node" << count << ";\n";
        cond->print(ofd);
        ofd << "\tnode" << ++count << "[label=\"Stmt\"];\n";
        ofd << "\tnode" << count_if << "->node" << count << ";\n";
        true_block->print(ofd);
        if (false_block) {
            ofd << "\tnode" << ++count << "[label=\"else\"];\n";
            ofd << "\tnode" << count_if << "->node" << count << ";\n";
            ofd << "\tnode" << ++count << "[label=\"Stmt\"];\n";
            ofd << "\tnode" << count_if << "->node" << count << ";\n";
            false_block->print(ofd);
        }
    }

    void WhileStmt::print(std::ofstream &ofd) {
        int count_while = count;
        ofd << "\tnode" << count_while << "[label=\"Stmt\"];\n";
        ofd << "\tnode" << ++count << "[label=\"while\"];\n";
        ofd << "\tnode" << count_while << "->node" << count << ";\n";
        ofd << "\tnode" << ++count << "[label=\"Cond\"];\n";
        ofd << "\tnode" << count_while << "->node" << count << ";\n";
        cond->print(ofd);
        ofd << "\tnode" << ++count << "[label=\"Stmt\"];\n";
        ofd << "\tnode" << count_while << "->node" << count << ";\n";
        block->print(ofd);
    }

    void BreakStmt::print(std::ofstream &ofd) {
        ofd << "\tnode" << count << "[label=\"Stmt\"];\n";
        ofd << "\tnode" << ++count << "[label=\"break\"];\n";
        ofd << "\tnode" << count - 1 << "->node" << count << ";\n";
    }

    void ContinueStmt::print(std::ofstream &ofd) {
        ofd << "\tnode" << count << "[label=\"Stmt\"];\n";
        ofd << "\tnode" << ++count << "[label=\"continue\"];\n";
        ofd << "\tnode" << count - 1 << "->node" << count << ";\n";
    }

    void ReturnStmt::print(std::ofstream &ofd) {
        ofd << "\tnode" << count << "[label=\"Stmt\"];\n";
        ofd << "\tnode" << ++count << "[label=\"return\"];\n";
        ofd << "\tnode" << count - 1 << "->node" << count << ";\n";
        if (ret) {
            ofd << "\tnode" << ++count << "[label=\"Exp\"];\n";
            ofd << "\tnode" << count - 2 << "->node" << count << ";\n";
            ret->print(ofd);
        }
    }
}