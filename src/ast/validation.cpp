#include <ast/ast.h>
#include <ast/validation.h>
#include <sstream>

namespace ast {

    void Node::validate(ValidationContext &ctx) {}

    void CompUnit::validate(ValidationContext &ctx) {
        Decl *decl;
        Function *func;
        for (const auto &node : this->entries) {
            if ((decl = dynamic_cast<Decl *>(node)))
                decl->set_global();
            else if ((func = dynamic_cast<Function *>(node)))
                ctx.symbol_table.InsertFunc(func->name, func);
            else
                throw std::runtime_error("Expect Decl or FuncDef, but found: emm");
            node->validate(ctx);
        }
    }

    void Decl::validate(ValidationContext &ctx) {
        if (initval)
            initval->validate(ctx);

        if (is_const && !(initval && initval->is_const))
            throw std::runtime_error("Const variable isn't initialized with const values");

        if (is_global && initval && !initval->is_const)
            throw std::runtime_error("Global variable isn't initialized with const values");

        if (is_array()) {
            // deal with multipliers first
            array_multipliers.resize(array_dims.size());
            int mul = 1;
            for (int i = array_dims.size() - 1; i >= 0; i--) {
                array_dims[i]->validate(ctx);
                mul *= array_dims[i]->get_value();
                if (mul)
                    array_multipliers[i] = mul;
                else if (!is_fparam)
                    throw std::runtime_error("zero-length array isn't allowed.");
            }

            // expand initval
            if (initval)
                expand_array();
        } else if (initval) {
            if (initval->exp)
                initval_expanded.emplace_back(initval->exp);
            else
                throw std::runtime_error("single variable initialized with array initializer.");
        }

        if (ctx.symbol_table.InsertVar(name, this))
            throw std::runtime_error(name + " is redefined.");
    }

    void Decl::expand_array() {
        initval_expanded.resize(array_multipliers[0]);
        int offset = 0;
        initval->fill_array(0, offset, array_multipliers, initval_expanded);
    }

    void InitVal::validate(ValidationContext &ctx) {
        if (exp) {
            exp->validate(ctx);
            is_const = exp->is_const();
        } else {
            is_const = true;
            for (auto i:vals) {
                i->validate(ctx);
                if (!i->is_const)
                    is_const = false;
            }
        }
    }

    void InitVal::fill_array(int dim, int &offset, const std::vector<int> &dims, std::vector<Exp *> &dst_vals) {
        int block_offs = offset / dims[dim];
        int block_start = block_offs * dims[dim];
        int block_end = (block_offs + 1) * dims[dim];
        int block_size = (dim == dims.size() - 1 ? 1 : dims[dim + 1]);
        if (exp)
            throw std::runtime_error("array initialized with a single variable.");
        if (block_start != offset)
            throw std::runtime_error("array isn't filled at it's start boundary, check the code!");
        for (auto i:vals) {
            if (offset >= block_end)
                throw std::runtime_error("too many values.");
            if (i->exp) {
                dst_vals[offset++] = i->exp;
            } else {
                while (offset % block_size)
                    dst_vals[offset++] = nullptr;
                i->fill_array(dim + 1, offset, dims, dst_vals);
            }
        }
        while (offset < block_end)
            dst_vals[offset++] = nullptr;
    }

    void LVal::validate(ValidationContext &ctx) {
        Decl *d = ctx.symbol_table.GetVar(name);
        if (!d)
            throw std::runtime_error("Unresolved lval: " + name);
        this->decl = d;

        if (d->array_dims.size() < array_dims.size())
            throw std::runtime_error("invalid array dimensions");

        for (auto i:array_dims)
            i->validate(ctx);
    }

    bool LVal::is_const() {
        for (auto &i:array_dims)
            if (i && !i->is_const())
                return false;
        return decl->is_const;
    }

    int LVal::get_value() {
        int offset = 0;
        int i;
        if (array_dims.empty())
            return decl->get_value(0);
        for (i = 0; i < array_dims.size() - 1; i++)
            offset += (array_dims[i]->get_value()) * (decl->array_multipliers[i + 1]);
        offset += array_dims[i]->get_value();
        return decl->get_value(offset);
    }

    void Exp::validate(ValidationContext &ctx) {
        bool const_folding_possible = true;
        if (lhs) {
            lhs->validate(ctx);
            if (!lhs->is_const())
                const_folding_possible = false;
        }
        if (rhs) {
            rhs->validate(ctx);
            if (!rhs->is_const())
                const_folding_possible = false;
        }
        if (op == Op::LVAL) {
            lval->validate(ctx);
            if (!lval->is_const())
                const_folding_possible = false;
        }

        if (op == Op::FuncCall) {
            funccall->validate(ctx);
            return;
        }

        if (const_folding_possible) {
            bool const_folding_done = true;
            switch (op) {
                case Op::UNARY_PLUS:
                    const_val = lhs->get_value();
                    break;
                case Op::UNARY_MINUS:
                    const_val = -(lhs->get_value());
                    break;
                case Op::LOGIC_NOT:
                    const_val = !(lhs->get_value());
                    break;
                case Op::PLUS:
                    const_val = lhs->get_value() + rhs->get_value();
                    break;
                case Op::MINUS:
                    const_val = lhs->get_value() - rhs->get_value();
                    break;
                case Op::MUL:
                    const_val = lhs->get_value() * rhs->get_value();
                    break;
                case Op::DIV:
                    const_val = lhs->get_value() / rhs->get_value();
                    break;
                case Op::MOD:
                    const_val = lhs->get_value() % rhs->get_value();
                    break;
                case Op::LESS_THAN:
                    const_val = (lhs->get_value() < rhs->get_value());
                    break;
                case Op::LESS_EQ:
                    const_val = (lhs->get_value() <= rhs->get_value());
                    break;
                case Op::GREATER_THAN:
                    const_val = (lhs->get_value() > rhs->get_value());
                    break;
                case Op::GREATER_EQ:
                    const_val = (lhs->get_value() >= rhs->get_value());
                    break;
                case Op::EQ:
                    const_val = (lhs->get_value() == rhs->get_value());
                    break;
                case Op::INEQ:
                    const_val = (lhs->get_value() != rhs->get_value());
                    break;
                case Op::LOGIC_AND:
                    const_val = (lhs->get_value() && rhs->get_value());
                    break;
                case Op::LOGIC_OR:
                    const_val = (lhs->get_value() || rhs->get_value());
                    break;
                case Op::LVAL:
                    const_val = lval->get_value();
                    break;
                default:
                    const_folding_done = false;
            }
            if (const_folding_done)
                op = Op::CONST_VAL;
        }
    }

    void Cond::validate(ValidationContext &ctx) {
        exp->validate(ctx);
    }

    void FuncCall::validate(ValidationContext &ctx) {
        Function *func = ctx.symbol_table.GetFunc(name);
        if (!func)
            throw std::runtime_error("Unresolved function: " + name);
        if (func->params.size() != func->params.size())
            throw std::runtime_error("Incorrect number of parameters, expect "
                                     + std::to_string(func->params.size()) + ", but get: " +
                                     std::to_string(this->params.size()));
        is_void = func->type == ast::Function::Type::VOID;
        for (const auto &node : this->params)
            node->validate(ctx);
    }

    void Function::validate(ValidationContext &ctx) {
        // FIXME: Function params should be in the same scope as function body!
        ctx.symbol_table.EnterScope();
        try {
            for (const auto &node : this->params) {
                node->validate(ctx);
            }
            if(block)
                block->validate(ctx);
        } catch (std::runtime_error &ex) {
            // make sure leave scope
            ctx.symbol_table.ExitScope();
            throw ex;
        }
        ctx.symbol_table.ExitScope();
        ctx.symbol_table.InsertFunc(name, this);
    }

    void Block::validate(ValidationContext &ctx) {
        ctx.symbol_table.EnterScope();
        for (const auto &node : this->entries) {
            node->validate(ctx);
        }
        ctx.symbol_table.ExitScope();
    }

    void AssignmentStmt::validate(ValidationContext &ctx) {
        this->exp->validate(ctx);
        this->lval->validate(ctx);
//        ctx.symbol_table.InsertVar(this->lval->name, );
    }

    void IfStmt::validate(ValidationContext &ctx) {
        this->cond->validate(ctx);
        if (this->true_block)
            this->true_block->validate(ctx);

        if (this->false_block)
            this->false_block->validate(ctx);
    }

    void WhileStmt::validate(ValidationContext &ctx) {
        cond->validate(ctx);
        if (block)
            block->validate(ctx);
    }

    void ReturnStmt::validate(ValidationContext &ctx) {
        if (this->ret)
            this->ret->validate(ctx);
    }

    void EvalStmt::validate(ValidationContext &ctx) {
        this->exp->validate(ctx);
    }
}
