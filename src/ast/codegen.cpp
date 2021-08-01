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
        size_t decl_array_dim_cnt = decl->array_dims.size();
        size_t array_dim_cnt = array_dims.size();
        if (array_dim_cnt > decl_array_dim_cnt)
            throw std::runtime_error("Referencing an array with incorrect dimension.");
        std::vector<ir::Value *> dim_value;
        for (auto &i:array_dims) {
            dim_value.emplace_back(i->codegen(builder));
        }
        return builder.CreateGetElementPtrInst(decl->addr, dim_value, decl->array_multipliers);
    }

    ir::Value *LVal::codegen(ir::IRBuilder &builder) {
        if (decl->is_array()) {
            size_t req_size = array_dims.size();
            size_t decl_size = decl->array_dims.size();

            auto ptr = resolve_addr(builder);

            if (req_size == decl_size)
                return builder.CreateLoadInst(ptr);
            else return ptr;
        } else if (decl->is_global) {
            return builder.CreateLoadInst(decl->addr);
        }
        return builder.GetCurBlock()->getVariable(decl, builder);
    }

    void store_stack_array_initval(Decl *decl, ir::IRBuilder &builder, std::vector<int> &dim,int offset) {
        if(dim.size() == decl->array_dims.size())
        {
            if(decl->initval_expanded[offset])
            {
                if(decl->initval_expanded[offset]->is_const() && decl->initval_expanded[offset]->get_value()==0) return;
                std::vector<ir::Value*> dim_val;
                for(auto &i:dim){
                    dim_val.push_back(builder.getConstant(i));
                }
                auto filling_addr = builder.CreateGetElementPtrInst(decl->addr,dim_val, decl->array_multipliers);
                builder.CreateStoreInst(filling_addr,decl->initval_expanded[offset]->codegen(builder));
            }
            return;
        }
        for (int i = 0; i < decl->array_dims[dim.size()]->get_value(); i++) {
            dim.emplace_back(i);
            store_stack_array_initval(decl, builder, dim,offset);
            dim.pop_back();
            if (dim.size() == decl->array_dims.size() - 1) {
                offset += 1;
            } else {
                offset += decl->array_multipliers[dim.size() + 1];
            }
        }
    }

    ir::Value *Decl::codegen(ir::IRBuilder &builder) {
        if (is_array()) {
            addr = builder.CreateAllocaInst(this);
            if (initval) {
                int index = -1;
                std::vector<ast::Exp *> memset_params;
                auto memset_target = new Exp(new LVal(this));
                for(size_t i=0;i<memset_target->lval->decl->array_dims.size()-1;i++)
                {
                    memset_target->lval->array_dims.emplace_back(new Exp(0));
                }
                memset_params.emplace_back(memset_target);
                memset_params.emplace_back(new Exp(0));
                memset_params.emplace_back(new Exp(array_multipliers[0] * 4));
                builder.CreateFuncCall("memset", true, memset_params);
                memset_target->lval->array_dims.clear();
                std::vector<int> dim;
                store_stack_array_initval(this,builder,dim,0);
            }
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

    ir::Value *FuncCall::codegen(ir::IRBuilder &builder) {
        return builder.CreateFuncCall(name, is_void, params);

    }

    ir::Value *Function::codegen(ir::IRBuilder &builder) {
        ir::Function *irFunc = builder.CreateFunction(name, type == Type::INT);
        irFunc->setupParams(params);

        // TODO: external functions?
        if(!block)
            return nullptr;
        // LLVM requires that the first block must have no predecessors. Create it here.
        ir::BasicBlock *bb = builder.CreateBlock();
        irFunc->addParamsToBB(bb);
        auto ret= block->codegen(builder);
        if(irFunc->return_int)
            builder.CreateReturnInst(builder.getConstant(0));
        else builder.CreateReturnInst(nullptr);
        return ret;
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
            return builder.CreateStoreInst(ptr, val);
        } else if (lval->decl->is_global) {
            return builder.CreateStoreInst(lval->decl->addr, val);
        }

        // Local Value Numbering: save its current defining IR
        lval->decl->set_var_def(builder.GetCurBlock(), val);
        return val;
    }

    ir::Value *EvalStmt::codegen(ir::IRBuilder &builder) {
        return exp->codegen(builder);
    }

    ir::Value *IfStmt::codegen(ir::IRBuilder &builder) {
        ir::BasicBlock *t_old = builder.TrueBlock, *f_old = builder.FalseBlock, *cont_old = builder.ContBlock;
        builder.ContBlock = new ir::BasicBlock();
        if(true_block)
            builder.TrueBlock = new ir::BasicBlock();
        else
            builder.TrueBlock = builder.ContBlock;

        if(false_block)
            builder.FalseBlock = new ir::BasicBlock();
        else
            builder.FalseBlock = builder.ContBlock;

        ir::Value *cond_val = cond->codegen(builder);
        builder.CreateBranchInst(cond_val, builder.TrueBlock, builder.FalseBlock);

        if(true_block) {
            builder.appendBlock(builder.TrueBlock);
            true_block->codegen(builder);
            builder.CreateJumpInst(builder.ContBlock);
        }

        if(false_block) {
            builder.appendBlock(builder.FalseBlock);
            false_block->codegen(builder);
            builder.CreateJumpInst(builder.ContBlock);
        }

        builder.appendBlock(builder.ContBlock);
        builder.TrueBlock = t_old;
        builder.FalseBlock = f_old;
        builder.ContBlock = cont_old;

        return nullptr;
    }

    ir::Value *WhileStmt::codegen(ir::IRBuilder &builder) {
        ir::BasicBlock *t_old = builder.TrueBlock;
        ir::BasicBlock *f_old = builder.FalseBlock;
        ir::BasicBlock *cont_old = builder.WhileContBlock;
        ir::BasicBlock *e_old = builder.EntryBlock;
        builder.EntryBlock = new ir::BasicBlock();
        builder.CreateJumpInst(builder.EntryBlock);
        builder.appendBlock(builder.EntryBlock);
        builder.WhileContBlock = new ir::BasicBlock();
        builder.TrueBlock = new ir::BasicBlock();
        builder.FalseBlock = builder.WhileContBlock;

        // incomplete CFG: TrueBlock can enter EntryBlock after its execution
        builder.EntryBlock->sealed = false;
        ir::Value *cond_val = cond->codegen(builder);
        builder.CreateBranchInst(cond_val, builder.TrueBlock, builder.FalseBlock);

        builder.appendBlock(builder.TrueBlock);
        block->codegen(builder);
        builder.CreateJumpInst(builder.EntryBlock);
        builder.EntryBlock->sealBlock(builder);
        builder.appendBlock(builder.WhileContBlock);
        builder.EntryBlock = e_old;
        builder.WhileContBlock = cont_old;
        builder.FalseBlock = f_old;
        builder.TrueBlock = t_old;
        return nullptr;
    }

    ir::Value *BreakStmt::codegen(ir::IRBuilder &builder) {
        if(!builder.EntryBlock)
            throw std::runtime_error("break outside a loop.");
        auto ret = builder.CreateJumpInst(builder.WhileContBlock);
        // new BB after Jump
        builder.CreateBlock();
        return ret;
    }

    ir::Value *ContinueStmt::codegen(ir::IRBuilder &builder) {
        if(!builder.EntryBlock)
            throw std::runtime_error("continue outside a loop.");
        auto ret = builder.CreateJumpInst(builder.EntryBlock);
        // new BB after Jump
        builder.CreateBlock();
        return ret;
    }

    ir::Value *ReturnStmt::codegen(ir::IRBuilder &builder) {
        ir::Value *retInst = builder.CreateReturnInst(ret ? ret->codegen(builder) : nullptr);
        builder.CreateBlock();
        return retInst;
    }

    ir::Value *Exp::codegen(ir::IRBuilder &builder) {

        ir::Value *L = nullptr, *R = nullptr;
        switch (op) {
            case Op::CONST_VAL:
                return builder.getConstant(get_value());
            case Op::LVAL:
                return lval->codegen(builder);
            case Op::FuncCall:
                return funccall->codegen(builder);
            case Op::LOGIC_AND:
                return codegen_and(builder);
            case Op::LOGIC_OR:
                return codegen_or(builder);
            default:
                break;
        }
        if (lhs->is_const())
            L = builder.getConstant(lhs->get_value());
        else
            L = lhs->codegen(builder);
        if (rhs) {
            if (rhs->is_const())
                R = builder.getConstant(rhs->get_value());
            else
                R = rhs->codegen(builder);
        }

        if(op==Op::UNARY_MINUS || op==Op::UNARY_PLUS || op==Op::LOGIC_NOT)
        {
            if (!L) return nullptr;
        } else if (!L || !R) return nullptr;

        switch (op) {
            case Op::UNARY_PLUS:
                return L;
            case Op::UNARY_MINUS:
                return builder.CreateBinaryInst(builder.getConstant(0) ,L, ir::OpType::SUB);
            case Op::LOGIC_NOT:
                return builder.CreateBinaryInst(L, builder.getConstant(0), ir::OpType::EQ);

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
            default:
                throw std::runtime_error("Invalid op, exp codegen failed.");
        }
    }

    ir::Value * Exp::codegen_and(ir::IRBuilder &builder) {
        ir::BasicBlock *t_old = builder.TrueBlock;
        builder.TrueBlock = new ir::BasicBlock();
        //builder.TrueBlock->sealed = false;
        ir::Value *cond_val = lhs->codegen(builder);
        builder.CreateBranchInst(cond_val, builder.TrueBlock, builder.FalseBlock);
        builder.appendBlock(builder.TrueBlock);
        //builder.TrueBlock->sealBlock(builder);
        builder.TrueBlock = t_old;
        return rhs->codegen(builder);
    }

    ir::Value * Exp::codegen_or(ir::IRBuilder &builder) {
        ir::BasicBlock *f_old = builder.FalseBlock;
        builder.FalseBlock = new ir::BasicBlock();
        ir::Value *cond_val = lhs->codegen(builder);
        builder.CreateBranchInst(cond_val, builder.TrueBlock, builder.FalseBlock);
        builder.appendBlock(builder.FalseBlock);
        builder.FalseBlock = f_old;
        return rhs->codegen(builder);
    }
}