#pragma clang diagnostic push
#pragma ide diagnostic ignored "misc-no-recursion"

#include "ir/passes/gvn_gcm.h"
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <algorithm>


namespace ir_passes {
    class GVNPass {
    public:
        std::vector<std::pair<ir::Value *, ir::Value *> > vn;
        std::unordered_map<ir::Value *, size_t> index_in_vn;
        ir::Function *func;
        ir::Module *module;

        explicit GVNPass(ir::Function *f, ir::Module *_module) : func(f), module(_module) {}

        ir::Value *find_eq(ir::GetElementPtrInst *inst);

        static bool is_swappable(ir::OpType op) {
            return op == ir::OpType::ADD || op == ir::OpType::MUL || op == ir::OpType::EQ || op == ir::OpType::NE ||
                   op == ir::OpType::AND || op == ir::OpType::OR;
        }

        ir::Value *find_eq(ir::BinaryInst *inst) {
            //constant propagation
            if (inst->ValueL.value->optype == ir::OpType::CONST &&
                inst->ValueR.value->optype == ir::OpType::CONST) {
                return ir::IRBuilder::getConstant(
                        dynamic_cast<ir::ConstValue *>(inst->ValueL.value)->value,
                        dynamic_cast<ir::ConstValue *>(inst->ValueR.value)->value,
                        inst->optype, module);
            }
            //remove add 0 ,sub 0 or mul 0
            if ((inst->optype == ir::OpType::ADD || inst->optype == ir::OpType::SUB) &&
                inst->ValueR.value->optype == ir::OpType::CONST &&
                dynamic_cast<ir::ConstValue *>(inst->ValueR.value)->value == 0) {
                return inst->ValueL.value;
            }
            //remove 0 sdiv x
            if (inst->optype == ir::OpType::SDIV &&
                inst->ValueL.value->optype == ir::OpType::CONST &&
                dynamic_cast<ir::ConstValue *>(inst->ValueL.value)->value == 0) {
                return ir::IRBuilder::getConstant(0, module);
            }
            //remove x mul 0
            if (inst->optype == ir::OpType::MUL &&
                inst->ValueR.value->optype == ir::OpType::CONST &&
                dynamic_cast<ir::ConstValue *>(inst->ValueR.value)->value == 0) {
                return ir::IRBuilder::getConstant(0, module);
            }
            //remove x mul 1
            if (inst->optype == ir::OpType::MUL &&
                inst->ValueR.value->optype == ir::OpType::CONST &&
                dynamic_cast<ir::ConstValue *>(inst->ValueR.value)->value == 1) {
                return get_vn(inst->ValueL.value);
            }
            //remove x sub x
            if (inst->optype == ir::OpType::SUB && get_vn(inst->ValueL.value) == get_vn(inst->ValueR.value)) {
                return ir::IRBuilder::getConstant(0, module);
            }
            //remove cmp cond x, x
            if (get_vn(inst->ValueL.value) == get_vn(inst->ValueR.value)) {
                switch (inst->optype) {
                    case ir::OpType::EQ:
                    case ir::OpType::SGE:
                    case ir::OpType::SLE:
                        return ir::IRBuilder::getConstant(1, module);
                    case ir::OpType::NE:
                    case ir::OpType::SGT:
                    case ir::OpType::SLT:
                        return ir::IRBuilder::getConstant(0, module);
                    case ir::OpType::AND:
                    case ir::OpType::OR:
                        return get_vn(inst->ValueL.value);
                }
            }
            if (inst->is_icmp()) return inst;
            size_t size = vn.size();
            for (int i = 0; i < size; i++) {
                auto inst_prev = dynamic_cast<ir::BinaryInst *>(vn[i].first);
                if (!inst_prev || inst == inst_prev) continue;

                //check op
                if (inst->optype != inst_prev->optype) continue;
                //check valueL and valueR
                if (get_vn(inst->ValueL.value) == get_vn(inst_prev->ValueL.value) &&
                    get_vn(inst->ValueR.value) == get_vn(inst_prev->ValueR.value) ||
                    (is_swappable(inst->optype) && get_vn(inst->ValueL.value) == get_vn(inst_prev->ValueR.value) &&
                     get_vn(inst->ValueR.value) == get_vn(inst_prev->ValueL.value))) {
                    return get_vn(vn[i].second);
                }
            }
            return inst;
        }

        ir::Value *find_eq(ir::CallInst *inst) {
            if (inst->function->has_side_effect) return inst;
            if (inst->function->load_from_addr) return inst;
            size_t size = vn.size();
            for (int i = 0; i < size; i++) {
                auto inst_prev = dynamic_cast<ir::CallInst *>(vn[i].first);
                if (!inst_prev || inst == inst_prev) continue;
                //check function
                if (inst->function != inst_prev->function) continue;
                //check param
                size_t param_size = inst->params.size();
                for (size_t index = 0; index < param_size; index++) {
                    if (get_vn(inst->params[index].value) != get_vn(inst_prev->params[index].value)) {
                        return inst;
                    }
                }
                return vn[i].second;
            }
            return inst;
        }

        ir::Value *get_vn(ir::Value *value) {
            if (index_in_vn.find(value) != index_in_vn.end()) {
                size_t index = index_in_vn[value];
                if (vn[index].first != vn[index].second) {
                    return vn[index].second = get_vn(vn[index].second);
                } else return vn[index].first;
            }
            vn.emplace_back(value, value);
            index_in_vn[value] = vn.size() - 1;
            size_t cur = vn.size() - 1;
            if (auto gepinst = dynamic_cast<ir::GetElementPtrInst *>(vn[cur].first)) {
                vn[cur].second = find_eq(gepinst);
            }
            if (auto bininst = dynamic_cast<ir::BinaryInst *>(vn[cur].first)) {
                vn[cur].second = find_eq(bininst);
            }
            if (auto callinst = dynamic_cast<ir::CallInst *>(vn[cur].first)) {
                vn[cur].second = find_eq(callinst);
            }
            return vn[cur].second;
        }

        //it is O(n^2)
        int run_pass() {
            int erase_count = 0;
            int inst_count = func->getInstCount() + 100; // number of instructions
            vn.reserve(inst_count);
            index_in_vn.reserve(inst_count);

            for (auto &bb:func->bList) {
                auto inst = bb->iList.begin();
                while (inst != bb->iList.end()) {
                    auto inst_v = get_vn(*inst);
                    if (inst_v != *inst) {
                        erase_count++;
                        (*inst)->replaceWith(inst_v);
                        delete *inst;
                        size_t index = index_in_vn[*inst];
                        index_in_vn.erase(vn[index].first);
                        std::swap(vn[index], vn.back());
                        vn.pop_back();
                        inst = bb->iList.erase(inst);
                        continue;
                    }
                    inst++;
                }
            }
            return erase_count;
        }


    };

    ir::Value *GVNPass::find_eq(ir::GetElementPtrInst *inst) {
        size_t size = vn.size();
        for (int i = 0; i < size; i++) {
            auto inst_prev = dynamic_cast<ir::GetElementPtrInst *>(vn[i].first);
            if (!inst_prev || inst == inst_prev) continue;

            //check dim size and arr address
            if (inst->dims.size() != inst_prev->dims.size()) continue;
            if (inst->decl != inst_prev->decl) continue;
            if (get_vn(inst->arr.value) != get_vn(inst_prev->arr.value)) continue;
            //check dim value
            bool dims_same = true;
            for (size_t dim_index = 0; dim_index < inst->dims.size(); dim_index++) {
                if (get_vn(inst->dims[dim_index].value) != get_vn(inst_prev->dims[dim_index].value)) {
                    dims_same = false;
                }
            }
            if (dims_same) {
                return get_vn(vn[i].second);
            }
        }
        return inst;
    }

    class GCMPass {
    public:
        enum class MoveBehavior {
            MOVE_FRONT,
            MOVE_END,
            MOVE_BEFORE,
            MOVE_AFTER
        };
        std::unordered_set<ir::Value *> vis_early;
        std::unordered_set<ir::Value *> vis_late;
        ir::Function *func;
        ir::Module *module;

        explicit GCMPass(ir::Function *f, ir::Module *_module) : func(f), module(_module) {}

        static bool isAlterMemInst(ir::Value *inst) {
            if (auto callinst = dynamic_cast<ir::CallInst *>(inst)) {
                return callinst->function->has_side_effect;
            }
            return inst->optype == ir::OpType::STORE;
            // FIXME: load can not move across VStore, so the frontend may produce wrong code.
            //  ADD a VSTORE here if vector instuctions are carefully inserted in the first place,
            //  to guarantee its correctness.
        }

        static bool canMoveToTarget(ir::Value *inst, ir::Value *target) {
            if (inst->optype != ir::OpType::LOAD && inst->optype != ir::OpType::CALL) return true;
            if (!target) return false;
            auto inst_iter = std::find(inst->bb->iList.begin(), inst->bb->iList.end(), inst);
            auto target_iter = std::find(inst->bb->iList.begin(), inst->bb->iList.end(), target);
            auto inst_iter_rev = std::find(inst->bb->iList.rbegin(), inst->bb->iList.rend(), inst);
            auto target_iter_rev = std::find(inst->bb->iList.rbegin(), inst->bb->iList.rend(), target);
            size_t inst_pos = std::distance(inst->bb->iList.begin(), inst_iter);
            size_t target_pos = std::distance(inst->bb->iList.begin(), target_iter);
            if (inst_pos < target_pos) {
                auto mem_inst = std::find_if(inst_iter, target_iter, [&](std::list<ir::Value *>::value_type i) {
                    return isAlterMemInst(i);
                });
                if (mem_inst != target_iter) {
                    return false;
                }
            } else { //we are climbing...
                auto mem_inst = std::find_if(inst_iter_rev, target_iter_rev,
                                             [&](std::list<ir::Value *>::value_type i) {
                                                 return isAlterMemInst(i);
                                             });
                if (mem_inst != target_iter_rev) {
                    return false;
                }
            }
            return true;
        }

        static void moveIcmpBack(ir::BasicBlock *bb) {
            std::vector<ir::Value *> insts;
            insts.reserve(bb->iList.size() + 10);
            for (auto &inst:bb->iList) {
                insts.emplace_back(inst);
            }
            for (auto &inst:insts) {
                if (auto bininst = dynamic_cast<ir::BinaryInst *>(inst)) {
                    if (!bininst->is_icmp()) continue;
                    if (inst->uList.size() > 1) continue;
                    for (auto &use:inst->uList) {
                        if (use->user->bb == inst->bb && dynamic_cast<ir::BranchInst *>(use->user)) {
                            move_inst(inst, inst->bb);
                        }
                    }
                }
            }
        }

        static void moveTerminalInstBack(ir::BasicBlock *bb) {
            std::vector<ir::Value *> insts;
            insts.reserve(bb->iList.size() + 10);
            for (auto &inst:bb->iList) {
                insts.emplace_back(inst);
            }
            for (auto &inst:insts) {
                if (dynamic_cast<ir::ReturnInst *>(inst) || dynamic_cast<ir::BranchInst *>(inst) ||
                    dynamic_cast<ir::JumpInst *>(inst)) {
                    move_inst(inst, inst->bb);
                }
            }
        }

        static void movePhiFront(ir::BasicBlock *bb) {
            std::vector<ir::Value *> insts;
            insts.reserve(bb->iList.size() + 10);
            for (auto &inst:bb->iList) {
                insts.emplace_back(inst);
            }
            for (auto inst = insts.rbegin(); inst != insts.rend(); inst++) {
                if (dynamic_cast<ir::PhiInst *>(*inst)) {
                    move_inst(*inst, (*inst)->bb, MoveBehavior::MOVE_FRONT);
                }
            }
        }

        void schedule_inner_early(ir::Value *_inst) {
            auto inst = dynamic_cast<ir::Inst *> (_inst);
            if (!inst || inst->vis) return;
            inst->vis = true;
            std::vector<ir::Value *> uses;
            uses.reserve(inst->uses().size());
            int dis = -1;
            ir::Value *last_input = nullptr;
            for (auto &val : inst->uses()) {
                if (val->bb == inst->bb) {
                    uses.emplace_back(val);
                }
            }
            for (auto &usage:uses) {
                schedule_inner_early(usage);
            }
            if (!dynamic_cast<ir::BinaryInst *>(inst) && !dynamic_cast<ir::GetElementPtrInst *>(inst) &&
                    !dynamic_cast<ir::LoadInst *>(inst) &&
                    !(dynamic_cast<ir::CallInst *>(inst) &&
                            !dynamic_cast<ir::CallInst *>(inst)->function->has_side_effect &&
                            !dynamic_cast<ir::CallInst *>(inst)->function->load_from_addr))
                return;
            for (auto &usage:uses) {
                if ((int) std::distance(inst->bb->iList.begin(),
                                        std::find(inst->bb->iList.begin(), inst->bb->iList.end(), usage)) > dis) {
                    dis = (int) std::distance(inst->bb->iList.begin(),
                                              std::find(inst->bb->iList.begin(), inst->bb->iList.end(), usage));
                    last_input = usage;
                }
            }
            if (!canMoveToTarget(inst, last_input)) return;
            if (last_input)
                move_inst(inst, inst->bb, MoveBehavior::MOVE_AFTER, last_input);
            else move_inst(inst, inst->bb, MoveBehavior::MOVE_FRONT);
        }

        void schedule_inner_late(ir::Value *_inst) {
            if (_inst->optype == ir::OpType::LOAD && (*_inst->uList.begin())->user->optype == ir::OpType::DUP)
                printf("DUP!!!\n");
            auto inst = dynamic_cast<ir::Inst *> (_inst);
            if (!inst || inst->vis) return;
            inst->vis = true;
            int dis = (int) std::distance(inst->bb->iList.begin(), inst->bb->iList.end()) + 1;
            ir::Value *first_user = nullptr;
            for (auto &use:inst->uList) {
                if (use->user->bb != inst->bb) continue;
                schedule_inner_late(use->user);
            }
            if (!dynamic_cast<ir::BinaryInst *>(inst) && !dynamic_cast<ir::GetElementPtrInst *>(inst) &&
                !dynamic_cast<ir::LoadInst *>(inst) &&
                !(dynamic_cast<ir::CallInst *>(inst) &&
                  !dynamic_cast<ir::CallInst *>(inst)->function->has_side_effect &&
                  !dynamic_cast<ir::CallInst *>(inst)->function->load_from_addr))
                return;
            for(auto &use:inst->uList){
                if (use->user->bb != inst->bb) continue;
                if (std::distance(inst->bb->iList.begin(),
                                  std::find(inst->bb->iList.begin(), inst->bb->iList.end(), use->user)) < dis) {
                    dis = (int) std::distance(inst->bb->iList.begin(),
                                              std::find(inst->bb->iList.begin(), inst->bb->iList.end(), use->user));
                    first_user = use->user;
                }
            }
            if (!canMoveToTarget(inst, first_user)) return;
            if (first_user) {
                // If an instruction A immediately precedes the [first user],
                // and A's uList has only one usage by [first user], then we move up...
                /*auto bef_first_user = --std::find(inst->bb->iList.rbegin(),inst->bb->iList.rend(),first_user);
                if(bef_first_user!=inst->bb->iList.rend())
                {
                    if((*bef_first_user)->uList.size()==1 && (*(*bef_first_user)->uList.begin())->user==first_user)
                    {
                        move_inst(inst, inst->bb, MoveBehavior::MOVE_BEFORE,(*bef_first_user));
                        return;
                    }
                }*/
                move_inst(inst, inst->bb, MoveBehavior::MOVE_BEFORE, first_user);
            } else move_inst(inst, inst->bb, MoveBehavior::MOVE_END);
        }

        void rescheduleInstOrder(ir::BasicBlock *bb) {
            std::vector<ir::Value *> insts;
            insts.reserve(bb->iList.size() + 10);
            for (auto &inst:bb->iList) {
                insts.emplace_back(inst);
                inst->vis = false;
            }
            for (auto &inst:insts) {
                schedule_inner_early(inst);
            }
            insts.clear();
            insts.reserve(bb->iList.size() + 10);
            for (auto &inst:bb->iList) {
                insts.emplace_back(inst);
                inst->vis = false;
            }
            for (auto &inst:insts) {
                schedule_inner_late(inst);
            }

        }

        void run_pass() {
            //saving all instruction's ptr, or the move would cause problem
            std::vector<ir::Value *> insts;
            int inst_count = func->getInstCount() + 100; // number of instructions
            insts.reserve(inst_count);
            vis_early.reserve(inst_count);
            vis_late.reserve(inst_count);
            for (auto &bb:func->bList) {
                for (auto &inst:bb->iList) {
                    insts.emplace_back(inst);
                }
            }
            for (auto &inst:insts) {
                schedule_early(inst);
            }

            for (auto inst:insts) {
                schedule_late(inst);
            }
            for (auto &bb:func->bList) {
                moveIcmpBack(bb);
                moveTerminalInstBack(bb);
                movePhiFront(bb);
                rescheduleInstOrder(bb);
                moveIcmpBack(bb);
                moveTerminalInstBack(bb);
                movePhiFront(bb);
            }
        }

        static ir::BasicBlock *find_lca(ir::BasicBlock *a, ir::BasicBlock *b) {
            if (a == nullptr) return b;
            while (a->dom_tree_depth < b->dom_tree_depth) {
                b = b->idom;
            }
            while (b->dom_tree_depth < a->dom_tree_depth) {
                a = a->idom;
            }
            while (a != b) {
                a = a->idom;
                b = b->idom;
            }
            return a;
        }

        static bool isPinned(ir::Value *inst) {
            if (auto bininst = dynamic_cast<ir::BinaryInst *>(inst)) {
                return bininst->is_icmp();
            }
            if (auto callinst = dynamic_cast<ir::CallInst *>(inst)) {
                return callinst->function->has_side_effect || callinst->function->load_from_addr;
            }
            return !(dynamic_cast<ir::GetElementPtrInst *>(inst) || dynamic_cast<ir::BinaryInst *>(inst));
        }


        static void
        move_inst(ir::Value *_inst, ir::BasicBlock *block, MoveBehavior move_behavior = MoveBehavior::MOVE_END,
                  ir::Value *target = nullptr) {
            if (auto inst = dynamic_cast<ir::Inst *>(_inst)) {
                inst->bb->eraseInst(inst);
                inst->bb = block;
                switch (move_behavior) {
                    case MoveBehavior::MOVE_FRONT:
                        block->InsertAtFront(inst);
                        break;
                    case MoveBehavior::MOVE_END:
                        block->InsertAtEnd(inst);
                        break;
                    case MoveBehavior::MOVE_BEFORE:
                        block->InsertBefore(inst, target);
                        break;
                    case MoveBehavior::MOVE_AFTER:
                        block->InsertAfter(inst, target);
                        break;
                }
            }
        }


        static void schedule_deeper(ir::Value *i, ir::Value *x) {
            if (!x || !x->bb) return;
            if (i->bb->dom_tree_depth < x->bb->dom_tree_depth) {
                move_inst(i, x->bb);
            }
        }

        void schedule_early(ir::Value *_inst) {
            if (vis_early.find(_inst) != vis_early.end()) return;
            vis_early.insert(_inst);
            ir::BasicBlock *block = func->bList.front();
            if (auto inst = dynamic_cast<ir::GetElementPtrInst *>(_inst)) {
                move_inst(_inst, block);
                schedule_early(inst->arr.value);
                schedule_deeper(inst, inst->arr.value);
                for (auto &i:inst->dims) {
                    schedule_early(i.value);
                    schedule_deeper(inst, i.value);
                }
            }
            if (auto inst = dynamic_cast<ir::CallInst *>(_inst)) {
                if (!inst->function->has_side_effect && !inst->function->load_from_addr) {
                    move_inst(_inst, block);
                    for (auto &i:inst->params) {
                        schedule_early(i.value);
                        schedule_deeper(inst, i.value);
                    }
                }

            }
            if (auto inst = dynamic_cast<ir::BinaryInst *>(_inst)) {
                ir::BasicBlock *old_bb = _inst->bb;
                if (!inst->is_icmp())
                    move_inst(_inst, block);
                schedule_early(inst->ValueL.value);
                if (!inst->is_icmp())
                    schedule_deeper(inst, inst->ValueL.value);
                schedule_early(inst->ValueR.value);
                if (!inst->is_icmp())
                    schedule_deeper(inst, inst->ValueR.value);
                if (inst->is_icmp()) {
                    move_inst(_inst, block);
                    move_inst(_inst, old_bb);
                }
            }

            if (auto inst = dynamic_cast<ir::LoadInst *>(_inst)) {
                ir::BasicBlock* old_bb = _inst->bb;
                schedule_early(inst->ptr.value);
                //schedule_deeper(inst,inst->ptr.value);
                move_inst(_inst, block);
                move_inst(_inst, old_bb);
            }
            if (auto inst = dynamic_cast<ir::StoreInst *>(_inst)) {
                ir::BasicBlock* old_bb = _inst->bb;
                schedule_early(inst->ptr.value);
                //schedule_deeper(inst,inst->ptr.value);
                schedule_early(inst->val.value);
                //schedule_deeper(inst,inst->val.value);
                move_inst(_inst, block);
                move_inst(_inst, old_bb);
            }
            if (auto inst = dynamic_cast<ir::CallInst *>(_inst)) {
                ir::BasicBlock *old_bb = _inst->bb;
                for (auto &i:inst->params) {
                    schedule_early(i.value);
                }
                move_inst(_inst, block);
                move_inst(_inst, old_bb);
            }
        }


        bool eliminateChainCalc(ir::Value *_inst, ir::Value *_usage) const {
            auto inst = dynamic_cast<ir::BinaryInst *>(_inst);
            auto usage = dynamic_cast<ir::BinaryInst *>(_usage);
            if (!inst || !usage) throw std::runtime_error("chain add elimination works only with binaryinst!");

            if (inst->ValueR.value->optype != ir::OpType::CONST ||
                usage->ValueR.value->optype != ir::OpType::CONST)
                return false;

            int constL = dynamic_cast<ir::ConstValue *>(inst->ValueR.value)->value;
            int constR = dynamic_cast<ir::ConstValue *>(usage->ValueR.value)->value;
            if (inst->optype == ir::OpType::ADD && usage->optype == ir::OpType::ADD) {
                usage->ValueL.use(inst->ValueL.value);
                usage->ValueR.use(ir::IRBuilder::getConstant(constL + constR, module));
                return true;
            }
            if (inst->optype == ir::OpType::ADD && usage->optype == ir::OpType::SUB) {
                usage->ValueL.use(inst->ValueL.value);
                int val = constR - constL;
                if (val < 0) {
                    val = -val;
                    usage->optype = ir::OpType::ADD;
                }
                usage->ValueR.use(ir::IRBuilder::getConstant(val, module));
                return true;
            }
            if (inst->optype == ir::OpType::SUB && usage->optype == ir::OpType::ADD) {
                usage->ValueL.use(inst->ValueL.value);
                int val = constR - constL;
                if (val < 0) {
                    val = -val;
                    usage->optype = ir::OpType::SUB;
                }
                usage->ValueR.use(ir::IRBuilder::getConstant(val, module));
                return true;
            }
            if (inst->optype == ir::OpType::SUB && usage->optype == ir::OpType::SUB) {
                usage->ValueL.use(inst->ValueL.value);
                usage->ValueR.use(ir::IRBuilder::getConstant(constL + constR, module));
                return true;
            }
            if (inst->optype == ir::OpType::MUL && usage->optype == ir::OpType::SDIV) {
                if(constL % constR != 0) return false;
                usage->ValueL.use(inst->ValueL.value);
                usage->ValueR.use(ir::IRBuilder::getConstant(constL / constR, module));
                usage->optype = ir::OpType::MUL;
                return true;
            }
            return false;
        }

        void schedule_late(ir::Value *inst) {
            if (!inst)
                throw std::runtime_error("invalid use which may not be part of IR!");
            if (vis_late.find(inst) != vis_late.end())
                return;
            vis_late.insert(inst);
            ir::BasicBlock *lca = nullptr;

            //eliminate chain calculation
            std::vector<ir::Use*> use;
            use.reserve(inst->uList.size());
            for(auto &i:inst->uList){
                use.emplace_back(i);
            }
            for(auto &i:use){
                if (dynamic_cast<ir::BinaryInst *>(inst) && dynamic_cast<ir::BinaryInst *>(i->user))
                    eliminateChainCalc(inst, i->user);
            }
            for (auto &y:inst->uList) {
                schedule_late(y->user);
                if (isPinned(inst)) continue;
                ir::BasicBlock *user_bb = y->user->bb;
                if (auto phiinst = dynamic_cast<ir::PhiInst *>(y->user)) {
                    auto it = std::find_if(phiinst->phicont.begin(), phiinst->phicont.end(),
                                           [y](std::map<ir::BasicBlock *, std::unique_ptr<ir::Use>>::value_type &pair) {
                                               return pair.second.get() == y;
                                           });
                    user_bb = it->first;
                }
                lca = find_lca(lca, user_bb);
            }
            if (isPinned(inst)) {
                return;
            }
            if (inst->uList.empty()) {
                //throw std::runtime_error("GCM should not be dealing with an empty uList.");
                return;
            }
            ir::BasicBlock *best = lca;
            //different from Click's paper, inspired by TrivialCompiler
            int best_loop_depth =best->loop_depth;
            while (true) {
                int cur_loop_depth = lca->loop_depth;
                if (cur_loop_depth < best_loop_depth) {
                    best = lca;
                    best_loop_depth = cur_loop_depth;
                }
                if (lca == inst->bb) break;
                lca = lca->idom;
            }
            if(inst->bb!=best)
                move_inst(inst, best,MoveBehavior::MOVE_FRONT);
        }
    };

    void gvn(ir::Module *module) {
        int erase_count = 0;
        for (auto &i:module->functionList) {
            if (!i->bList.empty()) {
                int cur_erase_count;
                while ((cur_erase_count = GVNPass(i, module).run_pass())) {
                    erase_count += cur_erase_count;
                }
            }
        }
        if (erase_count > 0)
            std::cerr << "GVN: Eliminated " << erase_count << " instructions." << std::endl;
    }

    void gcm(ir::Module *module) {
        for (auto &i:module->functionList)
            if (!i->bList.empty()) {
                GCMPass(i, module).run_pass();
            }
    }
}