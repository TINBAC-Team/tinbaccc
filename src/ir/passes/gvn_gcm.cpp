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
        ir::Function *func;

        explicit GVNPass(ir::Function *f) : func(f) {}

        std::map<std::vector<std::pair<ir::OpType, std::vector<ir::Value *> > >, int> inst_input;

        ir::Value *find_eq(ir::GetElementPtrInst *inst) {
            size_t size = vn.size();
            for (int i = 0; i < size; i++) {
                auto inst_prev = dynamic_cast<ir::GetElementPtrInst *>(vn[i].first);
                if (!inst_prev || inst == inst_prev) continue;

                //check dim size and arr address
                if (inst->dims.size() != inst_prev->dims.size()) continue;
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

        static bool is_swappable(ir::OpType op) {
            return op == ir::OpType::ADD || op == ir::OpType::MUL || op == ir::OpType::EQ || op == ir::OpType::NE ||
                   op == ir::OpType::AND || op == ir::OpType::OR;
        }

        ir::Value *find_eq(ir::BinaryInst *inst) {
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

        ir::Value *get_vn(ir::Value *value) {
            auto it = std::find_if(vn.begin(), vn.end(),
                                   [value](std::pair<ir::Value *, ir::Value *> _vn) { return _vn.first == value; });
            // path compression trick
            //.. if A,B,C are three identical instructions, vn[A]=A, vn[B]=A, vn[C]=B,
            //.. then vn[C] would be updated to A after path compression.
            //.. in this case: there are two instructions Inst1(A,B,C), Inst2(A,A,A)
            //.. if no path compression, the two instructions are different, but now they can be treated as the same.
            //.. I haven't seen any other implementation uses this trick, but it seems no counterexamples doing this.
            //..**REMOVE** it if encountered any problems.
            if (it != vn.end()) {
                if (it->first != it->second) {
                    return it->second = get_vn(it->second);
                } else return it->first;
            }

            vn.emplace_back(value, value);
            size_t cur = vn.size() - 1;
            if (auto gepinst = dynamic_cast<ir::GetElementPtrInst *>(vn[cur].first)) {
                vn[cur].second = find_eq(gepinst);
            }
            if (auto bininst = dynamic_cast<ir::BinaryInst *>(vn[cur].first)) {
                vn[cur].second = find_eq(bininst);
            }
            return vn[cur].second;
        }

        //it is O(n^2)
        void run_pass() {
            int inst_count = func->getInstCount() + 100; // number of instructions
            vn.reserve(inst_count);

            for (auto &bb:func->bList) {
                for (auto inst = bb->iList.begin(); inst != bb->iList.end(); inst++) {
                    auto inst_v = get_vn(*inst);
                    if (inst_v != *inst) {
                        (*inst)->replaceWith(inst_v);
                        inst = bb->iList.erase(inst);
                    }
                }
            }
        }


    };

    class GCMPass {
    public:
        std::unordered_set<ir::Value *> vis_early;
        std::unordered_set<ir::Value *> vis_late;
        ir::Function *func;

        explicit GCMPass(ir::Function *f) : func(f) {}

        ir::BasicBlock *depth_max(ir::BasicBlock *a, ir::BasicBlock *b) {
            if (a == nullptr && b == nullptr) throw std::runtime_error("Requesting max depth of two null BB");
            if (a == nullptr) return b;
            if (b == nullptr) return a;
            return (*a) < (*b) ? b : a;
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
                if (!dynamic_cast<ir::GetElementPtrInst *>(inst) && !dynamic_cast<ir::BinaryInst *>(inst)) {
                    vis_early.emplace(inst);
                    schedule_early_for_all_inputs(inst);
                }
            }

            for (auto &inst:insts) {
                if (!dynamic_cast<ir::GetElementPtrInst *>(inst) && !dynamic_cast<ir::BinaryInst *>(inst)) {
                    vis_late.emplace(inst);
                    for(auto &use:inst->uList){
                        schedule_late(inst);
                    }
                }
            }
        }

        ir::BasicBlock *find_lca(ir::BasicBlock *a, ir::BasicBlock *b) {
            if (a == nullptr) return b;
            while (a->dom_tree_depth < b->dom_tree_depth) {
                a = a->idom;
            }
            while (b->dom_tree_depth < a->dom_tree_depth) {
                b = b->idom;
            }
            while (a != b) {
                a = a->idom;
                b = b->idom;
            }
            return a;
        }

        void move_inst(ir::Value* _inst, ir::BasicBlock* block){
            if(auto inst = dynamic_cast<ir::Inst*>(_inst)){
                inst->bb->eraseInst(inst);
                inst->bb = block;
                block->InsertAtEnd(inst);
            }
        }

        void schedule_early(ir::Value *inst) {
            if (vis_early.find(inst) != vis_early.end()) return;
            vis_early.insert(inst);
            ir::BasicBlock *block;
            block = schedule_early_for_all_inputs(inst);
            move_inst(inst, block);
        }

        //returns the deepest input's bb
        ir::BasicBlock *schedule_early_for_all_inputs(ir::Value *_inst) {
            ir::BasicBlock *ret = func->bList.front();
            if (auto inst = dynamic_cast<ir::GetElementPtrInst *>(_inst)) {
                schedule_early(inst->arr.value);
                ret = depth_max(ret, inst->arr.value->bb);
                for (auto &i:inst->dims) {
                    schedule_early(i.value);
                    ret = depth_max(ret, i.value->bb);
                }
            }
            if (auto inst = dynamic_cast<ir::BinaryInst *>(_inst)) {
                schedule_early(inst->ValueL.value);
                ret = depth_max(ret, inst->ValueL.value->bb);
                schedule_early(inst->ValueR.value);
                ret = depth_max(ret, inst->ValueR.value->bb);
            }

            if (auto inst = dynamic_cast<ir::ReturnInst *>(_inst)) {
                schedule_early(inst->val.value);
                ret = depth_max(ret, inst->val.value->bb);
            }

            if (auto inst = dynamic_cast<ir::LoadInst *>(_inst)) {
                schedule_early(inst->ptr.value);
                ret = depth_max(ret, inst->ptr.value->bb);
            }
            return ret;
        }

        void schedule_late(ir::Value *inst) {
            if (vis_late.find(inst) != vis_late.end()) return;
            vis_late.insert(inst);
            ir::BasicBlock *lca = nullptr;
            for (auto &y:inst->uList) {
                schedule_late(y->value);
                ir::BasicBlock *use = y->value->bb;
                if (auto phiinst = dynamic_cast<ir::PhiInst *>(inst)) {
                    auto it = std::find_if(phiinst->phicont.begin(), phiinst->phicont.end(), [y](std::map<ir::BasicBlock *, std::unique_ptr<ir::Use>>::value_type &pair) {
                        return pair.second.get() == y;
                    });
                    use = it->first;
                }
                lca = find_lca(lca, use);
            }
            ir::BasicBlock *best = lca;
            while (lca != inst->bb) {
                if (lca->loop_deep < best->loop_deep) {
                    best = lca;
                }
                lca = lca->idom;
            }
            move_inst(inst, best);

        }
    };

    void gvn(ir::Module *module) {
        for (auto &i:module->functionList)
            if (!i->bList.empty())
                GVNPass(i).run_pass();
    }

    void gcm(ir::Module *module) {
        for (auto &i:module->functionList)
            if (!i->bList.empty())
                GCMPass(i).run_pass();
    }
}