//
// Created by GnSight on 2021/08/02.
//

#include "ir/passes/gvn.h"
#include <map>
#include <unordered_map>
#include <vector>
#include <algorithm>


namespace ir_passes {
    class GVNPass {
    public:
        std::vector<std::pair<ir::Value *, ir::Value *> > vn;
        ir::Function *func;

        GVNPass(ir::Function *f) : func(f) {}

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
            int inst_count = 100; // number of instructions
            for (auto &bb:func->bList) {
                inst_count += bb->iList.size();
            }

            vn.reserve(inst_count);

            for (auto &bb:func->bList) {
                for (auto &inst:bb->iList) {
                    auto inst_v = get_vn(inst);
                    if (inst_v != inst) {
                        for (auto &i:inst->uList) {
                            i->use(inst_v);
                        }
                        bb->eraseInst((ir::Inst*) inst);
                    }
                }
            }
        }


    };

    void gvn(ir::Module *module) {
        for (auto &i:module->functionList)
            if (!i->bList.empty())
                GVNPass(i).run_pass();
    }
}