#include <ir/passes.h>
#include <map>
#include <iostream>

namespace ir_passes {
    class FunctionProcessor {
    private:
        bool work_done;
        ir::Module *module;
        std::unordered_map<std::string, ir::Function *> func_name_map;
        std::map<ir::Function *, int> call_count;
        std::map<ir::Function *, unsigned long> func_length;
        std::set<ir::Function *> not_inlinable_func;

        struct CallInfo {
            ir::CallInst *call;
            ir::instList::iterator call_it;
            std::list<ir::BasicBlock *>::iterator bb_it;
            ir::Function *func;

            CallInfo(ir::CallInst *c, ir::instList::iterator c_it, std::list<ir::BasicBlock *>::iterator b_it,
                     ir::Function *f) : call(c), call_it(c_it), bb_it(b_it), func(f) {}
        };

        std::map<ir::Function *, std::list<CallInfo>> call_info;

        void collect_function_info();

        void remove_unused_function();

        void inline_one_function();

    public:
        FunctionProcessor(ir::Module *m) : module(m), work_done(false) {
            for (auto &func:module->functionList)
                func_name_map[func->name] = func;
        }

        void work();
    };

    void FunctionProcessor::collect_function_info() {
        call_count.clear();
        call_info.clear();
        func_length.clear();
        not_inlinable_func.clear();
        // preserve entry point
        call_count[func_name_map["main"]] = 1;
        not_inlinable_func.insert(func_name_map["main"]);
        for (auto &func:module->functionList) {
            if (func->bList.empty())
                not_inlinable_func.insert(func);
            for (auto bb_it = func->bList.begin(); bb_it != func->bList.end(); bb_it++) {
                auto &bb = *bb_it;
                func_length[func] += bb->iList.size();
                for (auto inst_it = bb->iList.begin(); inst_it != bb->iList.end(); inst_it++) {
                    auto &inst = *inst_it;
                    if (inst->optype == ir::OpType::ALLOCA)
                        not_inlinable_func.insert(func);
                    // check optype first to avoid too many vtable lookups
                    if (inst->optype != ir::OpType::CALL)
                        continue;
                    auto call_inst = dynamic_cast<ir::CallInst *>(inst);
                    auto call_target = func_name_map[call_inst->fname];
                    if (call_target == func)
                        not_inlinable_func.insert(func);
                    call_count[call_target]++;
                    call_info[call_target].emplace_back(call_inst, inst_it, bb_it, func);
                }
            }
        }
    }

    void FunctionProcessor::remove_unused_function() {
        ir::FunctionList used_functions;
        for (auto &func:module->functionList) {
            if (call_count[func]) {
                used_functions.push_back(func);
            } else {
                module->unusedFunctionList.push_back(func);
                // avoid noise on unused external functions
                if (!func->bList.empty())
                    std::cerr << "unused function " << func->name << " removed." << std::endl;
            }
        }
        module->functionList = std::move(used_functions);
    }

    void FunctionProcessor::inline_one_function() {
        work_done = true;
        for (auto &func:module->functionList) {
            if (not_inlinable_func.find(func) != not_inlinable_func.end())
                continue;
            if (call_count[func] > 1 && func_length[func] > 100)
                continue;
            if (call_count[func] == 0)
                continue;
            work_done = false;
            // we can only inline one call at a time.
            auto &call_tgt = call_info[func].front();
            auto &call_bb = *call_tgt.bb_it;
            std::cerr << "inlining function " << func->name << " into " << call_tgt.func->name << "." << std::endl;
            std::map<ir::BasicBlock *, ir::BasicBlock *> bb_map;
            std::map<ir::Value *, ir::Value *> value_map;
            std::list<std::pair<ir::Value *, ir::BasicBlock *>> ret_list;
            std::list<std::pair<ir::PhiInst *, ir::PhiInst *>> phi_list; // <dst, src>
            auto get_val = [&](const ir::Use &u) {
                ir::Value *ret;
                if (value_map.find(u.value) != value_map.end())
                    return value_map[u.value];
                else if ((ret = dynamic_cast<ir::ConstValue *>(u.value)))
                    return ret;
                else if ((ret = dynamic_cast<ir::GlobalVar *>(u.value)))
                    return ret;
                else
                    throw std::runtime_error("value not defined!");
            };
            // 1. split caller BB into two
            auto ret_bb = new ir::BasicBlock(call_bb->loop_deep);
            for (auto inst_it = std::next(call_tgt.call_it); inst_it != call_bb->iList.end(); inst_it++)
                ret_bb->InsertAtEnd(*inst_it);
            // at this point the call instruction exists in call_tgt.call only.
            call_bb->iList.erase(call_tgt.call_it, call_bb->iList.end());
            auto ret_bb_it = call_tgt.func->bList.insert(std::next(call_tgt.bb_it), ret_bb);
            // 2. create all BBs
            for (auto &bb:func->bList) {
                auto new_bb = new ir::BasicBlock(call_bb->loop_deep + bb->loop_deep);
                bb_map[bb] = new_bb;
                call_tgt.func->bList.insert(ret_bb_it, new_bb);
            }
            // 3. add a jump from caller BB into the first callee BB
            call_bb->InsertAtEnd(new ir::JumpInst(bb_map[func->bList.front()]));
            // 4. setup function parameter map, also clear the uses here.
            for (int i = 0; i < call_tgt.call->params.size(); i++) {
                value_map[func->params[i]] = call_tgt.call->params[i].value;
                call_tgt.call->params[i].user->uList.erase(&call_tgt.call->params[i]);
            }
            // 5. duplicate basicblocks
            for (auto &src_bb:func->bList) {
                auto &dst_bb = bb_map[src_bb];
                for (auto &inst:src_bb->iList) {
                    if (auto x = dynamic_cast<ir::BinaryInst *>(inst)) {
                        auto *dup = new ir::BinaryInst(x->optype, get_val(x->ValueL), get_val(x->ValueR));
                        dst_bb->InsertAtEnd(dup);
                        value_map[inst] = dup;
                    } else if (auto x = dynamic_cast<ir::PhiInst *>(inst)) {
                        auto *dup = new ir::PhiInst();
                        dst_bb->InsertAtEnd(dup);
                        value_map[inst] = dup;
                        phi_list.emplace_back(dup, x);
                    } else if (auto x = dynamic_cast<ir::CallInst *>(inst)) {
                        auto *dup = new ir::CallInst(x->fname, x->is_void);
                        dup->params.reserve(x->params.size());
                        for (auto &u:x->params)
                            dup->params.emplace_back(dup, get_val(u));
                        dst_bb->InsertAtEnd(dup);
                        value_map[inst] = dup;
                    } else if (auto x = dynamic_cast<ir::BranchInst *>(inst)) {
                        auto *dup = new ir::BranchInst(get_val(x->cond), bb_map[x->true_block],
                                                       bb_map[x->false_block]);
                        dst_bb->InsertAtEnd(dup);
                        value_map[inst] = dup;
                    } else if (auto x = dynamic_cast<ir::JumpInst *>(inst)) {
                        auto *dup = new ir::JumpInst(bb_map[x->to]);
                        dst_bb->InsertAtEnd(dup);
                        value_map[inst] = dup;
                    } else if (auto x = dynamic_cast<ir::ReturnInst *>(inst)) {
                        auto *dup = new ir::JumpInst(ret_bb);
                        dst_bb->InsertAtEnd(dup);
                        value_map[inst] = dup;
                        if (func->return_int)
                            ret_list.emplace_back(get_val(x->val), dst_bb);
                    } else if (auto x = dynamic_cast<ir::LoadInst *>(inst)) {
                        auto *dup = new ir::LoadInst(get_val(x->ptr));
                        dst_bb->InsertAtEnd(dup);
                        value_map[inst] = dup;
                    } else if (auto x = dynamic_cast<ir::StoreInst *>(inst)) {
                        auto *dup = new ir::StoreInst(get_val(x->ptr), get_val(x->val));
                        dst_bb->InsertAtEnd(dup);
                        value_map[inst] = dup;
                    } else if (auto x = dynamic_cast<ir::GetElementPtrInst *>(inst)) {
                        std::vector<ir::Value *> dims;
                        dims.reserve(x->dims.size());
                        for (const auto &u:x->dims)
                            dims.emplace_back(get_val(u));
                        auto *dup = new ir::GetElementPtrInst(get_val(x->arr), dims, std::vector<int>());
                        dup->multipliers = x->multipliers;
                        if(auto gep_src = dynamic_cast<ir::GetElementPtrInst *>(get_val(x->arr)))
                            dup->decl = x->decl;
                        dst_bb->InsertAtEnd(dup);
                        value_map[inst] = dup;
                    } else {
                        throw std::runtime_error("unknown instruction to duplicate.");
                    }
                }
            }
            // 6. fill phis
            for (auto &i:phi_list) {
                for (auto &u:i.second->phicont)
                    i.first->InsertElem(bb_map[u.first], get_val(*u.second));
            }
            // 7. fill return phi if needed
            if (func->return_int) {
                auto *phi = new ir::PhiInst();
                ret_bb->InsertAtFront(phi);
                for (auto &i:ret_list)
                    phi->InsertElem(i.second, i.first);
                // and replace call uses
                for (auto cur_use:call_tgt.call->uList) {
                    if (cur_use->value != call_tgt.call)
                        continue;
                    cur_use->use(phi);
                }
            } else {
                if (!call_tgt.call->uList.empty())
                    throw std::runtime_error("void function can't be used!");
            }
            // 8. fix phi in the successors of original blocks
            for(auto &succ:ret_bb->succ()) {
                for(auto &inst:succ->iList) {
                    auto x = dynamic_cast<ir::PhiInst *>(inst);
                    if(!x)
                        break;
                    x->replaceBB(call_bb, ret_bb);
                }
            }

            return;
        }
    }

    void FunctionProcessor::work() {
        while (!work_done) {
            collect_function_info();
            remove_unused_function();
            inline_one_function();
        }
    }

    void function_process(ir::Module *module) {
        FunctionProcessor(module).work();
    }
}