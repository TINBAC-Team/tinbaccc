//
// Created by 28174 on 2021/8/6.
//
#include <ir/passes.h>

namespace ir_passes {

    bool is_always_alive(ir::Inst *inst) {
        switch (inst->optype) {
            // isTerminator()
            case ir::OpType::RETURN:
            case ir::OpType::CALL:
            case ir::OpType::BRANCH:
            case ir::OpType::JUMP:
                return true;

            // mayHaveSideEffects()
            case ir::OpType::STORE:
                return true;
            default:
                return false;
        }
    }

    void deleteDeadInst(ir::Module *module) {
        bool need_scan = false;
        do {
            need_scan = false;
            for (auto &func : module->functionList) {
                for (auto &bb : func->bList) {
                    for (auto r_iter = bb->iList.rbegin(); r_iter != bb->iList.rbegin();) {
                        auto cur_inst = dynamic_cast<ir::Inst *>(*r_iter);
                        if (!cur_inst) {
                            r_iter++;
                            continue;
                        }
                        if (is_always_alive(cur_inst)) {
                            r_iter++;
                            continue;
                        }
                        if (cur_inst->uList.empty()) {
                            bb->iList.erase((++r_iter).base());
                            delete cur_inst;
                            need_scan = true;
                        }
                        else
                            ++r_iter;
                    }
                }
            }
        } while (need_scan);
    }




}