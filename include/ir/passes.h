#ifndef TINBACCC_PASSES_H
#define TINBACCC_PASSES_H

#include <ir/ir.h>

namespace ir_passes {
    void remove_decl_uses(ir::Module *module);
    void simplify_cfg(ir::Module *module, bool remove_empty_bb);
    void function_process(ir::Module *module);
    void loop_detect_compute(ir::Module *module);
    void tail_call_elimination(ir::Module *module);
    void rpo_compute(ir::Module *module);
    void dom_compute(ir::Module *module);
    void gvn(ir::Module *module);
    void gcm(ir::Module *module);
}
#endif //TINBACCC_PASSES_H
