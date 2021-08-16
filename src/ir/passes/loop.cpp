#include <ir/loop.h>
#include <ir/ir.h>
#include <algorithm>

void ir::LoopVariable::init(ir::Loop *loop, ir::BranchInst *branchInst, ir::PhiInst *defineInst) {
    this->loop = loop;
    this->loopVarDefine = defineInst;
    this->loopVarInit = loopVarDefine->GetRelatedValue(loop->prehead);
    this->loopVarBody = loopVarDefine->GetRelatedValue(branchInst->true_block);
}

ir::PhiInst *ir::LoopIR::getCorrespondingPhiInst(const ir::LoopIR *loopIR, const ir::PhiInst *phiInst) {
    auto iter = std::find(loopIR->phiInst.begin(), loopIR->phiInst.end(), phiInst);
    if (iter == loopIR->phiInst.cend()) return nullptr;
    int index = iter - loopIR->phiInst.cbegin();
    return this->phiInst[index];
}

void ir::LoopIR::build() {
    for (auto *inst : cond->iList) {
        if (auto *x = dynamic_cast<ir::PhiInst *>(inst)) {
            phiInst.push_back(x);
            auto *loopVar = new LoopVariable();
            loopVar->init(loop, branchInst, x);
            loopVarsByDefine[x] = loopVar;
            loopVarsByBody[x->GetRelatedValue(branchInst->true_block)].push_back(loopVar);
        }
    }
}

void ir::LoopIR::init(ir::Loop *loop) {
    this->loop = loop;
    this->cond = *loop->body.cbegin();
    this->body = *loop->body.rbegin();
    if (*loop->prehead->succ().cbegin() != cond) std::swap(this->cond, this->body);

    for (auto &iter : cond->iList) {
        if (auto *cmpInst = dynamic_cast<ir::BinaryInst *>(iter)) {
            if (cmpInst->is_icmp()) this->cmpInst = cmpInst;
        } else if (auto *branchInst = dynamic_cast<ir::BranchInst *>(iter)) {
            this->branchInst = branchInst;
        }
    }
}

ir::LoopIR::~LoopIR() {
    for (auto pair : loopVarsByDefine) delete pair.second;
    loopVarsByDefine.clear();
    loopVarsByBody.clear();
}



bool ir::tryInferLoopVarDelta(ir::LoopVariable *loopVar, ir::BasicBlock *scope, int &delta) {
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

bool ir::tryInferLoopCount(ir::OpType opType, int init, int cond, long delta, int &cnt) {
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

bool ir::constLoopCondAnalysis(ir::LoopIR *loopIR, int &loopCount, int &loopDelta) {
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

bool ir::flexibleLoopAnalysis(ir::LoopIR *loopIR, int &loopCount, int &loopDelta) {
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
