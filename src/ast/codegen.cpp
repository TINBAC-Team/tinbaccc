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

    ir::Value *LVal::resolve_addr(ir::IRBuilder &builder) {
        ir::Value *offset_val = builder.getConstant(0);
        size_t array_dim_cnt = array_dims.size();
        for (size_t i = 0; i < array_dim_cnt; i++) {
            dim_value.push_back(array_dims[i]->codegen(builder));
            ir::Value *cur_dim_multiplier = builder.getConstant(
                    i == array_dim_cnt - 1 ? 1 : decl->array_multipliers[i + 1]);
            ir::Value *cur_dim_offset = builder.CreateBinaryInst(dim_value.back(), cur_dim_multiplier,
                                                                 ir::OpType::MUL);
            offset_val = builder.CreateBinaryInst(offset_val, cur_dim_offset, ir::OpType::ADD);
        }
        return builder.CreateGetElementPtrInst(decl->addr, offset_val);
    }

    ir::Value *LVal::codegen(ir::IRBuilder &builder) {
        if (decl->is_array()) {
            size_t req_size = array_dims.size();
            size_t decl_size = decl->array_dims.size();

            auto ptr = resolve_addr(builder);

            if (req_size == decl_size)
                return builder.CreateLoadInst(ptr);
            else return ptr;
        }

        return builder.GetCurBlock()->getVariable(decl, builder);
    }

    ir::Value *Decl::codegen(ir::IRBuilder &builder) {
        if (is_array()) {
            addr = builder.CreateAllocaInst(array_multipliers[0]);
            if (initval);// TODO: fill initial values somehow
            return addr;
        } else if (initval) {
            ir::Value *val = initval_expanded[0]->codegen(builder);
            set_var_def(builder.GetCurBlock(), val);
            return val;
        }
        return nullptr;
    }

    ir::Value *Cond::codegen(ir::IRBuilder &builder) {
        return exp->codegen(builder);
    }

    ir::Value *FuncCall::codegen(ir::IRBuilder &builder) {}

    ir::Value *Function::codegen(ir::IRBuilder &builder) {
        ir::Function *irFunc = builder.CreateFunction();
        irFunc->setupParams(params);
        // LLVM requires that the first block must have no predecessors. Create it here.
        builder.CreateBlock();
        return block->codegen(builder);
    }

    ir::Value *Block::codegen(ir::IRBuilder &builder) {
        for (auto i:entries)
            i->codegen(builder);
        // Block doesn't produce a single value. It produces several BBs instead.
        return nullptr;
    }

    ir::Value *AssignmentStmt::codegen(ir::IRBuilder &builder) {
        ir::Value *val = exp->codegen(builder);
        if (lval->decl->is_array()) {
            auto ptr = lval->resolve_addr(builder);
            return builder.CreateStoreInst(ptr, exp->codegen(builder));
        }

        // Local Value Numbering: save its current defining IR
        lval->decl->set_var_def(builder.GetCurBlock(), val);
        return val;
    }

    ir::Value *EvalStmt::codegen(ir::IRBuilder &builder) {
        return exp->codegen(builder);
    }

    ir::Value *IfStmt::codegen(ir::IRBuilder &builder) {}

    ir::Value *WhileStmt::codegen(ir::IRBuilder &builder) {}

    ir::Value *BreakStmt::codegen(ir::IRBuilder &builder) {}

    ir::Value *ContinueStmt::codegen(ir::IRBuilder &builder) {}

    ir::Value *ReturnStmt::codegen(ir::IRBuilder &builder) {
        if(ret)
            return builder.CreateReturnInst(ret->codegen(builder));
        return builder.CreateReturnInst(nullptr);
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
                return lval->codegen(builder);
            case Op::FuncCall:
                return funccall->codegen(builder);
            default:
                throw std::runtime_error("Invalid op, exp codegen failed.");
        }
    }
}