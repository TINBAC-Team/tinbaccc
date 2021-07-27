#include <asm_arm/registers.h>
#include <stdexcept>
#include <iostream>

void asm_arm::RegisterAllocator::grabInitialVRegs() {
    for (const auto &b : function->bList) {
        for (const auto &i:b->insts) {
            for (auto &op :i->def)
                if (op->type == Operand::Type::VReg)
                    initial.insert(op);
            for (auto &op :i->use)
                if (op->type == Operand::Type::VReg)
                    initial.insert(op);
        }
    }
}

void asm_arm::RegisterAllocator::build() {
    for (const auto &b : function->bList) {
        auto &live = b->liveOut;
        for (auto iter = b->insts.rbegin(); iter != b->insts.rend(); iter++) {
            auto *inst = *iter;
            if (auto *movInst = dynamic_cast<MOVInst *>(inst)) {
                // live := live\use(I)
                for (const auto &x : movInst->use)
                    // XXX: do we even construct MOV IMM?
                    if (x->type != Operand::Type::Imm)
                        live.erase(x);
                // moveList[n] := moveList[n] ∪ {I}, n ∈ def(I)
                for (const auto &n : movInst->def)
                    moveList[n].insert(movInst);
                // moveList[n] := moveList[n] ∪ {I}, n ∈ use(I)
                for (const auto &n : movInst->use)
                    moveList[n].insert(movInst);
                // worklistMoves := worklistMoves ∪ {I}
                worklistMoves.insert(movInst);
            }
            // live := live ∪ def(I)
            for (auto & d : inst->def) live.insert(d);

            // AddEdge
            for (const auto &d : inst->def)
                for (const auto &l : live)
                    addEdge(l, d);

            // live := use(I) ∪ (live\def(I))
            for (auto & d : inst->def) live.erase(d);
            for (auto &u : inst->use)
                if (u->type != Operand::Type::Imm)
                    live.insert(u);
        }
    }
}

asm_arm::OperandSet asm_arm::RegisterAllocator::adjacent(asm_arm::Operand *n) const {
    auto iter = adjList.find(n);
    if (iter == adjList.cend()) return {};
    OperandSet result{iter->second.cbegin(), iter->second.cend()};
    // adjList[n] \ (selectStack ∪ coalescedNodes)
    for (auto &t : selectStack) result.erase(t);
    for (auto &t : coalescedNodes) result.erase(t);
    return std::move(result);
}

asm_arm::MOVInstSet asm_arm::RegisterAllocator::nodeMoves(asm_arm::Operand *n) {
    auto iter = moveList.find(n);
    if (iter == moveList.cend())
        return {};
    // tmp := activeMoves ∪ worklistMoves
    MOVInstSet tmp, result;
    std::set_union(activeMoves.cbegin(), activeMoves.cend(),
                   worklistMoves.cbegin(), worklistMoves.cend(),
                   std::inserter(tmp, tmp.cbegin()));
    // result := moveList[n] ∩ tmp
    std::set_intersection(iter->second.cbegin(), iter->second.cend(),
                          tmp.cbegin(), tmp.cend(),
                          std::inserter(result, result.cbegin()));
    return std::move(result);
}

bool asm_arm::RegisterAllocator::isMoveRelated(asm_arm::Operand *n) {
    return !nodeMoves(n).empty();
}

void asm_arm::RegisterAllocator::mkWorklist() {
    for (const auto &n:initial) {
        if (degree[n] >= K) // spill
            spillWorklist.insert(n);
        else if (isMoveRelated(n))
            freezeWorklist.insert(n);
        else
            simplifyWorklist.push_back(n);
    }
    initial.clear();
}

void asm_arm::RegisterAllocator::enableMoves(OperandSet &nodes) {
    for (const auto &n : nodes) {
        for (const auto &m : nodeMoves(n)) {
            auto iter = activeMoves.find(m);
            if (iter != activeMoves.cend()) {
                activeMoves.erase(iter);
                worklistMoves.insert(m);
            }
        }
    }
}

void asm_arm::RegisterAllocator::simplify() {
    if (simplifyWorklist.empty())
        throw std::runtime_error("SimplifyWorklist is empty!");
    auto n = simplifyWorklist.front();
    simplifyWorklist.erase(simplifyWorklist.cbegin());
    selectStack.push_back(n);
    for (const auto &m : adjacent(n)) {
        decrementDegree(m);
    }
}

bool asm_arm::RegisterAllocator::isOK(asm_arm::Operand *t, asm_arm::Operand *r) const {
    auto iter = degree.find(t);
    if ((iter != degree.cend() ? iter->second : 0) <= K)
        return true;
    if (t->type == Operand::Type::Reg)
        return true;
    if (adjSet.find({t, r}) != adjSet.cend())
        return true;
    return false;
}

asm_arm::Operand *asm_arm::RegisterAllocator::getAlias(asm_arm::Operand *n) {
    if (coalescedNodes.find(n) != coalescedNodes.cend())
        return getAlias(alias[n]);
    else
        return n;
}

bool asm_arm::RegisterAllocator::isConservative(asm_arm::OperandList &nodes) const {
    int k = 0;
    for (const auto &n : nodes) {
        auto iter = degree.find(n);
        if (iter != degree.cend() && iter->second >= K)
            k += 1;
    }
    return k < K;
}

void asm_arm::RegisterAllocator::coalesce() {
    if (worklistMoves.empty())
        throw std::runtime_error("WorklistMoves is empty!!!");
    auto * m = *worklistMoves.cbegin();
    auto x = getAlias(m->src);
    auto y = getAlias(m->dst);
    if (x->type == Operand::Type::Reg)
        std::swap(x, y);
    auto &u = x, &v = y;

    worklistMoves.erase(worklistMoves.cbegin());
    if (u == v) {
        coalescedMoves.insert(m);
        addWorkList(x);
    } else if (v->type == Operand::Type::Reg || adjSet.find({u, v}) != adjSet.cend()) {
        constrainedMoves.insert(m);
        addWorkList(u);
        addWorkList(v);
    } else if (u->type == Operand::Type::Reg && [this, u, v]() { // condition 1
        // u ∈ precolored ∧ (∀t ∈ Adjacent(v), isOK(t, u))
        auto adj = adjacent(v);
        return std::all_of(adj.cbegin(), adj.cend(), [this, u](Operand *t) { return isOK(t, u); });
    }() || ( u->type == Operand::Type::Reg && [this, u, v]() { // condition 2
        // u ∉ precolored ∧ isConservative(Adjacent(u) ∪ Adjacent(v)
        auto adjU = adjacent(u);
        auto adjV = adjacent(v);
        OperandList adjUnion;
        std::set_union(adjU.cbegin(), adjU.cend(), adjV.cbegin(), adjV.cend(), std::back_inserter(adjUnion));
        return isConservative(adjUnion);
    }())) {
        coalescedMoves.insert(m);
        combine(u, v);
        addWorkList(u);
    } else {
        activeMoves.insert(m);
    }
}

void asm_arm::RegisterAllocator::addWorkList(asm_arm::Operand *u) {
    // u ∈ precolored ∧ not(MoveRelated(u)) ∧ degree[u] < K
    if (u->type != Operand::Type::Reg && !isMoveRelated(u) && degree[u] < K) {
        freezeWorklist.erase(u);
        simplifyWorklist.push_back(u);
    }
}

void asm_arm::RegisterAllocator::combine(asm_arm::Operand *u, asm_arm::Operand *v) {
    if (freezeWorklist.find(v) != freezeWorklist.cend())
        freezeWorklist.erase(v);
    else
        spillWorklist.erase(v);

    coalescedNodes.insert(v);
    alias[v] = u;

    // nodeMoves[u] := nodeMoves[u] ∪ nodeMoves[v]
    auto &s1 = moveList[u];
    auto &s2 = moveList[v];
    for (const auto &tmp : s2) s1.insert(tmp);

    for (const auto &t : adjacent(v)) {
        addEdge(t, u);
        decrementDegree(t);
    }

    if (degree[u] >= K && freezeWorklist.find(u) != freezeWorklist.cend()) {
        freezeWorklist.erase(u);
        spillWorklist.insert(u);
    }
}

void asm_arm::RegisterAllocator::selectSpill() {
    // TODO Use more reasonable cost estimates to design heuristic algorithms.
    // Simply choose spilling node with maximum degree
    Operand *m = *std::max_element(spillWorklist.cbegin(), spillWorklist.cend(),
                                  [this](Operand *a, Operand *b) { return degree[a] < degree[b]; });
    spillWorklist.erase(m);
    simplifyWorklist.push_back(m);
    freezeMoves(m);
}

void asm_arm::RegisterAllocator::freezeMoves(asm_arm::Operand *u) {
    for (const auto & m : nodeMoves(u)) {
        if (activeMoves.find(m) != activeMoves.cend())
            activeMoves.erase(m);
        else
            worklistMoves.erase(m);
        frozenMoves.insert(m);
        auto v = m->dst;
        if (nodeMoves(v).empty() && degree[v] < K) {
            freezeWorklist.erase(v);
            simplifyWorklist.push_back(v);
        }
    }
}

void asm_arm::RegisterAllocator::assignColors() {
    while (!selectStack.empty()) {
        auto n = selectStack.back();
        selectStack.pop_back();
        ColorSet okColors;
        for(int i=0;i<K;i++) okColors.insert(i);
        for (const auto w : adjList[n]) {
            // tmp := coloredNodes ∪ precolored
            auto tmp = getAlias(w);
            // FIXME: lr can't be casted to int
            if (tmp->type == Operand::Type::Reg)
                okColors.erase(static_cast<int>(tmp->reg));
            else if (coloredNodes.find(tmp) != coloredNodes.end())
                okColors.erase(color[getAlias(w)]);
        }
        if (okColors.empty()) {
            spilledNodes.insert(n);
        } else {
            coloredNodes.insert(n);
            color[n] = *okColors.cbegin();
        }
    }
    for (const auto & n : coalescedNodes)
        color[n] = color[getAlias(n)];
}

void asm_arm::RegisterAllocator::rewriteProgram() {
    // initial := coloredNodes ∪ coalescedNodes ∪ {vi}
    initial.clear();
    std::set_union(coloredNodes.cbegin(), coloredNodes.cend(),
                   coalescedNodes.cbegin(), coalescedNodes.cend(),
                   std::inserter(initial, initial.cbegin()));
    for (const auto & v : spilledNodes) {
        // allocate memory locations and generate load and store instruction for spilled node
        int offs = function->allocate_stack(1);
        Operand *new_op = Operand::newVReg();
        for (auto &bb:function->bList) {
            for(auto inst_it = bb->insts.begin();inst_it!=bb->insts.end();inst_it++) {
                // in case we generated spill code between function call instructions.
                if((*inst_it)->move_stack)
                    offs -= (*inst_it)->move_stack;
                if((*inst_it)->replace_use(v, new_op)) {
                    Operand *ldr_offs_op;
                    if(Operand::op2Imm(offs)) {
                    ldr_offs_op = Operand::newImm(offs);
                    } else {
                        ldr_offs_op = Operand::newVReg();
                        auto ldrimm = new LDRInst(offs,new_op);
                        bb->insts.insert(inst_it, ldrimm);
                    }
                    auto ldrinst = new LDRInst(new_op, Operand::getReg(Reg::sp), ldr_offs_op);
                    bb->insts.insert(inst_it, ldrinst);
                    initial.insert(new_op);
                    new_op = Operand::newVReg();
                }
                if((*inst_it)->replace_def(v, new_op)) {
                    auto inst_next=std::next(inst_it);
                    Operand *str_offs_op;
                    if(Operand::op2Imm(offs)) {
                        str_offs_op = Operand::newImm(offs);
                    } else {
                        str_offs_op = Operand::newVReg();
                        auto ldrimm = new LDRInst(offs,new_op);
                        bb->insts.insert(inst_it, ldrimm);
                    }
                    auto strinst = new STRInst(new_op, Operand::getReg(Reg::sp), str_offs_op);
                    bb->insts.insert(inst_it, strinst);
                    initial.insert(new_op);
                    new_op = Operand::newVReg();
                }
            }
        }
        delete new_op;
    }
    coloredNodes.clear();
    coalescedNodes.clear();
}

void asm_arm::RegisterAllocator::livenessAnalysis() {
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
}

void asm_arm::RegisterAllocator::addEdge(asm_arm::Operand *u, asm_arm::Operand *v) {
    if (adjSet.find({u, v}) == adjSet.cend() && u != v) {
        adjSet.insert({u, v});
        adjSet.insert({v, u});
        if (u->type == Operand::Type::VReg) {
            adjList[u].push_back(v);
            degree[u] += 1;
        }
        if (v->type == Operand::Type::VReg) {
            adjList[v].push_back(u);
            degree[v] += 1;
        }
    }
}



void asm_arm::RegisterAllocator::decrementDegree(asm_arm::Operand *m) {
    int d = degree[m];
    degree[m] = d - 1;
    if (d == K) {
        // {m} ∪ Adjacent(m))
        OperandSet tmp = adjacent(m);
        tmp.insert(m);

        enableMoves(tmp);
        spillWorklist.erase(m);

        if (isMoveRelated(m))
            freezeWorklist.insert(m);
        else
            simplifyWorklist.push_back(m);

    }
}

void asm_arm::RegisterAllocator::freeze() {
    if (freezeWorklist.empty())
        throw std::runtime_error("FreezeWorklist is empty!");
    auto u = *freezeWorklist.cbegin();
    freezeWorklist.erase(freezeWorklist.cbegin());
    simplifyWorklist.push_back(u);
    freezeMoves(u);
}
#include <iostream>
void asm_arm::RegisterAllocator::allocatedRegister(asm_arm::Function *func) {
    this->function = func;
    grabInitialVRegs();
    while(true) {
        livenessAnalysis();
        build();
        mkWorklist();
        do {
            if (!simplifyWorklist.empty())
                simplify();
            else if (!worklistMoves.empty())
                coalesce();
            else if (!freezeWorklist.empty())
                freeze();
            else if (!spillWorklist.empty())
                selectSpill();
        } while (!(simplifyWorklist.empty() && worklistMoves.empty() && freezeWorklist.empty() &&
                   spillWorklist.empty()));
        assignColors();
        if (spillWorklist.empty()) break;
        rewriteProgram();
    }
    int x;
    for (auto node : coloredNodes) {
        int reg = color[node];
        node->assignReg(reg);
        if (reg > function->max_reg)
            function->max_reg = reg;
    }

}

namespace asm_arm {
    void allocate_register(asm_arm::Module *module) {
        for (auto &i:module->functionList) {
            asm_arm::RegisterAllocator allocator;
            allocator.allocatedRegister(i);
        }

    }
}
