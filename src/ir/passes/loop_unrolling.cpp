#include <ir/passes.h>
#include <ir/loop.h>
#include <algorithm>

using ir::LoopIR;
using ir::LoopVariable;

class LoopUnrolling {
private:
    ir::Function *function;

public:
    LoopUnrolling(ir::Function *function) : function(function) {}

    /**
     * Copy instruction to one basic block.
     * Note that loopIR and originLoopIR can be the same if the instructions to duplicate in the same basic block of
     * target. From loopIR, the loopVarDefine must filled.
     * @param loopIR  the loopIR corresponding to the body
     * @param body the basic block to insert instructions
     * @param originLoopIR the loopIR corresponding to the originInstList
     * @param originInstList the instructions to insert
     */
    void duplicateLoopBody(LoopIR *loopIR, ir::BasicBlock *body, const LoopIR *originLoopIR,
                           const std::vector<ir::Value *> &originInstList) {
        // Step1 prepare for value remapping
        std::unordered_map<ir::Value *, ir::Value *> value_map;
        auto get_val = [&](const ir::Use &u) -> ir::Value * {
            ir::Value *ret;
            if (originLoopIR->loopVarsByDefine.find(u.value) != loopIR->loopVarsByDefine.cend())
                if (loopIR == originLoopIR)
                    return loopIR->loopVarsByDefine[u.value]->loopVarDefine->GetRelatedValue(
                            loopIR->branchInst->true_block);
                else
                    // corresponding position
                    return loopIR->getCorrespondingPhiInst(originLoopIR, dynamic_cast<ir::PhiInst *>(u.value));
                else if (u.value->bb != originLoopIR->body)
                    return u.value; // special case: outside loop body
                    else if (value_map.find(u.value) != value_map.cend())
                        return value_map[u.value];
                    else if ((ret = dynamic_cast<ir::ConstValue *>(u.value)))
                        return ret;
                    else if ((ret = dynamic_cast<ir::GlobalVar *>(u.value)))
                        return ret;
                    else
                        throw std::runtime_error("value not defined!");
        };
        // Step2 duplicate each value
        for (auto *inst : originInstList) {
            if (auto x = dynamic_cast<ir::BinaryInst *>(inst)) {
                auto *dup = new ir::BinaryInst(x->optype, get_val(x->ValueL), get_val(x->ValueR));
                body->InsertAtEnd(dup);
                value_map[inst] = dup;
            } else if (auto x = dynamic_cast<ir::CallInst *>(inst)) {
                auto *dup = new ir::CallInst(x->fname, x->is_void, x->function);
                dup->params.reserve(x->params.size());
                for (auto &u:x->params)
                    dup->params.emplace_back(dup, get_val(u));
                body->InsertAtEnd(dup);
                value_map[inst] = dup;
            } else if (auto x = dynamic_cast<ir::LoadInst *>(inst)) {
                auto *dup = new ir::LoadInst(get_val(x->ptr));
                body->InsertAtEnd(dup);
                value_map[inst] = dup;
            } else if (auto x = dynamic_cast<ir::StoreInst *>(inst)) {
                auto *dup = new ir::StoreInst(get_val(x->ptr), get_val(x->val));
                body->InsertAtEnd(dup);
                value_map[inst] = dup;
            } else if (auto x = dynamic_cast<ir::GetElementPtrInst *>(inst)) {
                std::vector<ir::Value *> dims;
                dims.reserve(x->dims.size());
                for (const auto &u:x->dims)
                    dims.emplace_back(get_val(u));
                auto *dup = new ir::GetElementPtrInst(get_val(x->arr), dims, std::vector<int>());
                dup->multipliers = x->multipliers;
                if (auto gep_src = dynamic_cast<ir::GetElementPtrInst *>(get_val(x->arr)))
                    dup->decl = x->decl;
                body->InsertAtEnd(dup);
                value_map[inst] = dup;
            } else {
                throw std::runtime_error("unknown instruction to duplicate.");
            }
            // modify the phiNode of loopVar
            if (originLoopIR->loopVarsByBody.find(inst) != originLoopIR->loopVarsByBody.cend()) {
                if (loopIR == originLoopIR) {
                    for (auto *loopVar : loopIR->loopVarsByBody[inst]) {
                        // replace
                        loopVar->loopVarDefine->InsertElem(loopIR->branchInst->true_block, loopIR->body->iList.back());
                    }
                } else {
                    for(auto *originLoopVar : originLoopIR->loopVarsByBody.find(inst)->second) {
                        auto *loopVar = new LoopVariable();
                        loopVar->loop = loopIR->loop;
                        loopVar->loopVarInit = originLoopVar->loopVarDefine;
                        loopVar->loopVarDefine = loopIR->getCorrespondingPhiInst(originLoopIR,
                                                                                 originLoopVar->loopVarDefine);
                        loopVar->loopVarBody = loopIR->body->iList.back();
                        loopIR->loopVarsByDefine[loopVar->loopVarDefine] = loopVar;
                        loopIR->loopVarsByBody[loopIR->body->iList.back()].push_back(loopVar);
                    }

                }
            }

        }
    }

    void fixUnrollingLoopCondition(LoopIR *loopIR, int loopDelta, int loopResetCount) {
        ir::Use *toFix;
        if (loopIR->loopCondVar->loopVarDefine == loopIR->cmpInst->ValueL.value) {
            // left value
            toFix = &loopIR->cmpInst->ValueR;
        } else if (loopIR->loopCondVar->loopVarDefine == loopIR->cmpInst->ValueR.value) {
            toFix = &loopIR->cmpInst->ValueL;
        } else throw std::runtime_error("neither ValueL nor ValueR is loopCondVar.");

        if (auto x = dynamic_cast<ir::ConstValue *>(toFix->value)) {
            long long newValue;
            if (loopDelta > 0) {
                newValue = x->value - loopResetCount;
            } else {
                newValue = x->value + loopResetCount;
            }
            int castValue = (int)newValue;
            if (castValue == newValue) {
                // assert overflow will not occur
                *toFix = {loopIR->cmpInst, new ir::ConstValue(castValue)};
                return;
            }
        }
        // use conservative strategy to avoid overflow
        // while(i < n) -> while(i < n - K * delta) -> while(n - i > K * delta)

        loopIR->cmpInst->optype = flipOperator(loopIR->cmpInst->optype);
        ir::Use* useL = &loopIR->cmpInst->ValueL;
        ir::Use* useR = &loopIR->cmpInst->ValueR;
        if (useL->value != loopIR->loopCondVar->loopVarDefine) std::swap(useL, useR);
        auto * subInst = new ir::BinaryInst(ir::OpType::SUB, useR->value, loopIR->loopCondVar->loopVarDefine);

        auto iter = std::find(loopIR->cond->iList.begin(), loopIR->cond->iList.end(), loopIR->cmpInst);
        loopIR->cond->iList.insert(iter, subInst);
        subInst->bb = loopIR->cond;

        useL->use(subInst);
        useR->use(new ir::ConstValue(K * loopDelta));
    }

    void insertResetLoop(LoopIR *resetLoopIR, LoopIR *originLoopIR, std::vector<ir::Value *> &originInstList) {
        // Step1 prepare Loop struct
        auto *reset_loop = new ir::Loop();
        reset_loop->prehead = originLoopIR->body;
        reset_loop->depth = originLoopIR->loop->depth;
        auto *originCondVar = originLoopIR->loopCondVar;

        // Step2 prepare LoopIR struct
        resetLoopIR->loop = reset_loop;
        resetLoopIR->cond = new ir::BasicBlock("while_unrolling.entry");
        resetLoopIR->body = new ir::BasicBlock("while_unrolling.true");

        // Step3 prepare phi instructions, fill init value
        for (auto &originPhiInst : originLoopIR->phiInst) {
            auto *phiInst = new ir::PhiInst();
            phiInst->InsertElem(originLoopIR->cond, originPhiInst);
            resetLoopIR->phiInst.push_back(phiInst);
            resetLoopIR->cond->InsertAtEnd(phiInst);
            auto * originPhiFromTrueBlock = originPhiInst->GetRelatedValue(originLoopIR->body);
            auto findDefineInCond = originLoopIR->loopVarsByDefine.find(originPhiFromTrueBlock);
            if (findDefineInCond != originLoopIR->loopVarsByDefine.cend()) {
                phiInst->InsertElem(resetLoopIR->body, resetLoopIR->getCorrespondingPhiInst(originLoopIR, findDefineInCond->second->loopVarDefine));
            }
        }


        // Step4 insert the basic blocks
        auto iter = std::find(function->bList.begin(), function->bList.end(), originLoopIR->body);
        iter++;
        iter = function->bList.insert(iter, resetLoopIR->cond);
        iter++;
        function->bList.insert(iter, resetLoopIR->body);

        // Step5 generate IR for reset_loop body
        duplicateLoopBody(resetLoopIR, resetLoopIR->body, originLoopIR, originInstList);
        resetLoopIR->body->InsertAtEnd(new ir::JumpInst(resetLoopIR->cond));



        // Step6 generate IR for reset_loop cond
        resetLoopIR->loopCondVar =
                resetLoopIR->loopVarsByDefine[resetLoopIR->getCorrespondingPhiInst(originLoopIR,
                                                                                   originCondVar->loopVarDefine)];
        auto *valueL = originLoopIR->cmpInst->ValueL.value;
        auto *valueR = originLoopIR->cmpInst->ValueR.value;
        if (valueL == originCondVar->loopVarDefine) {
            valueL = resetLoopIR->loopCondVar->loopVarDefine;
        } else if (valueR == originCondVar->loopVarDefine) {
            valueR = resetLoopIR->loopCondVar->loopVarDefine;
        } else throw std::runtime_error("One of cmpInst\'s must be originCondVar.loopVarDefine.");
        resetLoopIR->cmpInst = new ir::BinaryInst(originLoopIR->cmpInst->optype, valueL, valueR);
        resetLoopIR->branchInst = new ir::BranchInst(resetLoopIR->cmpInst, resetLoopIR->body,
                                                     originLoopIR->branchInst->false_block);
        resetLoopIR->cond->InsertAtEnd(resetLoopIR->cmpInst);
        resetLoopIR->cond->InsertAtEnd(resetLoopIR->branchInst);

        // Step6 fill phi instruction
        for (auto &pair : resetLoopIR->loopVarsByBody) {
            for(auto *loopVar : pair.second) {
                auto *enterVar = pair.first;
                loopVar->loopVarDefine->InsertElem(resetLoopIR->branchInst->true_block, enterVar);
            }
        }

        // Step7 modify origin loop jumpInst
        originLoopIR->cond->replaceSucc(originLoopIR->branchInst->false_block, resetLoopIR->cond);

        // Step8 apply changes to loops
        function->deepestLoop.push_back(reset_loop);
        function->loops.insert(reset_loop);
        if (originLoopIR->loop->external)
            originLoopIR->loop->external->nested.insert(reset_loop);
        reset_loop->body.insert(resetLoopIR->cond);
        reset_loop->body.insert(resetLoopIR->body);

        // Step9 replace value
        std::set<ir::BasicBlock *> ignore;
        std::set_union(originLoopIR->loop->body.cbegin(), originLoopIR->loop->body.cend(),
                       reset_loop->body.cbegin(), reset_loop->body.cend(), std::inserter(ignore, ignore.cbegin()));

        auto resetIter = resetLoopIR->phiInst.begin();
        for (auto *phiInst : originLoopIR->phiInst) {
            std::vector<ir::Use *> usesCopy;
            usesCopy.assign(phiInst->uList.cbegin(), phiInst->uList.cend());
            for (auto &use : usesCopy) {
                if (ignore.find(use->user->bb) != ignore.cend()) continue; // ignore those two loop
                auto * resetPhi = *resetIter;
                if (!resetPhi->GetRelatedValue(resetLoopIR->branchInst->true_block)) continue;
                std::cout << "rep" << std::endl;
                use->use(resetPhi, true);
            }
            resetIter++;
        }
    }

    void makeRunOnce(ir::LoopIR *loopIR) {

        for (auto * phi : loopIR->phiInst) {
            std::set<ir::Use*> uList;
            for (auto * use : phi->uList) {
                if (use->user->bb == loopIR->body || use->user->bb == loopIR->cond)
                    uList.insert(use);
                else
                    use->use(phi->GetRelatedValue(loopIR->branchInst->true_block), false);
            }
            phi->uList = uList;
        }
        loopIR->cond->removeParent(loopIR->body);
        loopIR->body->iList.pop_back();
        loopIR->body->InsertAtEnd(new ir::JumpInst(loopIR->branchInst->false_block));
        loopIR->body->removeParent(loopIR->cond);
        loopIR->cond->iList.erase(std::find(loopIR->cond->iList.begin(), loopIR->cond->iList.end(), loopIR->branchInst));
        loopIR->cond->InsertAtEnd(new ir::JumpInst(loopIR->body));
        loopIR->branchInst->false_block->replacePred(loopIR->cond, loopIR->body);

        // it's unlikely, but just in case
        loopIR->cmpInst->replaceWith(new ir::ConstValue(0));
        loopIR->cond->iList.erase(std::find(loopIR->cond->iList.begin(), loopIR->cond->iList.end(), loopIR->cmpInst));

    }



    void unrolling(ir::Loop *loop) {
        // Step1 only consider one basic block
        if (loop->body.size() != 2) return;

        // Step2 Collect necessary information about loop cond
        LoopIR loopIR{};
        loopIR.init(loop);
        loopIR.build();
        int loopCount;
        int loopDelta;
        if (!loopIR.cmpInst || flipOperator(loopIR.cmpInst->optype) == loopIR.cmpInst->optype)
            return;
        // Step3 Process loop which execution times can be inferred
        if (constLoopCondAnalysis(&loopIR, loopCount, loopDelta) && loopCount >= 0) {
            if (loopCount == 0) {
                // remove loop where loopCount = 0
                //loopIR.cmpInst->replaceWith(new ir::ConstValue(0), true);
                std::cout << "replace meaningless cmpInst with const val 0" << std::endl;
                return;
            }
            int loopUnrollingCount = loopCount / K;
            int loopResetCount = loopCount - loopUnrollingCount * K;
            if (loopUnrollingCount == 0) return; // do nothing
            auto *jumpToCond = loopIR.body->iList.back();
            loopIR.body->iList.pop_back();
            std::vector<ir::Value *> iList;
            iList.assign(loopIR.body->iList.cbegin(), loopIR.body->iList.cend());

            for (int i = 0; i < K - 1; i++)
                duplicateLoopBody(&loopIR, loopIR.body, &loopIR, iList);
            loopIR.body->InsertAtEnd(jumpToCond);
            std::cout << "unrolling constLoopCond #1 (" << loopUnrollingCount << ")" << std::endl;

            if (loopResetCount > 0) {
                std::cout << "unrolling constLoopCond #2 (" << loopResetCount << ")" << std::endl;
                LoopIR resetLoopIR;
                insertResetLoop(&resetLoopIR, &loopIR, iList);
                fixUnrollingLoopCondition(&loopIR, loopDelta, loopResetCount);
                if (loopResetCount > 1) {
                    auto * resetLoopJump = resetLoopIR.body->iList.back();
                    resetLoopIR.body->iList.pop_back();
                    std::vector<ir::Value*> resetLoopList;
                    resetLoopList.assign(resetLoopIR.body->iList.cbegin(),  resetLoopIR.body->iList.cend());
                    for(int i=0; i < loopResetCount -1; i++) {
                        duplicateLoopBody(&resetLoopIR, resetLoopIR.body, &resetLoopIR, resetLoopList);
                    }
                    resetLoopIR.body->InsertAtEnd(resetLoopJump);
                }
                if (loopResetCount > 0) {
                    makeRunOnce(&resetLoopIR);
                }
            }
            if (loopUnrollingCount == 1) {
                makeRunOnce(&loopIR);
            }
        } else if (flexibleLoopAnalysis(&loopIR, loopDelta)) {
            auto *jumpToCond = loopIR.body->iList.back();
            loopIR.body->iList.pop_back();
            std::vector<ir::Value *> iList;
            iList.assign(loopIR.body->iList.cbegin(), loopIR.body->iList.cend());
            for (int i = 0; i < K - 1; i++)
                duplicateLoopBody(&loopIR, loopIR.body, &loopIR, iList);
            loopIR.body->InsertAtEnd(jumpToCond);
            std::cout << "unrolling flexibleLoopCond #1" << std::endl;
            LoopIR resetLoopIR;
            insertResetLoop(&resetLoopIR, &loopIR, iList);
            // handle two case: 1. the condVarInit is not const; 2. the bound is not const
            fixUnrollingLoopCondition(&loopIR, loopDelta, K);
            std::cout << "unrolling flexibleLoopCond #2" << std::endl;
        }
    }
};

void ir_passes::loop_unrolling(ir::Module *module) {

    for (auto *func : module->functionList) {
        LoopUnrolling unrollingFunc(func);
        for (auto *loop : std::vector<ir::Loop *>{func->deepestLoop.cbegin(), func->deepestLoop.cend()})
            unrollingFunc.unrolling(loop);
    }

}