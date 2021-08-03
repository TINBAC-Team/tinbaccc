//
// Created by colors_wind on 2021/8/3.
//

#include <asm_arm/optimization.h>

void asm_arm::architecture_optimize(asm_arm::Module *module) {
    for(auto *func : module->functionList) {
        livenessAnalysis(func);
        for (auto *bb : func->bList) {
            ArchitectureOptimizer(bb).process();
        }
    }

}

asm_arm::ArchitectureOptimizer::ArchitectureOptimizer(BasicBlock *bb) : bb(bb) {}

asm_arm::InstSet *asm_arm::ArchitectureOptimizer::getUse(Operand *key) {
    auto iter = use.find(key);
    if (iter != use.cend() && !iter->second.empty()) {
        return &iter->second;
    } return nullptr;
}

void asm_arm::ArchitectureOptimizer::build() {
    use.clear();
    for (auto iter=bb->insts.begin(); iter !=bb->insts.end(); iter++) {
        auto &inst = *iter;
        for (auto *n : inst->use) {
            use[n].insert(inst);
        }
        if (inst->op == Inst::Op::MUL || inst->op == Inst::Op::ADD)
            def[dynamic_cast<BinaryInst*>(inst)->dst] = iter;
    }
}

void asm_arm::ArchitectureOptimizer::process() {
    build();
    for(auto iter = bb->insts.begin();iter != bb->insts.cend();iter++) {
        tryCombineMLA(iter);
    }
}

void asm_arm::ArchitectureOptimizer::tryCombineMLA(asm_arm::InstLinkedList::iterator &iter) {
    auto &inst = *iter;
    if (inst->op != Inst::Op::ADD || inst->nop()) return;
    auto validMUL = [this](Operand* node) -> BinaryInst* {
        auto maybeMULIter = getDef(node);
        // check if the operand is generated from a InstMUL
        if (maybeMULIter == bb->insts.cend() || maybeMULIter.operator*()->op != Inst::Op::MUL) return nullptr;
        auto * instMUL = dynamic_cast<BinaryInst*>(*maybeMULIter);
        // check if the operand is only use in InstAdd
        auto * dep = getUse(instMUL->dst);
        if (!dep) throw std::runtime_error("Use indicates the inst is dead but it actually used in ADD inst.");
        if (dep->size() != 1) return nullptr;
        return instMUL;
    };
    auto *instADD = dynamic_cast<BinaryInst*>(inst);
    if (auto instMUL = validMUL(instADD->lhs)) {
        Operand* Rn;
        if (instADD->rhs->type == Operand::Type::Imm) {
            Rn = Operand::newVReg();
            auto * instMOV = new MOVInst(Rn, instADD->rhs);
            instMOV->bb = this->bb;
            iter = bb->insts.insert(iter, instMOV);
            iter++;
        } else Rn = instADD->rhs;
        instMUL->mark_nop();
        // replace
        *iter = new TernaryInst(Inst::Op::MLA, instADD->dst, instMUL->lhs, instMUL->rhs, Rn);
        iter.operator*()->bb = this-> bb;
    } else if (auto instMUL2 = validMUL(instADD->rhs)) {
        instMUL2->mark_nop();
        // replace
        *iter = new TernaryInst(Inst::Op::MLA, instADD->dst, instMUL2->lhs, instMUL2->rhs, instADD->lhs);
        iter.operator*()->bb = this-> bb;
    }
}




void asm_arm::livenessAnalysis(Function *function) {
    // calculate use and def
    for (auto &b : function->bList) {
        b->use.clear();
        b->def.clear();
        for (const auto &i:b->insts) {
            for (auto &u : i->use)
                if (u->type != Operand::Type::Imm && b->def.find(u) == b->def.end())
                    b->use.insert(u);

            for (auto &d : i->def)
                if (d->type != Operand::Type::Imm && b->use.find(d) == b->use.end())
                    b->def.insert(d);
        }
        b->liveIn = b->use;
        b->liveOut.clear();
    }

    // calculate liveOut and liveIn
    bool flag = true; // change flag
    while (flag) {
        flag = false;
        // in reverse topological order
        for (auto iter = function->bList.rbegin(); iter != function->bList.rend(); iter++) {
            auto *block = *iter;
            //  newOut := ∪ liveIn(s), s ∈ succ(n)
            OperandSet newOut;
            for (auto &s : block->succ())
                for (auto &i : s->liveIn)
                    newOut.insert(i);
            // liveOut changed <=> liveIn changed
            if (newOut != block->liveOut) {
                flag = true;
                block->liveOut = std::move(newOut);
                // use(n) ∪ (liveOut(n) − def (n))
                block->liveIn = block->use;
                for (const auto &i:block->liveOut) {
                    if (block->def.find(i) == block->def.end())
                        block->liveIn.insert(i);
                }
            }
        }
    }
    for (const auto &i:function->bList.front()->liveIn)
        if (i->type == Operand::Type::VReg)
            throw std::runtime_error("first block should not have any live VReg!");
}
