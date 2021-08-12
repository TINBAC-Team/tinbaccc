#include <ir/passes.h>
#include <algorithm>
#include <cmath>

const int K = 4;

struct LoopVariable {
    ir::Loop *loop;
    ir::Value *loopVarInit;
    ir::PhiInst *loopVarEnter;
    ir::Value *loopVarExit;

    void init(ir::Loop *loop, ir::BranchInst *branchInst, ir::PhiInst *defineInst) {
        this->loop = loop;
        this->loopVarEnter = defineInst;
        this->loopVarInit = loopVarEnter->GetRelatedValue(loop->head);
        this->loopVarExit = loopVarEnter->GetRelatedValue(branchInst->true_block);
    }

};

struct LoopIR {
    ir::Loop *loop;
    ir::BasicBlock *cond;
    ir::BasicBlock *body;
    ir::BinaryInst *cmpInst;
    ir::BranchInst *branchInst;

    std::unordered_map<ir::Value *, LoopVariable*> loopVarsByDefine;
    std::unordered_map<ir::Value *, LoopVariable*> loopVarsByEnter;

    void build() {
        for(auto * inst : cond->iList) {
            if (auto * x = dynamic_cast<ir::PhiInst*>(inst)) {
                auto * loopVar = new LoopVariable();
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
        auto *currInst = loopVar->loopVarExit;
        while (currInst != loopVar->loopVarEnter && currInst->bb == scope) {
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

    int constLoopCondAnalysis(ir::Loop *loop, ir::BinaryInst *cmpInst, ir::BranchInst *branchInst, int &loopCount,
                              LoopVariable *loopVar) {
        auto *cmpValueL = dynamic_cast<ir::ConstValue *>(cmpInst->ValueL.value);
        auto *cmpValueR = dynamic_cast<ir::ConstValue *>(cmpInst->ValueR.value);
        int loopDelta = 0;
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
#include <iostream>

    void duplicateLoopBody(LoopIR *loopIR, LoopVariable *newLoopVar, ir::BasicBlock *body, std::vector<ir::Value *> &originInstList,
                           LoopVariable *originLoopVar) {

        // Step1 prepare for value remapping
        std::unordered_map<ir::Value *, ir::Value *> value_map;
        auto get_val = [&](const ir::Use &u) -> ir::Value * {
            ir::Value *ret;
            if (loopIR->loopVarsByDefine.find(u.value) != loopIR->loopVarsByDefine.cend())
                return loopIR->loopVarsByDefine[u.value]->loopVarEnter->GetRelatedValue(loopIR->branchInst->true_block);
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
            // set newLoopVar.loopVarExit
            if (loopIR->loopVarsByEnter.find(inst) != loopIR->loopVarsByEnter.cend()) {
                LoopVariable* loopVar = loopIR->loopVarsByEnter[inst];
                loopVar->loopVarEnter->InsertElem(loopIR->branchInst->true_block, loopIR->body->iList.back());
                loopIR->body->iList.back()->print(std::cout);
                std::cout << std::endl;
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
        LoopIR reset_loopIR{};
        reset_loopIR.cond = new ir::BasicBlock("while_unrolling.entry");
        reset_loopIR.body = new ir::BasicBlock("while_unrolling.true");
        reset_loopIR.cmpInst = new ir::BinaryInst(originLoopIR->cmpInst->optype,
                                                  originLoopIR->cmpInst->ValueL.value,
                                                  originLoopIR->cmpInst->ValueR.value);
        reset_loopIR.branchInst = new ir::BranchInst(reset_loopIR.cmpInst, reset_loopIR.body,
                                                     originLoopIR->branchInst->false_block);

        // Step3 prepare LoopVariable struct, note that loopVarEnter must be assigned
        LoopVariable reset_loopVar{};
        reset_loopVar.loopVarEnter = new ir::PhiInst();

        // Step4 generate IR for reset_loop body
        duplicateLoopBody(&reset_loopIR, &reset_loopVar, reset_loopIR.body, originInstList, unrollingLoopVar);
        reset_loopIR.body->InsertAtEnd(new ir::JumpInst(reset_loopIR.cond));

        // Step5 generate IR for reset_loop cond
        reset_loopVar.loopVarEnter->InsertElem(reset_loop->head, unrollingLoopVar->loopVarEnter); // init value
        reset_loopVar.loopVarEnter->InsertElem(reset_loopIR.body, reset_loopVar.loopVarExit); // loop value
        if (reset_loopIR.cmpInst->ValueL.value == originLoopVar->loopVarEnter) {
            reset_loopIR.cmpInst->ValueL.value = reset_loopVar.loopVarEnter;
        } else if (reset_loopIR.cmpInst->ValueR.value == originLoopVar->loopVarEnter) {
            reset_loopIR.cmpInst->ValueR.value = reset_loopVar.loopVarEnter;
        } else throw std::runtime_error("One of cmpInst\'s must be originLoopVar.loopVarEnter.");
        reset_loopIR.cond->InsertAtEnd(reset_loopIR.cmpInst);
        reset_loopIR.cond->InsertAtEnd(reset_loopIR.branchInst);

        // Step6 apply changes to loops
        function->deepestLoop.push_back(reset_loop);
        function->loops.insert(reset_loop);
        if (originLoopIR->loop->external) originLoopIR->loop->external->nested.push_back(reset_loop);
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
        if (constLoopCondAnalysis(loop, loopIR.cmpInst, loopIR.branchInst, cnt, &loopVar) && cnt > 0) {
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
                // loopVarEnter must be filled
                unrollingLoopVar.loopVarEnter = loopVar.loopVarEnter;
                duplicateLoopBody(&loopIR, &unrollingLoopVar, loopIR.body, iList, &loopVar);
            }
            loopIR.body->InsertAtEnd(jumpToCond);
            if (loopResetCount > 0) {
                insertResetLoop(&loopVar, &loopIR, iList, &unrollingLoopVar);

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