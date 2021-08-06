/**
 * Compute dominator tree of current IR module
 * Implementation of: A Simple, Fast Dominance Algorithm
 * https://www.cs.rice.edu/~keith/EMBED/dom.pdf
 */

#include <ir/passes.h>

namespace ir_passes {
    class DomComputer {
    private:
        ir::Function *func;


        ir::BasicBlock *intersect(ir::BasicBlock *b1, ir::BasicBlock *b2) {
            while (b1 != b2) {
                while (b1->rpo_id > b2->rpo_id)
                    b1 = b1->idom;
                while (b2->rpo_id > b1->rpo_id)
                    b2 = b2->idom;
            }
            return b1;
        }

    public:
        DomComputer(ir::Function *f) : func(f) {
            // doms[b] ← Undefined
            for (const auto &bb:f->bList)
                bb->idom = nullptr;
        }

        void compute() {
            // doms[start node] ← start node
            func->rpoBList.front()->idom = func->rpoBList.front();
            bool changed = true;
            while (changed) {
                changed = false;
                // for all nodes, b, in reverse postorder (except start node)
                auto it = std::next(func->rpoBList.begin());
                for (; it != func->rpoBList.end(); it++) {
                    auto &bb = *it;
                    ir::BasicBlock *new_idom = nullptr;
                    for (auto &predinst:bb->parentInsts) {
                        auto &pbb = predinst->bb;
                        // new_idom ← first (processed) predecessor of b /* (pick one) */
                        // we just skip unprocessed nodes here.
                        if (!pbb->idom)
                            continue;
                        // at this point doms[p] is always defined.
                        if (new_idom)
                            new_idom = intersect(pbb, new_idom);
                        else
                            new_idom = pbb;
                    }
                    if (bb->idom != new_idom) {
                        bb->idom = new_idom;
                        changed = true;
                    }
                }
            }
            // save dominator tree depth
            func->rpoBList.front()->dom_tree_depth = 0;
            auto it = std::next(func->rpoBList.begin());
            for (; it != func->rpoBList.end(); it++) {
                auto &bb = *it;
                if (bb->idom && bb->idom->dom_tree_depth != -1)
                    bb->dom_tree_depth = bb->idom->dom_tree_depth + 1;
            }
        }
    };

    void dom_compute(ir::Module *module) {
        for (auto &i:module->functionList)
            if (!i->bList.empty())
                DomComputer(i).compute();
    }
}