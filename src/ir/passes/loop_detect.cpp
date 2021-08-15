/**
 * Use dfs tree to detect natural loop
 * Reference
 * Compilers: Principles, Techniques, and Tools,
 * A. Aho, R. Sethi, and J. Ullman, Addison-Wesley, 1988. Edition 2
 * Chapters 9.6
 */
#include <ir/passes.h>
#include <algorithm>
#include <stack>
namespace ir_passes {
    typedef std::pair<ir::BasicBlock*, ir::BasicBlock*> Edge;

    class LoopDetector {
    private:
        ir::Function *func;

        std::set<Edge> advancingEdges;
        // Note: for reducible flow graphs, retreating edges and backward edges are equivalent.
        std::set<Edge> retreatingEdges;
        // Mapping: Basic block -> Loop
        std::unordered_map<ir::BasicBlock*, std::set<ir::Loop*>> blocks2loop;

        // check if a dominates b
        bool dominates(ir::BasicBlock *a, ir::BasicBlock *b) {
            while (a->dom_tree_depth < b->dom_tree_depth)
                b = b->idom;
            return a == b;
        }

        // divide the edges of the dominator tree into advancing edge and retreating edge
        void build() {
            for (auto * u : func->bList) {
                for (auto * v : u->succ()) {
                    // edge {b, i} is a loop retreating edge if i dominates b
                    if (dominates(v, u))
                        retreatingEdges.insert({u, v});
                    else
                        advancingEdges.insert({u, v});
                }
            }
        }

        void insert(ir::BasicBlock* m, std::stack<ir::BasicBlock*> &st, std::set<ir::BasicBlock *> &body) {
            if (body.find(m) == body.cend()) {
                body.insert(m);
                st.push(m);
                m->loop_depth = 0;
            }
        }

        void markNaturalLoops() {
            // detect natural loop for each retreating edge
            for (auto &edge : retreatingEdges) {
                auto *loop = new ir::Loop();
                func->loops.insert(loop);
                // Step1 detect natural loop
                auto &n = edge.first;
                auto &d = edge.second;
                std::stack<ir::BasicBlock *> st;
                loop->prehead = *(++std::find(func->bList.crbegin(), func->bList.crend(), d));
                loop->head = d;
                loop->body.insert(d);
                insert(n, st, loop->body);
                while (!st.empty()) {
                    auto *m = st.top();
                    st.pop();
                    // insert(p), p ∈ pred(m)
                    for (auto *pInst : m->parentInsts)
                        insert(pInst->bb, st, loop->body);
                }

                // Step2 determine the relationship between loop
                for (auto *bb : loop->body) {
                    auto iter = blocks2loop.find(bb);
                    if (iter == blocks2loop.cend()) continue;
                    for (auto &findLoop : iter->second) {
                        if (std::includes(findLoop->body.cbegin(), findLoop->body.cend(),
                                          loop->body.cbegin(), loop->body.cend())) {
                            // body ⊆ findLoop, body is the nested loop of findLoop
                            findLoop->nested.insert(loop);
                        } else if (std::includes(loop->body.cbegin(), loop->body.cend(),
                                                 findLoop->body.cbegin(), findLoop->body.cend())) {
                            // findLoop ⊆ body, body is the external loop of findLoop
                            loop->nested.insert(findLoop);
                        } else {
                            // neither body ⊆ findLoop nor findLoop ⊆ body
                            if (findLoop->head == d) {
                                // the same loop head, they should be merged into one
                                findLoop->body.insert(loop->body.cbegin(), loop->body.cend());
                                for (auto *currBB : loop->body) blocks2loop[currBB].insert(findLoop);
                                func->loops.erase(loop);
                                delete loop;
                                loop = nullptr;
                            } else {
                                // they are separated loop
                            }
                        }
                    }
                    if (!loop)
                        break;
                }

                if (!loop)
                    continue;

                // Step3 make sure constructed loop can be indexed
                for (auto *currBB : loop->body)
                    blocks2loop[currBB].insert(loop);

            }

            // mark deepest loop and reset loop depth
            for (auto *loop : func->loops) {
                if (loop->nested.empty())
                    func->deepestLoop.push_back(loop);
                loop->depth = 1;
            }
            // use number of parent nodes as loop depth.
            // count this by adding 1 to every nested loops for every loop
            for (auto *loop : func->loops) {
                for (auto &i:loop->nested)
                    i->depth++;
            }

            for (auto *loop : func->loops) {
                // mark direct external loop
                for (auto &i:loop->nested) {
                    if (i->depth - loop->depth == 1)
                        i->external = loop;
                }
                // update BB loop depth
                loop->updateBasicBlocks();
                // remove indirect nested loop
                for (auto it = loop->nested.begin(); it != loop->nested.end();) {
                    if ((*it)->depth - loop->depth != 1)
                        it = loop->nested.erase(it);
                    else
                        ++it;
                }
            }
        }

    public:
        LoopDetector(ir::Function* func) : func(func) {}

        void compute() {
            build();
            markNaturalLoops();
        }
    };

    void loop_detect_compute(ir::Module *module) {
        for (auto &i:module->functionList)
        {
            i->loops.clear();
            if (!i->bList.empty())
                LoopDetector{i}.compute();
        }
    }
}
