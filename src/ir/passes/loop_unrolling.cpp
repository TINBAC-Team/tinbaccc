#include <ir/passes.h>
#include <algorithm>

const int K = 2;

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
        this->loopVarInit = loopVarDefine->GetRelatedValue(loop->head);
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

    std::vector<ir::PhiInst *> phiInst;
    std::unordered_map<ir::Value *, LoopVariable *> loopVarsByDefine;
    std::unordered_map<ir::Value *, LoopVariable *> loopVarsByEnter;

     ir::PhiInst* getCorrespondingPhiInst(const LoopIR *loopIR, const ir::PhiInst *phiInst) {
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
                loopVarsByEnter[x->GetRelatedValue(branchInst->true_block)] = loopVar;
            }
        }
    }

    void init(ir::Loop *loop) {
        this->loop = loop;
        this->cond = *loop->body.cbegin();
        this->body = *loop->body.rbegin();
        if (*loop->head->succ().cbegin() != cond) std::swap(this->cond, this->body);

        for (auto iter = cond->iList.begin(); iter != cond->iList.end(); iter++) {
            if ((cmpInst = dynamic_cast<ir::BinaryInst *>(*iter)) && cmpInst->is_icmp()) {
                // Assume that BranchInst immediately follows CMPInst
                branchInst = dynamic_cast<ir::BranchInst *>(*(++iter));
                break;
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
        return true;
    }

    // loopVar OP cond
    bool tryInferLoopCount(ir::OpType opType, int init, int cond, int delta, int &cnt) {
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
                if (init > cond) cnt = (init - cond - 1) / delta + 1;
                else cnt = 0;
                break;
            case ir::OpType::SGE:
                if (init >= cond) cnt = (init - cond) / delta + 1;
                else cnt = 0;
                break;
            default:
                return false;
        }
        return true;
    }

    bool constLoopCondAnalysis(ir::Loop *loop, ir::BinaryInst *cmpInst, ir::BranchInst *branchInst, int &loopCount,
                              int &loopDelta, LoopVariable *loopVar) {
        auto *cmpValueL = dynamic_cast<ir::ConstValue *>(cmpInst->ValueL.value);
        auto *cmpValueR = dynamic_cast<ir::ConstValue *>(cmpInst->ValueR.value);
        loopDelta = 0;
        if (cmpValueR && !cmpValueL) {
            // left operator is loopVar and right operator is const
            loopVar->init(loop, branchInst, dynamic_cast<ir::PhiInst *>(cmpInst->ValueL.value));
            auto *loopVarInit = dynamic_cast<ir::ConstValue *>(loopVar->loopVarInit);
            if (!loopVarInit) return false;
            int loopInit = loopVarInit->value;
            int condConst = cmpValueR->value;
            if (tryInferLoopVarDelta(loopVar, branchInst->true_block, loopDelta)
                && tryInferLoopCount(cmpInst->optype, loopInit, condConst, loopDelta, loopCount)) {
                return true;
            }
        } else if (cmpValueL && !cmpValueR) {
            // right operator is loopVar and left operator is const
            loopVar->init(loop, branchInst, dynamic_cast<ir::PhiInst *>(cmpInst->ValueR.value));
            auto *loopVarInit = dynamic_cast<ir::ConstValue *>(loopVar->loopVarInit);
            if (!loopVarInit) return false;
            int loopInit = loopVarInit->value;
            int condConst = cmpValueL->value;
            ir::OpType opType;
            // flip operator
            switch (cmpInst->optype) {
                case ir::OpType::SLT:
                    opType = ir::OpType::SGT;
                    break;
                case ir::OpType::SLE:
                    opType = ir::OpType::SGE;
                    break;
                case ir::OpType::SGT:
                    opType = ir::OpType::SLT;
                    break;
                case ir::OpType::SGE:
                    opType = ir::OpType::SLE;
                    break;
                default:
                    opType = cmpInst->optype;
                    break;
            }
            if (tryInferLoopVarDelta(loopVar, branchInst->true_block, loopDelta)
                && tryInferLoopCount(opType, loopInit, condConst, loopDelta, loopCount)) {
                return true;
            }
        }
        return false;
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
            else if (u.value->bb != body)
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
                    LoopVariable *loopVar = loopIR->loopVarsByEnter[inst];
                    // replace
                    loopVar->loopVarDefine->InsertElem(loopIR->branchInst->true_block, loopIR->body->iList.back());
                } else {
                    LoopVariable *loopVar = new LoopVariable();
                    loopVar->loop = loopIR->loop;
                    auto * originLoopVar = originLoopIR->loopVarsByEnter.find(inst)->second;
                    loopVar->loopVarInit  = originLoopVar->loopVarDefine;
                    loopVar->loopVarDefine = loopIR->getCorrespondingPhiInst(originLoopIR, originLoopVar->loopVarDefine);
                    loopVar->loopVarBody  =  loopIR->body->iList.back();
                    loopIR->loopVarsByDefine[loopVar->loopVarDefine] = loopVar;
                    loopIR->loopVarsByEnter[loopIR->body->iList.back()] = loopVar;
                }
            }

        }

    }

    void insertResetLoop(LoopVariable *originLoopVar, LoopIR *originLoopIR, std::vector<ir::Value *> &originInstList,
                         LoopVariable *unrollingLoopVar) {
        // Step1 prepare Loop struct
        auto *reset_loop = new ir::Loop();
        reset_loop->head = originLoopIR->body;
        reset_loop->depth = originLoopIR->loop->depth;

        // Step2 prepare LoopIR struct
        LoopIR reset_loopIR;
        reset_loopIR.loop = reset_loop;
        reset_loopIR.cond = new ir::BasicBlock("while_unrolling.entry");
        reset_loopIR.body = new ir::BasicBlock("while_unrolling.true");

        // Step3 prepare phi instructions
        for (auto &originPhiInst : originLoopIR->phiInst) {
            auto *phiInst = new ir::PhiInst();
            phiInst->InsertElem(originLoopIR->cond, originPhiInst);
            reset_loopIR.phiInst.push_back(phiInst);
            reset_loopIR.cond->InsertAtEnd(phiInst);
        }

        // Step4 insert the basic blocks
        auto iter = std::find(function->bList.begin(), function->bList.end(), originLoopIR->body);
        iter++;
        iter = function->bList.insert(iter, reset_loopIR.cond);
        iter++;
        function->bList.insert(iter, reset_loopIR.body);

        // Step5 generate IR for reset_loop body
        duplicateLoopBody(&reset_loopIR, reset_loopIR.body, originLoopIR, originInstList);
        reset_loopIR.body->InsertAtEnd(new ir::JumpInst(reset_loopIR.cond));



        // Step6 generate IR for reset_loop cond
        LoopVariable *reset_loopVar =
                reset_loopIR.loopVarsByDefine[reset_loopIR.getCorrespondingPhiInst(originLoopIR, originLoopVar->loopVarDefine)];
        auto * valueL = originLoopIR->cmpInst->ValueL.value;
        auto * valueR = originLoopIR->cmpInst->ValueR.value;
        if (valueL == originLoopVar->loopVarDefine) {
            valueL = reset_loopVar->loopVarDefine;
        } else if (valueR == originLoopVar->loopVarDefine) {
            valueR = reset_loopVar->loopVarDefine;
        } else throw std::runtime_error("One of cmpInst\'s must be originLoopVar.loopVarDefine.");
        reset_loopIR.cmpInst = new ir::BinaryInst(originLoopIR->cmpInst->optype, valueL, valueR);
        reset_loopIR.branchInst = new ir::BranchInst(reset_loopIR.cmpInst, reset_loopIR.body,
                                                     originLoopIR->branchInst->false_block);
        reset_loopIR.cond->InsertAtEnd(reset_loopIR.cmpInst);
        reset_loopIR.cond->InsertAtEnd(reset_loopIR.branchInst);

        // Step6 fill phi instruction
        for (auto & pair : reset_loopIR.loopVarsByEnter) {
            auto * loopVar = pair.second;
            auto * enterVar = pair.first;
            loopVar->loopVarDefine->InsertElem(reset_loopIR.branchInst->true_block, enterVar);
        }

        // Step7 modify origin loop jumpInst
        originLoopIR->cond->replaceSucc(originLoopIR->branchInst->false_block, reset_loopIR.cond);

        // Step8 apply changes to loops
        function->deepestLoop.push_back(reset_loop);
        function->loops.insert(reset_loop);
        if (originLoopIR->loop->external)
            originLoopIR->loop->external->nested.push_back(reset_loop);
        reset_loop->body.insert(reset_loopIR.cond);
        reset_loop->body.insert(reset_loopIR.body);

        // Step9 replace value
        std::set<ir::BasicBlock*> ignore;
        std::set_union(originLoopIR->loop->body.cbegin(),  originLoopIR->loop->body.cend(),
                       reset_loop->body.cbegin(),  reset_loop->body.cend(), std::inserter(ignore, ignore.cbegin()));

        auto resetIter = reset_loopIR.phiInst.begin();
        for (auto * phiInst : originLoopIR->phiInst) {
            std::vector<ir::Use*> usesCopy;
            usesCopy.assign(phiInst->uList.cbegin(), phiInst->uList.cend());
            for (auto & use : usesCopy) {
                if (ignore.find(use->user->bb) != ignore.cend()) continue;
                std::cout << "rep" << std::endl;
                use->use(*resetIter, true);
            }
            resetIter++;
        }
    }

    void unrolling(ir::Loop *loop) {
        // Step1 only consider one basic block
        if (loop->body.size() != 2) return;

        // Step2 Collect necessary information about loop cond
        LoopVariable loopVar{};
        LoopIR loopIR{};
        loopIR.init(loop);
        loopIR.build();
        int cnt;

        // Step3 Process loop which execution times can be inferred
        // TODO Remove loop where cnt = 0
        int loopDelta;
        if (constLoopCondAnalysis(loop, loopIR.cmpInst, loopIR.branchInst, cnt, loopDelta, &loopVar) && cnt > 0) {
            int loopUnrollingCount = cnt / K;
            int loopResetCount = cnt - loopUnrollingCount * K;
            if (loopUnrollingCount == 0) return; // do nothing
            auto *jumpToCond = loopIR.body->iList.back();
            loopIR.body->iList.pop_back();
            std::vector<ir::Value *> iList{loopIR.body->iList.size()};
            iList.assign(loopIR.body->iList.cbegin(), loopIR.body->iList.cend());

            // reserve last loopVar
            LoopVariable unrollingLoopVar{};
            for (int i = 0; i < K - 1; i++) {
                // loopVarDefine must be filled
                unrollingLoopVar.loopVarDefine = loopVar.loopVarDefine;
                duplicateLoopBody(&loopIR, loopIR.body, &loopIR, iList);
            }
            std::cout << "unrolling #1" << std::endl;
            loopIR.body->InsertAtEnd(jumpToCond);
            if (loopResetCount > 0) {
                insertResetLoop(&loopVar, &loopIR, iList, &unrollingLoopVar);
                std::cout << "unrolling #2" << std::endl;
            }

            if (loopDelta > 0) {
                loopDelta = -loopResetCount;
            } else {
                loopDelta = loopResetCount;
            }
            if (auto x = dynamic_cast<ir::ConstValue*>(loopIR.cmpInst->ValueL.value)) {
                loopIR.cmpInst->ValueL = {loopIR.cmpInst, new ir::ConstValue(x->value + loopDelta)};
            } else if (auto x = dynamic_cast<ir::ConstValue*>(loopIR.cmpInst->ValueR.value)) {
                loopIR.cmpInst->ValueR = {loopIR.cmpInst, new ir::ConstValue(x->value + loopDelta)};
            }
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