#include "ast/ast.h"
#include "ir/ir.h"

namespace ast {

    ir::Value *CompUnit::codegen(ir::IRBuilder &builder) {
        Decl *decl;
        Function *func;
        for (auto node:entries) {
            if ((decl = dynamic_cast<Decl *>(node)))
                builder.CreateGlobalVar(decl);
            else if ((func = dynamic_cast<Function *>(node)))
                func->codegen(builder);
        }
        return nullptr; // There's no ir::Value* for a module
    }

    ir::Value * Function::codegen(ir::IRBuilder &builder) {
        return nullptr;
    }

    ir::Value *Exp::codegen(ir::IRBuilder &builder) {
        ir::Value *L = nullptr, *R = nullptr;
        if (!lhs->is_const())
            L = builder.getConstant(lhs->get_value());
        else L = lhs->codegen(builder);
        if (!rhs->is_const())
            R = builder.getConstant(rhs->get_value());
        else R = rhs->codegen(builder);

        if (!L || !R) return nullptr;
        switch (op) {
            case Op::UNARY_PLUS:
                return L;
            case Op::UNARY_MINUS:
                return builder.CreateBinaryInst(L, builder.getConstant(0), ir::OpType::SUB);
            case Op::LOGIC_NOT:
                return builder.CreateBinaryInst(L, builder.getConstant(0), ir::OpType::NE);

            case Op::PLUS:
                return builder.CreateBinaryInst(L, R, ir::OpType::ADD);
            case Op::MINUS:
                return builder.CreateBinaryInst(L, R, ir::OpType::SUB);
            case Op::MUL:
                return builder.CreateBinaryInst(L, R, ir::OpType::MUL);
            case Op::DIV:
                return builder.CreateBinaryInst(L, R, ir::OpType::SDIV);
            case Op::MOD:
                return builder.CreateBinaryInst(L, R, ir::OpType::SREM);

            case Op::LESS_THAN:
                return builder.CreateBinaryInst(L, R, ir::OpType::SLT);
            case Op::LESS_EQ:
                return builder.CreateBinaryInst(L, R, ir::OpType::SLE);
            case Op::GREATER_THAN:
                return builder.CreateBinaryInst(L, R, ir::OpType::SGT);
            case Op::GREATER_EQ:
                return builder.CreateBinaryInst(L, R, ir::OpType::SGE);
            case Op::EQ:
                return builder.CreateBinaryInst(L, R, ir::OpType::EQ);
            case Op::INEQ:
                return builder.CreateBinaryInst(L, R, ir::OpType::NE);

            case Op::LOGIC_AND:
                return builder.CreateBinaryInst(L, R, ir::OpType::AND);
            case Op::LOGIC_OR:
                return builder.CreateBinaryInst(L, R, ir::OpType::OR);

            case Op::CONST_VAL:
                return builder.getConstant(get_value());
            case Op::LVAL:
                return builder.CreateLoadInst(); //to be done
            case Op::FuncCall:
                return builder.CreateCallInst(); //to be done
            default:
                throw std::runtime_error("Invalid op, exp codegen failed.");
        }
    }
}