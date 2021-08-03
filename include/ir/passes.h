#ifndef TINBACCC_PASSES_H
#define TINBACCC_PASSES_H

#include <ir/ir.h>

namespace ir_passes {
    void function_process(ir::Module *module);
    void rpo_compute(ir::Module *module);
    void dom_compute(ir::Module *module);
}
#endif //TINBACCC_PASSES_H
