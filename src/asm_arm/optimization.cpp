//
// Created by colors_wind on 2021/8/3.
//

#include <asm_arm/optimization.h>

void asm_arm::architecture_optimize(asm_arm::Module *module) {
    for (auto *func : module->functionList) {
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
    }
    return nullptr;
}

void asm_arm::ArchitectureOptimizer::build() {
    use.clear();
    for (auto iter = bb->insts.begin(); iter != bb->insts.end(); iter++) {
        auto &inst = *iter;
        for (auto *n : inst->use) {
            use[n].insert(inst);
        }
        if (inst->op == Inst::Op::MUL || inst->op == Inst::Op::ADD)
            def[dynamic_cast<BinaryInst *>(inst)->dst] = iter;
    }
}

void asm_arm::ArchitectureOptimizer::process() {
    build();
    for (auto iter = bb->insts.begin(); iter != bb->insts.cend(); iter++) {
        if ((*iter)->nop())
            continue;
        tryCombineMLA(iter);
    }
    tryCombineVMLA();

}

void asm_arm::ArchitectureOptimizer::tryCombineMLA(asm_arm::InstLinkedList::iterator &iter) {
    auto &inst = *iter;
    Inst::Op opType;
    switch (inst->op) {
        case Inst::Op::ADD:
            if (dynamic_cast<BinaryInst *>(inst)->lsl)
                return;
            opType = Inst::Op::MLA;
            break;
        case Inst::Op::SUB:
        case Inst::Op::RSB:
            opType = Inst::Op::MLS;
            break;
        default:
            return;
    }
    if (inst->nop()) return;

    auto validMUL = [this](Operand *node) -> BinaryInst * {
        auto maybeMULIter = getDef(node);
        // check if the operand is generated from a InstMUL
        if (maybeMULIter == bb->insts.cend() || maybeMULIter.operator*()->op != Inst::Op::MUL) return nullptr;
        auto *instMUL = dynamic_cast<BinaryInst *>(*maybeMULIter);
        // check if the operand is only use in InstAdd
        if (bb->liveOut.find(instMUL->dst) != bb->liveOut.end())
            return nullptr;
        auto *useList = getUse(instMUL->dst);
        if (!useList)
            throw std::runtime_error(
                    "Broken useList-def, this operand is used in ADD/SUB/RSB but can't be found in useList.");
        if (useList->size() != 1) return nullptr;
        return instMUL;
    };

    auto *instADD = dynamic_cast<BinaryInst *>(inst);
    BinaryInst *instMUL, *instMUL2;
    if ((instMUL = validMUL(instADD->lhs)) && (inst->op == Inst::Op::ADD || inst->op == Inst::Op::RSB)) {
        Operand *Rn;
        if (instADD->rhs->type == Operand::Type::Imm) {
            Rn = Operand::newVReg();
            auto *instMOV = new MOVInst(Rn, instADD->rhs);
            instMOV->bb = this->bb;
            iter = bb->insts.insert(iter, instMOV);
            iter++;
        } else Rn = instADD->rhs;
        instMUL->mark_nop();
        // replace
        *iter = new TernaryInst(opType, instADD->dst, instMUL->lhs, instMUL->rhs, Rn);
        iter.operator*()->bb = this->bb;
    } else if ((instMUL2 = validMUL(instADD->rhs)) && (inst->op == Inst::Op::ADD || inst->op == Inst::Op::SUB)) {
        instMUL2->mark_nop();
        // replace
        *iter = new TernaryInst(opType, instADD->dst, instMUL2->lhs, instMUL2->rhs, instADD->lhs);
        iter.operator*()->bb = this->bb;
    }
}

void asm_arm::ArchitectureOptimizer::tryCombineVMLA() {
    auto findMul = [this](const asm_arm::InstLinkedList::iterator &iter, SIMDQReg dst) {
        for(auto find=this->bb->insts.begin();find !=iter;find++) {
            if (auto *binaryInst = dynamic_cast<VBinaryInst *>(*find)) {
                if (binaryInst->op == Inst::Op::VMUL && binaryInst->dst == dst) return binaryInst;
            }
        }
        return (VBinaryInst *) nullptr;
        //throw std::runtime_error("CANNOT find inst with that operator.");
    };

    auto findUse = [this](SIMDQReg n) {
        std::vector<Inst *> uList;
        for (auto *inst : this->bb->insts) {
            if (auto x = dynamic_cast<VBinaryInst *>(inst)) {
                if (x->lhs == n || x->rhs == n) uList.push_back(x);
            } else if (auto x = dynamic_cast<VSTRInst *>(inst)) {
                if (x->src == n) uList.push_back(x);
            }
        }
        return uList;
    };

    auto replace = [this](const asm_arm::InstLinkedList::iterator &iter, SIMDQReg o, SIMDQReg n) {
        auto find=iter;
        find++;
        for(;find != bb->insts.cend();find++) {
            auto * inst = *find;
            if (auto x = dynamic_cast<VBinaryInst*>(inst)) {
                if (x->lhs == o) x->lhs = n;
                if (x->rhs == o) x->rhs = n;
            } else if (auto x = dynamic_cast<VSTRInst*>(inst)) {
                if (x->src == o) x->src = n;
            }
        }
    };

    for (auto iter = this->bb->insts.begin(); iter != bb->insts.cend(); iter++) {
        auto *inst = *iter;
        if (!inst->nop() && (inst->op == Inst::Op::VADD || inst->op == Inst::Op::VSUB)) {
            auto *binaryInst = dynamic_cast<VBinaryInst *>(inst);
            if (inst->op == Inst::Op::VADD) {
                auto *mulL = findMul(iter, binaryInst->lhs);
                auto *mulR = findMul(iter, binaryInst->rhs);
                if (mulL && findUse(binaryInst->lhs).size() == 1 && findUse(mulL->dst).size() == 1) {
                    auto dst = binaryInst->dst;
                    binaryInst->dst = binaryInst->rhs;
                    binaryInst->lhs = mulL->lhs;
                    binaryInst->rhs = mulL->rhs;
                    binaryInst->op = Inst::Op::VMLA;
                    replace(iter, dst, binaryInst->dst);
                    mulL->mark_nop(true);
                } else if (mulR && findUse(binaryInst->rhs).size() == 1 && findUse(mulR->dst).size() == 1) {
                    auto dst = binaryInst->dst;
                    binaryInst->dst = binaryInst->lhs;
                    binaryInst->lhs = mulR->lhs;
                    binaryInst->rhs = mulR->rhs;
                    binaryInst->op = Inst::Op::VMLA;
                    replace(iter, dst, binaryInst->dst);
                    mulR->mark_nop(true);
                } else continue;
            } else if (inst->op == Inst::Op::VSUB) {
                auto *mulR = findMul(iter, binaryInst->rhs);
                if (mulR && findUse(binaryInst->rhs).size() == 1 && findUse(mulR->dst).size() == 1) {
                    auto dst = binaryInst->dst;
                    binaryInst->dst = binaryInst->rhs;
                    binaryInst->lhs = mulR->lhs;
                    binaryInst->rhs = mulR->rhs;
                    binaryInst->op = Inst::Op::VMLS;
                    replace(iter, dst, binaryInst->dst);
                    mulR->mark_nop(true);
                } else continue;
            } else continue;
        }
    }
}


void asm_arm::livenessAnalysis(Function *function) {
    // calculate use and def
    for (auto &b : function->bList) {
        b->use.clear();
        b->def.clear();
        for (const auto &i:b->insts) {
            if (i->nop())
                continue;
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
