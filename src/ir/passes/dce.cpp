//
// Created by 28174 on 2021/8/6.
//
#include <ir/passes.h>

namespace ir_passes {
    bool BasicBlockInfo::changeState(bool s, bool nr) {
        revised = s;
        changeRevisionState(nr);
    }

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

    void initialize(ir::Module *module) {
        for (auto &func : module->functionList) {
            for (auto &bb : func->bList) {
                if (bb2info.find(bb) == bb2info.end())
                    bb2info[bb] = new BasicBlockInfo(bb);
                for (auto &in : bb->iList) {
                    auto cur_inst = dynamic_cast<ir::Inst *>(in);
                    if (!cur_inst) {
                        std::cerr << "Not a inst!\n";
                        continue;
                    }
                    if (inst2info.find(cur_inst) == inst2info.end())
                        inst2info[cur_inst] = new InstInfo(cur_inst);
                }
            }
        }
    }

    void detectDeadInst(ir::Module *module) {
        BasicBlockInfo::need_scan = false;
        do {
            BasicBlockInfo::changeRevisionState(false);
            for (auto &func : module->functionList) {
                for (auto &bb : func->bList) {
                    auto cur_bbinfo = bb2info[bb];
                    for (auto r_iter = bb->iList.rbegin(); r_iter != bb->iList.rend(); r_iter++) {
                        auto cur_inst = dynamic_cast<ir::Inst *>(*r_iter);
                        if (is_always_alive(cur_inst)) {
                            // every inst is alive after it is initialized
                            continue;
                        }
                        auto cur_instinfo = inst2info[cur_inst];
                        if (cur_inst->uList.empty()) {
                            cur_instinfo->changeState(false); // this inst is dead
                            cur_bbinfo->changeState(true, true); // this block should be revised
                            continue;
                        }
                        // now uList of cur_inst is not empty
                        if (!cur_instinfo->allUsersinMap()) { // I don't know if this is needed. Theoretically, all insts are in the map after initialization.
                            for (auto & _use : cur_inst->uList) {
                                auto _user = dynamic_cast<ir::Inst *>(_use->user); // TODO: Might have some problems here. I don't know if this is right.
                                if (inst2info.find(_user) == inst2info.end()) {
                                    inst2info[_user] = new InstInfo(_user);
                                }
                            }
                            inst2info[cur_inst]->uList_in_map = true;
                        }
                        // now all users of cur_inst is in the map inst2info.
                        bool flag = true;
                        for (auto & _use : cur_inst->uList) {
                            auto _user = dynamic_cast<ir::Inst *>(_use->user); // TODO: Might have some problems here. I don't know if this is right.
                            if (inst2info[_user]->isLiveInst()) {
                                flag = false;
                                break;
                            }
                        }
                        if (flag) { // all users are dead inst
                            cur_instinfo->changeState(false);
                            cur_bbinfo->changeState(true, true);
                        }
                    }
                }
            }
        } while (BasicBlockInfo::need_scan);
    }

    void deleteDeadInst(ir::Module *module) {
        for (auto &func : module->functionList) {
            for (auto &bb : func->bList) {
                if (!bb2info[bb]->isRevised())
                    continue;
                for (auto iter = bb->iList.begin(); iter != bb->iList.end();) {
                    auto _inst = dynamic_cast<ir::Inst *>(*iter);
                    if (_inst && !inst2info[_inst]->isLiveInst()) {
                        bb->iList.erase(iter++);
                        delete _inst;
                    }
                    else
                        iter++;
                }
                for (auto &_i : bb->iList) {
                    auto _inst = dynamic_cast<ir::Inst *>(_i);
                    if (!inst2info[_inst]->isLiveInst()) {
                        auto inst_p = _i;
                        bb->iList.remove(_i);
                    }
                }
            }
        }
    }

    void dce_copycat(ir::Module *module) {
        initialize(module);
        detectDeadInst(module);
        deleteDeadInst(module);
        return;
    }
}