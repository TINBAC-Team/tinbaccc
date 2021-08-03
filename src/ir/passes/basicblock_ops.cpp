#include <ir/passes.h>

namespace ir_passes {
    void remove_unreachable_bbs(ir::Module *module) {
        bool done = false;
        while (!done) {
            done = true;
            for (auto &func:module->functionList) {
                if(func->bList.size()<2)
                    continue;
                auto bb_it = std::next(func->bList.begin());
                while (bb_it != func->bList.end()) {
                    auto &bb = *bb_it;
                    if (bb->parentInsts.empty()) {
                        done = false;
                        for (auto &succ:bb->succ()) {
                            succ->removeParent(bb);
                        }
                        func->unreachableBList.push_back(bb);
                        func->bList.erase(bb_it++);
                        continue;
                    }
                    bb_it++;
                }
            }
        }
    }
}