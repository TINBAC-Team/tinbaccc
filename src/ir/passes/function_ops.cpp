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
        std::set<ir::Function *> recursive_func;

        void collect_function_info();

        void remove_unused_function();

    public:
        FunctionProcessor(ir::Module *m) : module(m), work_done(false) {
            for (auto &func:module->functionList)
                func_name_map[func->name] = func;
        }

        void work();
    };

    void FunctionProcessor::collect_function_info() {
        call_count.clear();
        func_length.clear();
        recursive_func.clear();
        // preserve entry point
        call_count[func_name_map["main"]] = 1;
        for (auto &func:module->functionList) {
            for (auto &bb:func->bList) {
                func_length[func] += bb->iList.size();
                for (auto &inst:bb->iList) {
                    // check optype first to avoid too many vtable lookups
                    if (inst->optype != ir::OpType::CALL)
                        continue;
                    auto call_inst = dynamic_cast<ir::CallInst *>(inst);
                    auto call_target = func_name_map[call_inst->fname];
                    if (call_target == func)
                        recursive_func.insert(func);
                    call_count[call_target]++;
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

    void FunctionProcessor::work() {
        while (!work_done) {
            collect_function_info();
            remove_unused_function();
            work_done = true;
        }
    }

    void function_process(ir::Module *module) {
        FunctionProcessor(module).work();
    }
}