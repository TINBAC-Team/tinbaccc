#include <asm_arm/registers.h>
#include <stdexcept>

void asm_arm::RegisterAllocator::insertAdjList(asm_arm::Operand *u, asm_arm::Operand *v) {
    auto iter = adjList.find(u);
    if (iter == adjList.cend()) {
        adjList[u] = std::vector<Operand *>{v};
        degree[u] = 1;
    } else {
        iter->second.push_back(v);
        degree[u] = degree[u] + 1;
    }
}

void asm_arm::RegisterAllocator::build() {
    for (const auto &b : function->bList) {
        auto &live = b->liveOut;
        for (auto iter = b->insts.rbegin(); iter != b->insts.rend(); iter++) {
            auto *inst = *iter;
            auto *movInst = dynamic_cast<MOVInst *>(inst);
            if (movInst) {
                // live := live\use(I)
                for (const auto &x : movInst->use)
                    live.erase(x);
                // moveList[n] := moveList[n] ∪ {I}, n ∈ def(I)
                for (const auto &n : movInst->def)
                    moveList[n].insert(movInst);
                // moveList[n] := moveList[n] ∪ {I}, n ∈ use(I)
                for (const auto &n : movInst->use)
                    moveList[n].insert(movInst);
                // worklistMoves := worklistMoves ∪ {I}
                for (const auto &x : movInst->def)
                    worklistMoves.insert(movInst);
            }
            // live := live ∪ def(I)
            for (const auto &x : inst->def)
                live.insert(x);
            // AddEdge
            for (const auto &d : inst->def)
                for (const auto &l : live)
                    addEdge(l, d);

            // live := use(I) ∪ (live\def(I))
            for (const auto &x : inst->use)
                live.insert(x);
            for (const auto &x : inst->def)
                live.erase(x);

        }
    }
}

asm_arm::OperandList asm_arm::RegisterAllocator::adjacent(asm_arm::Operand *n) const {
    auto iter = adjList.find(n);
    if (iter == adjList.cend()) return {};
    OperandSet tmp;
    // tmp := selectStack ∪ coalescedNodes
    std::set_union(selectStack.rbegin(), selectStack.rend(),
                   coalescedNodes.begin(), coalescedNodes.end(),
                   std::inserter(tmp, tmp.cbegin()));
    OperandList result;
    // result := adjList[n] \ tmp
    std::set_difference(iter->second.cbegin(), iter->second.cend(),
                        tmp.cbegin(), tmp.cend(),
                        std::back_inserter(result));
    return std::move(result);
}

bool asm_arm::RegisterAllocator::isMoveRelated(asm_arm::Operand *n) {
    return !nodeMoves(n).empty();
}

void asm_arm::RegisterAllocator::mkWorklist() {
    for (auto iter = initial.cbegin(); iter != initial.cend(); iter++) {
        auto n = *iter;
        iter = initial.erase(iter);
        if (degree[n] >= K) // spill
            spillWorklist.insert(n);
        else if (isMoveRelated(n))
            freezeWorklist.insert(n);
        else
            simplifyWorklist.push_back(n);
    }
}

void asm_arm::RegisterAllocator::enableMoves(asm_arm::OperandList &nodes) {
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
    if (preColored.find(t) != preColored.cend())
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
    if (preColored.find(x) != preColored.cend())
        std::swap(x, y);
    auto &u = x, &v = y;

    worklistMoves.erase(worklistMoves.cbegin());
    if (u == v) {
        coalescedMoves.insert(m);
        addWorkList(x);
    } else if (preColored.find(v) != preColored.cend() || adjSet.find({u, v}) != adjSet.cend()) {
        constrainedMoves.insert(m);
        addWorkList(u);
        addWorkList(v);
    } else if (preColored.find(u) != preColored.cend() && [this, u, v]() {
        // u ∈ precolored ∧ (∀t ∈ Adjacent(v), isOK(t, u))
        auto adj = adjacent(v);
        return std::all_of(adj.cbegin(), adj.cend(), [this, u](Operand *t) { return isOK(t, u); });
    }() || ( preColored.find(u) == preColored.cend() && [this, u, v]() {
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
    if (preColored.find(u) == preColored.cend() && !isMoveRelated(u) && degree[u] < K) {
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
    Operand *m = *std::max_element(spilledNodes.cbegin(), spilledNodes.cend(),
                                  [this](Operand *a, Operand *b) { return degree[a] < degree[b]; });
    spilledNodes.erase(m);
    simplifyWorklist.push_back(m);
    throw std::runtime_error("Not implemented yet!");
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
            OperandSet tmp;
            std::set_union(coloredNodes.cbegin(), coloredNodes.cend(),
                           preColored.cbegin(), preColored.cend(),
                           std::inserter(tmp, tmp.cbegin()));
            if (tmp.find(getAlias(w)) != tmp.cend())
                okColors.erase(color[getAlias(w)]);
        }
        if (okColors.empty()) {
            spilledNodes.insert(n);
        } else {
            coloredNodes.push_back(n);
            color[n] = *okColors.cbegin();
        }
    }
    for (const auto & n : coloredNodes)
        color[n] = color[getAlias(n)];
}

void asm_arm::RegisterAllocator::rewriteProgram() {
    // initial := coloredNodes ∪ coalescedNodes ∪ {vi}
    initial.clear();
    std::set_union(coloredNodes.cbegin(), coloredNodes.cend(),
                   coloredNodes.cbegin(), coloredNodes.cend(),
                   std::inserter(initial, initial.cbegin()));
    for (const auto & v : spilledNodes) {
        // allocate memory locations and generate load and store instruction for spilled node
        int offs = function->allocate_stack(1);
        Operand *new_op = Operand::newVReg();
        for (auto &bb:function->bList) {
            for(auto inst_it = bb->insts.begin();inst_it!=bb->insts.end();inst_it++) {
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
    for (auto & b : function->bList)
        for (auto iter = b->insts.rbegin(); iter != b->insts.rend(); iter++) {
            auto & i = *iter;
            // def = def ∪ dst
            b->def.insert(i->def.cbegin(), i->def.cend());
            // use = (use \ def) ∪ src
            b->use.erase( i->def.cbegin(), i->def.cend());
            b->use.insert(i->use.cbegin(), i->use.cend());
        }

    // calculate liveOut and liveIn
    bool flag = true; // change flag
    while(flag) {
        flag = false;
        // in reverse topological order
        for(auto iter = function->bList.rbegin(); iter !=function->bList.rend();iter++) {
            auto * block = *iter;
            //  newOut := ∪ liveIn(s), s ∈ succ(n)
            OperandSet newOut;
            for (auto & s : block->succ()) newOut.insert(s->liveIn.cbegin(), s->liveIn.cend());
            // liveOut changed <=> liveIn changed
            if (newOut != block->liveOut) {
                flag = true;
                block->liveOut = std::move(newOut);
                // use(n) ∪ (liveOut(n) − def (n))
                OperandSet tmp;
                std::set_difference(block->liveOut.cbegin(), block->liveOut.cend(),
                                    block->def.cbegin(), block->def.cend(),
                                    std::inserter(tmp, tmp.cbegin()));
                block->liveIn.clear();
                std::set_union(block->use.cbegin(), block->use.cend(),
                               tmp.cbegin(), tmp.cend(),
                               std::inserter(block->liveIn, block->liveIn.cbegin()));
            }
        }
    }
}

void asm_arm::RegisterAllocator::addEdge(asm_arm::Operand *u, asm_arm::Operand *v) {
    adjSet.insert({u, v});
    if (preColored.find(u) == preColored.cend())
        insertAdjList(u, v);
    if (preColored.find(v) == preColored.cend())
        insertAdjList(v, u);
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
                          std::inserter(tmp, tmp.cbegin()));
    return std::move(result);
}

void asm_arm::RegisterAllocator::decrementDegree(asm_arm::Operand *m) {
    int d = degree[m];
    degree[m] = d - 1;
    if (d == K) {
        // {m} ∪ Adjacent(m))
        OperandList tmp = adjacent(m);
        tmp.push_back(m);

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

void asm_arm::RegisterAllocator::allocatedRegister() {
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
}
