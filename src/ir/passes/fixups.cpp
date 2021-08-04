#include <ir/passes.h>

namespace ir_passes {
    void remove_decl_uses(ir::Module *module) {
        for (auto &func:module->functionList) {
            for (auto &bb:func->bList) {
                for (auto &inst:bb->iList) {
                    std::list<ir::Use *> rm_uses;
                    for (auto u:inst->uList)
                        if (!u->user)
                            rm_uses.push_back(u);
                    for (auto rmu:rm_uses) {
                        inst->uList.erase(rmu);
                        rmu->value = nullptr;
                    }
                }
            }
        }
    }
}