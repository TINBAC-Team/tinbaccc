#include <ast/ast.h>
#include <ast/validation.h>
#include <sstream>
namespace ast {

    void Node::validate(ValidationContext &ctx) {}

    void CompUnit::validate(ValidationContext &ctx) {
        Decl* decl;
        Function* func;
        for (const auto & node : this->entries) {
            if ((decl = dynamic_cast<Decl*>(node)))
                ctx.symbol_table.InsertVar(decl->name, decl);
            else if ((func = dynamic_cast<Function*>(node)))
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
        if (is_const && !initval)
            throw std::runtime_error("Const val isn't initialized");
        if (initval) {
            this->validate_array();
            this->expand_array();
        }
        ctx.symbol_table.InsertVar(name, this);
    }



    void InitVal::validate(ValidationContext &ctx) {
        // do nothing
    }

    void LVal::validate(ValidationContext &ctx) {
        Decl* d = ctx.symbol_table.GetVar(name);
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
        Function* func = ctx.symbol_table.GetFunc(name);
        if (!func)
            throw std::runtime_error("Unresolved function: " + name);
        if (func->params.size() != func->params.size())
            throw std::runtime_error("Incorrect number of parameters, expect "
            + std::to_string(func->params.size()) + ", but get: " + std::to_string(this->params.size()));
        for (const auto & node : this->params)
            node->validate(ctx);
    }

    void Function::validate(ValidationContext &ctx) {
        ctx.symbol_table.EnterScope();
        try {
            for (const auto & node : this->params) {
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
        for (const auto & node : this->entries) {
            node->validate(ctx);
        }
    }

    void AssignmentStmt::validate(ValidationContext &ctx) {
        this->exp->validate(ctx);
        this->lval->validate(ctx);
//        ctx.symbol_table.InsertVar(this->lval->name, );
    }

    void IfStmt::validate(ValidationContext &ctx) {
        this->cond->validate(ctx);
        ctx.symbol_table.EnterScope();
        try {
            this->true_block->validate(ctx);
        } catch (std::runtime_error &ex) {
            ctx.symbol_table.ExitScope();
            throw ex;
        }
        ctx.symbol_table.ExitScope();
        ctx.symbol_table.EnterScope();
        try {
            this->false_block->validate(ctx);
        } catch (std::runtime_error &ex) {
            ctx.symbol_table.ExitScope();
            throw ex;
        }
        ctx.symbol_table.ExitScope();
    }

    void ReturnStmt::validate(ValidationContext &ctx) {
        this->ret->validate(ctx);
    }

    void EvalStmt::validate(ValidationContext &ctx) {
        this->exp->validate(ctx);
    }
}
