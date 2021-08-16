#include "ir/passes.h"
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <algorithm>

namespace ir_passes {
    class SideEffectCompute {
    public:
        ir::Module *module;

        SideEffectCompute(ir::Module *_module) : module(_module) {}

        //Reversed call graph
        std::unordered_map<ir::Function *, std::vector<ir::Function *>> rev_call_graph;

        void clearFunctionVis(ir::FunctionList &function_list) {
            for (auto &i:function_list) {
                i->visit = false;
            }
        }

        bool hasPureSideEffect(ir::Value *inst) { // without considering Call
            switch (inst->optype) {
                case ir::OpType::STORE:
                case ir::OpType::VSTORE:
                    return true;
                default:
                    return false;
            }
        }

        void dfs(ir::Function *func) {
            if (func->visit) return;
            func->visit = true;
            if (!func->has_side_effect) return;
            for (auto &v:rev_call_graph[func]) {
                if (func->has_side_effect) {
                    v->has_side_effect = true;
                }
                dfs(v);
            }
        }


        void run_pass() {
            clearFunctionVis(module->functionList);
            for (auto &func:module->functionList) {
                for (auto &bb:func->bList) {
                    for (auto &inst:bb->iList) {
                        if (hasPureSideEffect(inst)) {
                            func->has_side_effect = true;
                        }
                        if (auto callinst = dynamic_cast<ir::CallInst *>(inst)) {
                            auto call_target = std::find_if(module->functionList.begin(), module->functionList.end(),
                                                            [&](std::list<ir::Function *>::value_type &target) {
                                                                return target->name == callinst->fname;
                                                            });
                            if (call_target == module->functionList.end())
                                throw std::runtime_error("Calling invalid function.");
                            rev_call_graph[*call_target].push_back(func);
                        }
                    }
                }
            }
            for (auto &func:module->functionList) {
                dfs(func);
            }
        };
    };

    void side_effect_compute(ir::Module *module) {
        SideEffectCompute{module}.run_pass();
    }
}