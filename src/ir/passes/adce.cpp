/**
* Aggressive dead code elimination
 *
 * with post-dominator and reverse-dominator-frontier.
 * Just throwing them here because they aren't used elsewhere.
*/
#include <ir/passes.h>
#include <iostream>
#include <queue>

#if 1
#define DBG(a) std::cerr << "adce: dbg: " << a << std::endl
#else
#define DBG()
#endif
namespace ir_passes {
    class AggresiveDCE {
    private :
        ir::Function *func;
        std::set<ir::BasicBlock *> vis;
        ir::BlockList exit_blocks;
        ir::BlockList reverse_rpo_blist;
        struct BlockInfo {
            int dfs_tree_depth = -1;
            int rpo_id = 0;
            ir::BasicBlock *idom = nullptr;
            bool idom_calculated = false;
            int dom_tree_depth = 0;
            ir::BlockList dom_frontier;
        };
        std::map<ir::BasicBlock *, BlockInfo> blk_info;

        void collect_exit_blocks();

        void do_dfs(ir::BasicBlock *b, int depth = 1);

        void dfs();

        void compute_postdom();

        void compute_rdf();

        ir::BasicBlock *intersect(ir::BasicBlock *b1, ir::BasicBlock *b2);

        void do_dce();

    public:
        AggresiveDCE(ir::Function *f) : func(f) {
        }

        void dce();
    };

    void AggresiveDCE::collect_exit_blocks() {
        exit_blocks.clear();
        for (auto &bb:func->bList) {
            if (bb->iList.empty())
                continue;
            if (bb->iList.back()->optype == ir::OpType::RETURN)
                exit_blocks.emplace_back(bb);
        }
    }

    void AggresiveDCE::do_dfs(ir::BasicBlock *b, int depth) {
        vis.insert(b);
        blk_info[b].dfs_tree_depth = depth;
        for (auto &inst:b->parentInsts) {
            auto &bb = inst->bb;
            if (vis.find(bb) == vis.end()) {
                do_dfs(bb, depth + 1);
            }
        }
        reverse_rpo_blist.push_front(b);
    }

    void AggresiveDCE::dfs() {
        if (func->bList.empty())
            return;
        vis.clear();
        reverse_rpo_blist.clear();
        for (auto &bb:exit_blocks)
            do_dfs(bb);
        int rpo_id = 0;
        for (auto &bb:reverse_rpo_blist)
            blk_info[bb].rpo_id = rpo_id++;
    }

    ir::BasicBlock *AggresiveDCE::intersect(ir::BasicBlock *b1, ir::BasicBlock *b2) {
        while (b1 != b2) {
            if (!b1 || !b2)
                return nullptr;
            while (blk_info[b1].rpo_id > blk_info[b2].rpo_id)
                b1 = blk_info[b1].idom;
            while (blk_info[b2].rpo_id > blk_info[b1].rpo_id)
                b2 = blk_info[b2].idom;
        }
        return b1;
    }

    void AggresiveDCE::compute_postdom() {
        // doms[start node] ← start node
        for (auto &i:exit_blocks) {
            blk_info[i].idom = nullptr;
            blk_info[i].idom_calculated = true;
        }
        bool changed = true;
        while (changed) {
            changed = false;
            // for all nodes, b, in reverse postorder (except start node)
            for (auto &bb:reverse_rpo_blist) {
                if (blk_info[bb].dfs_tree_depth <= 1)
                    continue;
                ir::BasicBlock *new_idom = nullptr;
                for (auto &pbb:bb->succ()) {
                    // new_idom ← first (processed) predecessor of b /* (pick one) */
                    // we just skip unprocessed nodes here.
                    if (!blk_info[pbb].idom_calculated)
                        continue;
                    // at this point doms[p] is always defined.
                    if (new_idom)
                        new_idom = intersect(pbb, new_idom);
                    else
                        new_idom = pbb;
                }
                if (!blk_info[bb].idom_calculated || blk_info[bb].idom != new_idom) {
                    blk_info[bb].idom = new_idom;
                    blk_info[bb].idom_calculated = true;
                    changed = true;
                }
            }
        }
        // save dominator tree depth
        for (auto &bb:exit_blocks)
            blk_info[bb].dom_tree_depth = 0;
        for (auto &bb:reverse_rpo_blist) {
            if (blk_info[bb].idom && blk_info[blk_info[bb].idom].dom_tree_depth != -1)
                blk_info[bb].dom_tree_depth = blk_info[blk_info[bb].idom].dom_tree_depth + 1;
        }
    }

    void AggresiveDCE::compute_rdf() {
        for (auto &bb:func->bList) {
            if (bb->succ().size() >= 2) {
                for (auto &succ:bb->succ()) {
                    auto runner = succ;
                    while (runner != blk_info[bb].idom) {
                        blk_info[runner].dom_frontier.emplace_back(bb);
                        runner = blk_info[runner].idom;
                    }
                }
            }
        }
    }

    void AggresiveDCE::do_dce() {
        std::queue<ir::Value *> worklist;
        std::set<ir::Value *> inst_mark;
        std::set<ir::BasicBlock *> block_mark;
        // Phase 1: mark
        for (auto &bb:reverse_rpo_blist) {
            for (auto &inst:bb->iList) {
                // critical instructions: instructions with side
                // effects except branch/jump
                if ((inst->optype != ir::OpType::BRANCH) &&
                    (inst->optype != ir::OpType::JUMP) &&
                    inst->sideEffect()) {
                    inst_mark.insert(inst);
                    block_mark.insert(inst->bb);
                    worklist.push(inst);
                }
            }
        }
        while (!worklist.empty()) {
            auto inst = worklist.front();
            worklist.pop();
            for (auto &u:inst->uses()) {
                if (inst_mark.find(u) != inst_mark.end())
                    continue;
                if (dynamic_cast<ir::Inst *>(u)) {
                    inst_mark.insert(u);
                    block_mark.insert(u->bb);
                    worklist.push(u);
                }
            }

            auto mark_rdf = [&](ir::BasicBlock *bb) {
                for (auto rdf:blk_info[bb].dom_frontier) {
                    auto rdf_branch = rdf->iList.back();
                    if (inst_mark.insert(rdf_branch).second) {
                        block_mark.insert(rdf);
                        worklist.push(rdf_branch);
                    }
                }
            };
            // PHI is a weird thing: we need to mark its incoming BBs.
            if (auto phi = dynamic_cast<ir::PhiInst *>(inst))
                for (auto &i:phi->phicont)
                    mark_rdf(i.first);
            mark_rdf(inst->bb);
        }
        // Phase 2: sweep
        int inst_count = 0;
        for (auto &bb:reverse_rpo_blist) {
            for (auto inst_it = bb->iList.begin(); inst_it != bb->iList.end();) {
                auto inst = *inst_it;
                if (inst_mark.find(inst) != inst_mark.end()) {
                    inst_it++;
                    continue;
                }
                if (inst->optype == ir::OpType::BRANCH) {
                    // rewrite branch to a jump to its nearest useful post dominator
                    ir::BasicBlock *tgt_bb = blk_info[bb].idom;
                    while (tgt_bb && block_mark.find(tgt_bb) == block_mark.end())
                        tgt_bb = blk_info[tgt_bb].idom;
                    if (!tgt_bb)
                        throw std::runtime_error("useless branch doesn't have a useful PD?");
                    DBG("rewrite branch in " << bb->name << " to a jump to " << tgt_bb->name);
                    // special handling of jumping into branch target:
                    auto branch = dynamic_cast<ir::BranchInst *>(inst);
                    tgt_bb->parentInsts.remove(branch);
                    if (branch->true_block == tgt_bb)
                        branch->true_block = nullptr;
                    if (branch->false_block == tgt_bb)
                        branch->false_block = nullptr;
                    // Branch should be the last instruction
                    bb->iList.erase(inst_it++);
                    delete inst;
                    // Insert a jump
                    bb->InsertAtEnd(new ir::JumpInst(tgt_bb));
                    continue;
                } else if (inst->optype != ir::OpType::JUMP) {
                    bb->iList.erase(inst_it++);
                    delete inst;
                    inst_count++;
                    continue;
                }
                inst_it++;
            }
        }
        if (inst_count)
            std::cerr << "adce: removed " << inst_count << " instructions from " << func->name << ".\n";
    }

    void AggresiveDCE::dce() {
        collect_exit_blocks();
        dfs();
        compute_postdom();
        compute_rdf();
        do_dce();
    }

    void aggressive_dce(ir::Module *module) {
        for (auto &func:module->functionList) {
            if (func->bList.empty())
                continue;
            AggresiveDCE(func).dce();
        }
    }
}
