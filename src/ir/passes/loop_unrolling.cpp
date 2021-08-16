#include <ir/passes.h>
#include <algorithm>

const int K = 4;
const int DELTA_LIMIT = 64;

static ir::OpType flipOperator(ir::OpType opType) {
    switch (opType) {
        case ir::OpType::SLT:
            return ir::OpType::SGT;
        case ir::OpType::SLE:
            return ir::OpType::SGE;
        case ir::OpType::SGT:
            return ir::OpType::SLT;
        case ir::OpType::SGE:
            return ir::OpType::SLE;
        default:
            return opType;
    }
}

/**
 * Represent a loop variable which is defined outside the loop body and use inside the loop body.
 */
struct LoopVariable {
    ir::Loop *loop = nullptr;
    ir::Value *loopVarInit = nullptr;
    ir::PhiInst *loopVarDefine = nullptr;
    ir::Value *loopVarBody = nullptr;

    void init(ir::Loop *loop, ir::BranchInst *branchInst, ir::PhiInst *defineInst) {
        this->loop = loop;
        this->loopVarDefine = defineInst;
        this->loopVarInit = loopVarDefine->GetRelatedValue(loop->prehead);
        this->loopVarBody = loopVarDefine->GetRelatedValue(branchInst->true_block);
    }

};

/**
 * Represent a loop with single basic block (IR form)
 */
struct LoopIR {
    ir::Loop *loop = nullptr;
    ir::BasicBlock *cond = nullptr;
    ir::BasicBlock *body = nullptr;
    ir::BinaryInst *cmpInst = nullptr;
    ir::BranchInst *branchInst = nullptr;

    LoopVariable *loopCondVar = nullptr;

    std::vector<ir::PhiInst *> phiInst;
    std::unordered_map<ir::Value *, LoopVariable *> loopVarsByDefine;
    // a loopVarEnter may correspond to multiple variables (it may be use in one more phiInst)
    std::unordered_map<ir::Value *, std::vector<LoopVariable *>> loopVarsByEnter;

    ir::PhiInst *getCorrespondingPhiInst(const LoopIR *loopIR, const ir::PhiInst *phiInst) {
        auto iter = std::find(loopIR->phiInst.begin(), loopIR->phiInst.end(), phiInst);
        if (iter == loopIR->phiInst.cend()) return nullptr;
        int index = iter - loopIR->phiInst.cbegin();
        return this->phiInst[index];
    }

    void build() {
        for (auto *inst : cond->iList) {
            if (auto *x = dynamic_cast<ir::PhiInst *>(inst)) {
                phiInst.push_back(x);
                auto *loopVar = new LoopVariable();
                loopVar->init(loop, branchInst, x);
                loopVarsByDefine[x] = loopVar;
                loopVarsByEnter[x->GetRelatedValue(branchInst->true_block)].push_back(loopVar);
            }
        }
    }

    void init(ir::Loop *loop) {
        this->loop = loop;
        this->cond = *loop->body.cbegin();
        this->body = *loop->body.rbegin();
        if (*loop->prehead->succ().cbegin() != cond) std::swap(this->cond, this->body);

        for (auto & iter : cond->iList) {
            if (auto * cmpInst = dynamic_cast<ir::BinaryInst *>(iter)) {
                if (cmpInst->is_icmp()) this->cmpInst = cmpInst;
            } else if (auto * branchInst = dynamic_cast<ir::BranchInst*>(iter)) {
                this->branchInst = branchInst;
            }
        }
    }

    virtual ~LoopIR() {
        for (auto pair : loopVarsByDefine) delete pair.second;
        loopVarsByDefine.clear();
        loopVarsByEnter.clear();
    }
};

class LoopUnrolling {
private:
    ir::Function *function;

public:
    LoopUnrolling(ir::Function *function) : function(function) {}

    bool tryInferLoopVarDelta(LoopVariable *loopVar, ir::BasicBlock *scope, int &delta) {
        delta = 0;
        auto *currInst = loopVar->loopVarBody;
        while (currInst != loopVar->loopVarDefine && currInst->bb == scope) {
            auto *binaryInst = dynamic_cast<ir::BinaryInst *>(currInst);
            if (!binaryInst) return false;
            if (binaryInst->optype == ir::OpType::ADD) {
                auto *opL = dynamic_cast<ir::ConstValue *>(binaryInst->ValueL.value);
                auto *opR = dynamic_cast<ir::ConstValue *>(binaryInst->ValueR.value);
                if (opL && !opR) {
                    delta += opL->value;
                    currInst = binaryInst->ValueR.value;
                } else if (!opL && opR) {
                    delta += opR->value;
                    currInst = binaryInst->ValueL.value;
                } else return false;
            } else if (binaryInst->optype == ir::OpType::SUB) {
                auto *opR = dynamic_cast<ir::ConstValue *>(binaryInst->ValueR.value);
                if (opR) {
                    delta -= opR->value;
                    currInst = binaryInst->ValueR.value;
                } else return false;
            } else return false;
        }
        return delta != 0 && abs(delta) <= DELTA_LIMIT;
    }

    // loopVar OP cond
    bool tryInferLoopCount(ir::OpType opType, int init, int cond, long delta, int &cnt) {
        // after n-1 loop execution, loopVar := init + (n-1) * delta
        // and the n-th loop condition: loopVar OP cond
        // therefore, cnt := floor((cond - init) / delta) + 1 (SLE)
        switch (opType) {
            case ir::OpType::SLT:
                if (init < cond) cnt = (cond - init - 1) / delta + 1;
                else cnt = 0;
                break;
            case ir::OpType::SLE:
                if (init <= cond) cnt = (cond - init) / delta + 1;
                else cnt = 0;
                break;
            case ir::OpType::SGT:
                if (init > cond) cnt = (init - cond - 1) / -delta + 1;
                else cnt = 0;
                break;
            case ir::OpType::SGE:
                if (init >= cond) cnt = (init - cond) / -delta + 1;
                else cnt = 0;
                break;
            default:
                return false;
        }
        return true;
    }

    bool constLoopCondAnalysis(LoopIR *loopIR, int &loopCount, int &loopDelta) {
        auto *&cmpInst = loopIR->cmpInst;
        auto *&branchInst = loopIR->branchInst;
        auto *cmpValueL = dynamic_cast<ir::ConstValue *>(cmpInst->ValueL.value);
        auto *cmpValueR = dynamic_cast<ir::ConstValue *>(cmpInst->ValueR.value);
        loopDelta = 0;
        if (cmpValueR && !cmpValueL) {
            // left operator is loopVar and right operator is const
            auto *phiInst = dynamic_cast<ir::PhiInst *>(cmpInst->ValueL.value);
            if (!phiInst) return false;
            auto iter = loopIR->loopVarsByDefine.find(cmpInst->ValueL.value);
            if (iter == loopIR->loopVarsByDefine.cend())
                throw std::runtime_error("Cannot find loopCondVar from ValueL of cmpInst.");
            loopIR->loopCondVar = iter->second;
            auto *loopVarInit = dynamic_cast<ir::ConstValue *>(loopIR->loopCondVar->loopVarInit);
            if (!loopVarInit) return false;
            int loopInit = loopVarInit->value;
            int condConst = cmpValueR->value;
            if (tryInferLoopVarDelta(loopIR->loopCondVar, branchInst->true_block, loopDelta)
                && tryInferLoopCount(cmpInst->optype, loopInit, condConst, loopDelta, loopCount)) {
                return true;
            }
        } else if (cmpValueL && !cmpValueR) {
            // right operator is loopVar and left operator is const
            auto *phiInst = dynamic_cast<ir::PhiInst *>(cmpInst->ValueR.value);
            if (!phiInst) return false;
            auto iter = loopIR->loopVarsByDefine.find(cmpInst->ValueR.value);
            if (iter == loopIR->loopVarsByDefine.cend())
                throw std::runtime_error("Cannot find loopCondVar from ValueR of cmpInst.");
            loopIR->loopCondVar = iter->second;
            loopIR->loopCondVar->init(loopIR->loop, branchInst, dynamic_cast<ir::PhiInst *>(phiInst));
            auto *loopVarInit = dynamic_cast<ir::ConstValue *>(loopIR->loopCondVar->loopVarInit);
            if (!loopVarInit) return false;
            int loopInit = loopVarInit->value;
            int condConst = cmpValueL->value;
            ir::OpType opType = flipOperator(cmpInst->optype);
            if (tryInferLoopVarDelta(loopIR->loopCondVar, branchInst->true_block, loopDelta)
                && tryInferLoopCount(opType, loopInit, condConst, loopDelta, loopCount)) {
                return true;
            }
        }
        return false;
    }

    bool flexibleLoopAnalysis(LoopIR *loopIR, int &loopCount, int &loopDelta) {
        auto *&cmpInst = loopIR->cmpInst;
        auto *&branchInst = loopIR->branchInst;
        auto *cmpValueL = dynamic_cast<ir::PhiInst *>(cmpInst->ValueL.value);
        auto *cmpValueR = dynamic_cast<ir::PhiInst *>(cmpInst->ValueR.value);
        loopIR->loopCondVar = new LoopVariable();
        if (cmpValueL && (!cmpValueR || !cmpValueR->GetRelatedValue(loopIR->body))) {
            // left operator is loopVar and right operator will not change in loop body
            auto iter = loopIR->loopVarsByDefine.find(cmpValueL);
            if (iter == loopIR->loopVarsByDefine.cend())
                throw std::runtime_error("Cannot find loopCondVar from cmpValueL.");
            loopIR->loopCondVar = iter->second;
            return tryInferLoopVarDelta(loopIR->loopCondVar, branchInst->true_block, loopDelta);
        } else if (cmpValueR && (!cmpValueL || !cmpValueL->GetRelatedValue(loopIR->body))) {
            auto iter = loopIR->loopVarsByDefine.find(cmpValueR);
            if (iter == loopIR->loopVarsByDefine.cend())
                throw std::runtime_error("Cannot find loopCondVar from cmpValueR.");
            loopIR->loopCondVar = iter->second;
            return tryInferLoopVarDelta(loopIR->loopCondVar, branchInst->true_block, loopDelta);
        } else return false;
    }

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
                auto *dup = new ir::CallInst(x->fname, x->is_void);
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
            if (originLoopIR->loopVarsByEnter.find(inst) != originLoopIR->loopVarsByEnter.cend()) {
                if (loopIR == originLoopIR) {
                    for (auto *loopVar : loopIR->loopVarsByEnter[inst]) {
                        // replace
                        loopVar->loopVarDefine->InsertElem(loopIR->branchInst->true_block, loopIR->body->iList.back());
                    }
                } else {
                    for(auto *originLoopVar : originLoopIR->loopVarsByEnter.find(inst)->second) {
                        auto *loopVar = new LoopVariable();
                        loopVar->loop = loopIR->loop;
                        loopVar->loopVarInit = originLoopVar->loopVarDefine;
                        loopVar->loopVarDefine = loopIR->getCorrespondingPhiInst(originLoopIR,
                                                                                 originLoopVar->loopVarDefine);
                        loopVar->loopVarBody = loopIR->body->iList.back();
                        loopIR->loopVarsByDefine[loopVar->loopVarDefine] = loopVar;
                        loopIR->loopVarsByEnter[loopIR->body->iList.back()].push_back(loopVar);
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
        for (auto &pair : resetLoopIR->loopVarsByEnter) {
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
        // TODO Remove loop where loopCount = 0
        if (constLoopCondAnalysis(&loopIR, loopCount, loopDelta) && loopCount > 0) {
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
            std::cout << "unrolling constLoopCond #1" << std::endl;

            if (loopResetCount > 0) {
                LoopIR resetLoopIR;
                insertResetLoop(&resetLoopIR, &loopIR, iList);
                fixUnrollingLoopCondition(&loopIR, loopDelta, loopResetCount);
                std::cout << "unrolling constLoopCond #2" << std::endl;
            }
        } else if (flexibleLoopAnalysis(&loopIR, loopCount, loopDelta)) {
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