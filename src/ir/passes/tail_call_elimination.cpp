/**
 * Tail Call Elimination
 *
 * This transforms calls of the current function (self recursion) followed by a return instruction
 * with a branch to the entry of the function, creating a loop.
 *
 * LLVM can do more than that: https://llvm.org/docs/Passes.html#tailcallelim-tail-call-elimination
 */

#include <ir/passes.h>
#include <ast/ast.h>
#include <iostream>

namespace ir_passes {
    static std::vector<ir::PhiInst *> tce_preparation(ir::Function *func) {
        std::vector<ir::PhiInst *> ret;
        ret.reserve(func->params.size());
        auto entry = func->bList.front();
        // 1. insert PHI for parameters into the first block
        for (auto &param : func->params) {
            // we can't print GEP from PHI yet. Skip them for easier testing.
            if (param->decl->is_array()) {
                ret.emplace_back(nullptr);
                continue;
            }

            auto phi = new ir::PhiInst();
            entry->InsertAtFront(phi);
            param->replaceWith(phi);
            ret.emplace_back(phi);
        }

        // 2. create new entry block
        auto new_entry = new ir::BasicBlock(0);
        func->bList.push_front(new_entry);
        new_entry->InsertAtEnd(new ir::JumpInst(entry));

        // 3. setup PHIs for params
        for (int i = 0; i < func->params.size(); i++)
            if (ret[i])
                ret[i]->InsertElem(new_entry, func->params[i]);

        return ret;
    }

    void tail_call_elimination(ir::Module *module) {
        for (auto &func:module->functionList) {
            static std::vector<ir::PhiInst *> param_map;
            bool tce_prepared = false;
            auto entry = func->bList.front();
            for (auto &bb:func->bList) {
                if (bb->iList.size() < 2)
                    continue;
                if (bb->iList.back()->optype != ir::OpType::RETURN)
                    continue;
                auto call = dynamic_cast<ir::CallInst *>(*std::next(bb->iList.rbegin()));
                if (!call || call->fname != func->name)
                    continue;
                auto ret = dynamic_cast<ir::ReturnInst *>(bb->iList.back());
                if (ret->val.value != call)
                    continue;
                bool tce_possible = true;
                for (int i = 0; i < call->params.size(); i++) {
                    if (func->params[i]->decl->is_array() && call->params[i].value != func->params[i]) {
                        std::cerr << "ir: WARN: " << func->name << " has TCE we can't deal with yet. \n";
                        tce_possible = false;
                    }
                }
                if (!tce_possible)
                    continue;
                std::cerr << "ir: performing tail call elimination on " << func->name << ". \n";
                if (!tce_prepared) {
                    param_map = tce_preparation(func);
                    tce_prepared = true;
                }
                bb->eraseInst(ret);
                delete ret;
                bb->eraseInst(call);
                bb->InsertAtEnd(new ir::JumpInst(entry));
                for (int i = 0; i < call->params.size(); i++)
                    if (param_map[i])
                        param_map[i]->InsertElem(bb, call->params[i].value);
            }
        }
    }
}