#include <asm_arm/instructions.h>

namespace asm_arm {
    void function_param_pointer_fixup(asm_arm::Module *module) {
        for (auto &func:module->functionList) {
            if (func->stack_size & 7)
                func->stack_size += 4;
            int stack_movement = func->stack_size;
            // lr
            stack_movement += 4;
            if (func->max_reg >= 12)
                stack_movement += 8 * 4;
            else if (func->max_reg >= 4)
                stack_movement += (func->max_reg - 4 + 1) * 4;

            for (auto &param:func->param_fixup_list)
                param->val += stack_movement;

            // fixup illegal ldr offset like this:
            // LDR Rd, [Rs, offs] ->
            // LDR Rd, =offs
            // LDR Rd, [Rs, Rd]
            // This should only be produced in param pointer for now, so just fix the first block.
            for (auto it = func->bList.front()->insts.begin(); it != func->bList.front()->insts.end(); it++) {
                auto ldr = dynamic_cast<LDRInst *>(*it);
                // only look for LDR
                if (!ldr)
                    continue;
                // skip LDR Rd,=const
                if(ldr->type!=LDRInst::Type::REGOFFS)
                    continue;
                // skip legal imm12 and reg offset
                if(ldr->offs->type!=Operand::Type::Imm || ldr->offs->val < 4096)
                    continue;
                if(ldr->dst->reg==ldr->src->reg)
                    throw std::runtime_error("Illegal Imm offset can't be auto fixed.");
                int offs = ldr->offs->val;
                delete ldr->offs;
                ldr->offs = ldr->dst;
                auto inst = new LDRInst(offs, Operand::getReg(ldr->dst->reg));
                func->bList.front()->insts.insert(it, inst);
            }
        }
    }

    void branchinst_fixup(asm_arm::Module *module) {
        for (auto &f : module->functionList) {
            for (auto &b : f->bList) {
                auto inst0 = b->insts.end();
                if (b->insts.size() >= 2) {
                    inst0--; // last inst
                    if (auto _binst_last1 = dynamic_cast<BInst *>(*inst0)) {
                        if (b != *(f->bList.end()) && _binst_last1->cond == Inst::OpCond::NONE) {
                            if (_binst_last1->tgt == std::next(b)) { // last inst is unconditional jump and tgt is the next bb
                                // b->pass(inst0); // erase last inst
                            }
                            else {
                                inst0--;
                                if (auto _binst_last2 = dynamic_cast<BInst *>(*inst0)) {
                                    if (_binst_last2->isCondJP() && _binst_last2->tgt == std::next(b)) { // second to last inst is conditional jump and tgt is the next bb
                                        _binst_last2->reverseCond(); // change condition
                                        std::swap(_binst_last1->tgt, _binst_last2->tgt); // swap the tgt
                                        inst0++;
                                        // b->pass(inst0); // erase the last inst
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}