#include <ast/ast.h>
#include <ast/validation.h>
#include <sstream>

namespace ast {

    void Node::validate(ValidationContext &ctx) {}

    void CompUnit::validate(ValidationContext &ctx) {
        Decl *decl;
        Function *func;
        for (const auto &node : this->entries) {
            if ((decl = dynamic_cast<Decl *>(node)));
            else if ((func = dynamic_cast<Function *>(node)))
                ctx.symbol_table.InsertFunc(func->name, func);
            else
                throw std::runtime_error("Expect Decl or FuncDef, but found: emm");
            node->validate(ctx);
        }
    }

    void Decl::validate(ValidationContext &ctx) {

        if (is_array()) {
            // array
        }
        if (initval)
            initval->validate(ctx);

        if (is_const && !(initval && initval->is_const))
            throw std::runtime_error("Const variable isn't initialized with const values");

        if (ctx.symbol_table.InsertVar(name, this))
            throw std::runtime_error(name + " is redefined.");
    }

    void InitVal::validate(ValidationContext &ctx) {
        if (exp) {
            exp->validate(ctx);
            is_const = exp->op == Exp::Op::CONST_VAL;
        } else {
            is_const = true;
            for (auto i:vals) {
                i->validate(ctx);
                if (!i->is_const)
                    is_const = false;
            }
        }
    }

    void LVal::validate(ValidationContext &ctx) {
        Decl *d = ctx.symbol_table.GetVar(name);
        if (!d)
            throw std::runtime_error("Unresolved lval: " + name);
        this->decl = d;
    }

    void Exp::validate(ValidationContext &ctx) {
        Node::validate(ctx);
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
            this->block->validate(ctx);
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

    void ReturnStmt::validate(ValidationContext &ctx) {
        if (this->ret)
            this->ret->validate(ctx);
    }

    void EvalStmt::validate(ValidationContext &ctx) {
        this->exp->validate(ctx);
    }
}
