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

        // divide the edges of the dominator tree into advancing edge and retreating edge
        void build() {
            for (auto * u : func->bList) {
                for (auto * v : u->succ()) {
                    // edge {b, i} is retreating edge if and only if dfs(b) >= dfs(i)
                    if (u->dfs_tree_depth >= v->dfs_tree_depth)
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
                loop->head = *(++std::find(func->bList.crbegin(),  func->bList.crend(), d));
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
                        // the same loop head
                        if (std::includes(findLoop->body.cbegin(), findLoop->body.cend(),
                                          loop->body.cbegin(), loop->body.cend())) {
                            // body ⊆ findLoop, body is the nested loop of findLoop
                            loop->external = findLoop;
                            findLoop->nested.push_back(loop);
                            break;
                        } else if (std::includes(loop->body.cbegin(), loop->body.cend(),
                                                 findLoop->body.cbegin(), findLoop->body.cend())) {
                            // findLoop ⊆ body, body is the external loop of findLoop
                            findLoop->external = loop;
                            findLoop->updateBasicBlocks();
                            loop->nested.push_back(findLoop);
                            break;
                        } else {
                            // neither body ⊆ findLoop nor findLoop ⊆ body
                            if (findLoop->head == d) {
                                // the same loop head, they should be merged into one
                                findLoop->body.insert(loop->body.cbegin(), loop->body.cend());
                                for (auto *currBB : loop->body) blocks2loop[currBB].insert(findLoop);
                                loop->updateBasicBlocks();
                                func->loops.erase(loop);
                                delete loop;
                                return;
                            } else {
                                // they are separated loop
                                break;
                            }
                        }
                    }
                }

                // Step3 make sure constructed loop can be indexed
                loop->updateBasicBlocks();
                for (auto *currBB : loop->body)
                    blocks2loop[currBB].insert(loop);

            }

            // mark deepest loop
            for (auto * loop : func->loops) {
                if (loop->nested.empty())
                    func->deepestLoop.push_back(loop);
                loop->depth = 1;
                auto * external = loop->external;
                while (external) {
                    loop->depth++;
                    external = external->external;
                }
                for (auto * bb : loop->body) {
                    if (bb->loop_depth < loop->depth)
                        bb->loop_depth = loop->depth;
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