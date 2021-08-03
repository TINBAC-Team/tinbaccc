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
            int size = vn.size();
            for(int i=0;i<size;i++){
                auto _inst = dynamic_cast<ir::GetElementPtrInst*>(vn[i].first);
                if(!_inst || inst == _inst) continue;

                //dims count and arr should be the same
                if(inst->dims.size() != _inst->dims.size()) continue;
                if(get_vn(inst->arr.value) != get_vn(_inst->arr.value)) continue;

                bool dims_same = true;
                for(int i=0;i<inst->dims.size();i++) {
                    if(get_vn(inst->dims[i].value) != get_vn(_inst->dims[i].value)) {
                        dims_same = false;
                    }
                }
                if(dims_same){
                    return _inst;
                }
            }
            return inst;
        }

        ir::Value *get_vn(ir::Value *value) {
            auto it = std::find_if(vn.begin(), vn.end(),
                                   [value](std::pair<ir::Value *, ir::Value *> _vn) { return _vn.first == value; });
            if(it!=vn.end()) return it->second;

            vn.emplace_back(value, value);
            size_t cur = vn.size()-1;
            if (auto gepinst = dynamic_cast<ir::GetElementPtrInst *>(vn[cur].first)) {
                vn[cur].second = find_eq(gepinst);
            }
            return vn[cur].second;
        }

        void run_pass() {
            vn.reserve(100000); //FIXME: should use number of instructions instead
            for (auto &bb:func->bList) {
                for (auto &inst:bb->iList) {
                    if(get_vn(inst)!=inst){
                        std::cout<<"GOT!\n";
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