/**
 * What LLVM -simplifycfg does:
 * Performs dead code elimination and basic block merging. Specifically:
 *
 * Removes basic blocks with no predecessors.
 * Merges a basic block into its predecessor if there is only one and the predecessor only has one successor.
 * Eliminates PHI nodes for basic blocks with a single predecessor.
 * Eliminates a basic block that only contains an unconditional branch.
 */
#include <ir/passes.h>

namespace ir_passes {
    void simplify_cfg(ir::Module *module) {
        bool done = false;
        while (!done) {
            done = true;
            for (auto &func:module->functionList) {
                if (func->bList.size() < 2)
                    continue;
                auto bb_it = std::next(func->bList.begin());
                while (bb_it != func->bList.end()) {
                    auto &bb = *bb_it;

                    // 1. remove unreachable BBs
                    if (bb->parentInsts.empty()) {
                        done = false;
                        for (auto &succ:bb->succ()) {
                            succ->removeParent(bb);
                        }
                        func->unreachableBList.push_back(bb);
                        func->bList.erase(bb_it++);
                        continue;
                    }

                    // 2. eliminate trivial phi
                    // we can't iterate over instructions while deleting. Collect PHIs first.
                    std::vector<ir::PhiInst *> philist;
                    for (auto inst:bb->iList) {
                        if (inst->optype == ir::OpType::PHI)
                            philist.push_back(dynamic_cast<ir::PhiInst *>(inst));
                        else
                            break;
                    }
                    // this part is a duplication of tryRemoveTrivialPhi. However we don't have
                    // a builder here to handle Undef so it's copied instead.
                    for (auto phi:philist) {
                        ir::Value *same = nullptr;
                        bool trivial = true;
                        for (auto &op_it:phi->phicont) {
                            ir::Value *op = op_it.second->value;
                            if (op == same || op == phi)
                                continue; // first unique value or self-reference
                            if (same) {
                                trivial = false; // The phi merges at least two values: not trivial
                                break;
                            }
                            same = op;
                        }
                        if (!trivial)
                            break;
                        if (!same) // The phi is unreachable or in the start block
                            throw std::runtime_error("We can't have an undef PHI at this point!");

                        done = false;
                        // Replace all users of this Phi with same
                        phi->replaceWith(same);

                        // we are sweeping until nothing changes anyway.
                        // skip the recursion part in tryRemoveTrivialPhi

                        phi->bb->eraseInst(phi);
                        delete phi;
                    }

                    // 3. eliminate BBs with only a single unconditional branch
                    if (bb->parentInsts.size() == 1 && bb->iList.front()->optype == ir::OpType::JUMP) {
                        auto succ = bb->succ().front();
                        auto pred = bb->parentInsts.front()->bb;
                        // check if PHI in succ merges different value from pred and bb
                        bool can_be_eliminated = true;
                        for (auto i:succ->iList) {
                            if (auto phi = dynamic_cast<ir::PhiInst *>(i)) {
                                auto succ_val = phi->GetRelatedValue(pred);
                                if (!succ_val)
                                    continue;
                                if (succ_val != phi->GetRelatedValue(bb)) {
                                    can_be_eliminated = false;
                                    break;
                                }
                            } else {
                                break;
                            }
                        }
                        // If this doesn't work we can't do the next step on current BB. Skip it.
                        if (!can_be_eliminated) {
                            bb_it++;
                            continue;
                        }
                        done = false;
                        pred->replaceSucc(bb, succ);
                        succ->replacePred(bb, pred);
                        func->unreachableBList.push_back(bb);
                        func->bList.erase(bb_it++);
                        continue;
                    }

                    // 4. merge BB into its predecessor
                    // BB predecessor is determined by parentInsts and successor is determined with BB's
                    // ending branch. To merge BBs, we erase the last jump in predecessor and add all
                    // instructions into its predecessor including the branch in current BB.
                    if (bb->parentInsts.size() == 1 && bb->parentInsts.front()->optype == ir::OpType::JUMP) {
                        done = false;
                        auto pred = bb->parentInsts.front()->bb;
                        delete pred->iList.back();
                        pred->iList.pop_back();
                        auto insts = std::move(bb->iList);
                        for (auto &i:insts)
                            pred->InsertAtEnd(i);
                        for (auto succ:pred->succ())
                            succ->replacePred(bb, pred);
                        delete bb;
                        func->bList.erase(bb_it++);
                        continue;
                    }

                    bb_it++;
                }
            }
        }
    }
}