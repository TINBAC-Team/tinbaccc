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

    /**
     * convert B<cond> L1; B L2; L1: toB<inv_cond> L2; B L1; L1:
     * @param module
     */
    void switch_branch_target(asm_arm::Module *module) {
        for (auto &f : module->functionList) {
            if (f->bList.size() < 2)
                continue;
            for (auto iter = f->bList.begin(); std::next(iter) != f->bList.end(); iter++) {
                const auto &curbb = *iter;
                if (curbb->insts.size() < 2)
                    continue;
                const auto &nextbb = *std::next(iter);
                auto nocond_branch = dynamic_cast<BInst *>(curbb->insts.back());
                auto cond_branch = dynamic_cast<BInst *>(*std::next(curbb->insts.rbegin()));
                if (!nocond_branch || !cond_branch)
                    continue;
                if (nocond_branch->isCondJP() || nocond_branch->tgt == nextbb || !cond_branch->isCondJP())
                    continue;
                if (cond_branch->tgt == nextbb) {
                    cond_branch->reverseCond();
                    std::swap(nocond_branch->tgt, cond_branch->tgt);
                }
            }
        }
    }

    /**
     * remove B .L; .L:
     * @param module
     */
    void remove_nop_branch(asm_arm::Module *module) {
        for (auto &f : module->functionList) {
            if (f->bList.size() < 2)
                continue;
            for (auto iter = f->bList.begin(); std::next(iter) != f->bList.end(); iter++) {
                const auto &curbb = *iter;
                const auto &nextbb = *std::next(iter);
                auto branch = dynamic_cast<BInst *>(curbb->insts.back());
                if (!branch || branch->isCondJP() || branch->append_pool || branch->tgt != nextbb)
                    continue;
                branch->mark_nop();
            }
        }
    }
}