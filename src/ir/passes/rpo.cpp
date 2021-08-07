/**
 * compute reverse postorder for a function.
 * to be confirmed: Aren't we already generating BBs in RPO?
 */
#include <ir/passes.h>

namespace ir_passes {
    typedef std::pair<ir::BasicBlock*, ir::BasicBlock*> Edge;

    class rpoDFS {
    private:
        ir::Function *func;
        std::set<ir::BasicBlock *> vis;


        void _dfs(ir::BasicBlock *b, int depth=1) {
            vis.insert(b);
            b->dfs_tree_depth = depth;
            for (auto &i:b->succ())
                if (vis.find(i) == vis.end()) {
                    _dfs(i, depth + 1);
                }
            func->rpoBList.push_front(b);
        }

    public:
        rpoDFS(ir::Function *f) : func(f) {
            func->rpoBList.clear();
        }

        void compute() {
            if (func->bList.empty())
                return;
            _dfs(func->bList.front());
            int rpo_index = 0;
            for (const auto &bb:func->bList) {
                bb->rpo_id = rpo_index++;
            }
        }
    };

    void rpo_compute(ir::Module *module) {
        for (auto &i:module->functionList)
            if (!i->bList.empty())
                rpoDFS(i).compute();
    }
}