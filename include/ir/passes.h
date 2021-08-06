#ifndef TINBACCC_PASSES_H
#define TINBACCC_PASSES_H

#include <ir/ir.h>
#include <unordered_map>

namespace ir_passes {
    void remove_decl_uses(ir::Module *module);
    void remove_unreachable_bbs(ir::Module *module);
    void function_process(ir::Module *module);
    void rpo_compute(ir::Module *module);
    void dom_compute(ir::Module *module);
    void gvn(ir::Module *module);
    void gcm(ir::Module *module);


    class BasicBlockInfo {
    public:
        static bool need_scan; // some insts became dead in the last scan. need to scan again.
        ir::BasicBlock *irbb;
        bool revised; // This block is revised in a scan

        explicit BasicBlockInfo(ir::BasicBlock *ib, bool r = false) : revised(r), irbb(ib) {}

        bool isRevised() { return revised; }

        bool changeState(bool s, bool nr);

        static void changeRevisionState(bool nr) { need_scan = nr; }
    };
    std::unordered_map<ir::BasicBlock *, BasicBlockInfo *> bb2info;

    class InstInfo {
    public:
        ir::Inst *irinst;
        bool live;
        bool uList_in_map;
        bool is_revised;

        explicit InstInfo(ir::Inst *ii, bool l = true, bool u = false, bool r = false) : live(l), uList_in_map(u), is_revised(r), irinst(ii) {}

        bool isLiveInst() { return live; }

        bool allUsersinMap() { return uList_in_map; }

        bool changeState(bool s) { live = s; }
    };
    std::unordered_map<ir::Inst *, InstInfo *> inst2info;

    bool is_always_alive(ir::Inst *inst);
    void initalize(ir::Module *module);
    void detectDeadInst(ir::Module *module);
    void deleteDeadInst(ir::Module *module);
    void dce_copycat(ir::Module *module);
};
#endif //TINBACCC_PASSES_H
