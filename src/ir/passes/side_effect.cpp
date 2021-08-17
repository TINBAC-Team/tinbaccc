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
                case ir::OpType::STORE: { // for clang warning: jump bypass variable initialization
                    auto storeinst = dynamic_cast<const ir::StoreInst *>(inst);
                    if (storeinst->ptr.value->optype == ir::OpType::GETELEMPTR) {
                        auto gep = dynamic_cast<ir::GetElementPtrInst *>(storeinst->ptr.value);
                        if (gep->arr.value->optype == ir::OpType::ALLOCA)
                            return false;
                    }
                    return true;
                }
                case ir::OpType::VSTORE:
                    return true;
                default:
                    return false;
            }
        }

        bool isLoadFromAddr(ir::Value *inst) {
            auto gepinst = dynamic_cast<ir::GetElementPtrInst *>(inst);
            auto ldinst = dynamic_cast<ir::LoadInst *>(inst);
            if (gepinst)
                return dynamic_cast<ir::GlobalVar *>(gepinst->arr.value) ||
                       dynamic_cast<ir::FuncParam *>(gepinst->arr.value);
            if (ldinst)
                return dynamic_cast<ir::GlobalVar *>(ldinst->ptr.value) ||
                       dynamic_cast<ir::FuncParam *>(ldinst->ptr.value);
            return false;
        }

        void dfs(ir::Function *func) {
            if (func->visit) return;
            func->visit = true;
            for (auto &v:rev_call_graph[func]) {
                if (func->has_side_effect) {
                    v->has_side_effect = true;
                }
                if (func->load_from_addr) {
                    v->load_from_addr = true;
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
                        if (isLoadFromAddr(inst)) {
                            func->load_from_addr = true;
                        }
                        if (auto callinst = dynamic_cast<ir::CallInst *>(inst)) {
                            auto call_target = callinst->function;
                            if (!call_target)
                                throw std::runtime_error("Calling invalid function.");
                            rev_call_graph[call_target].push_back(func);
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