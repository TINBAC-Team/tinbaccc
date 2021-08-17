#include <ir/ir.h>
#include <ir/loop.h>
#include <ir/passes.h>
#include <algorithm>

using ir::LoopIR;
using ir::LoopVariable;

void replaceValue(ir::BasicBlock *scope, ir::Value *value, int delta) {
    if (auto * x = dynamic_cast<ir::ConstValue*>(value)) {
        x->value += delta;
    } else if (delta != 0) {
        auto * binaryInst = new ir::BinaryInst(ir::OpType::ADD, value, new ir::ConstValue(delta));
        scope->InsertAfter(binaryInst, value);
        value->replaceWithDependence(binaryInst, true);
    }
}
void computeCondVar(LoopIR *loopIR, bool loopDeltaAvailable, int loopDelta, bool loopCountAvailable, int loopCount) {
    // loopVar might be inferred
    if (loopDeltaAvailable && loopIR->loopCondVar->loopVarBody->uList.size() == 1) {
        auto *cmpL = loopIR->cmpInst->ValueL.value;
        auto *cmpR = loopIR->cmpInst->ValueR.value;
        auto cmpOP = loopIR->cmpInst->optype;
        // standard form
        if (cmpR == loopIR->loopCondVar->loopVarDefine) {
            std::swap(cmpL, cmpR);
            cmpOP = flipOperator(cmpOP);
        }
        if (cmpL == loopIR->loopCondVar->loopVarDefine) {
            if ((loopDelta == 1 && cmpOP == ir::OpType::SLT)
                || (loopDelta == -1 && cmpOP == ir::OpType::SGT)) {
                loopIR->loopCondVar->loopVarDefine->replaceWithDependence(cmpR, true);
            } else if (loopDelta == 1 && cmpOP == ir::OpType::SLE) {
                auto * binaryInst = new ir::BinaryInst(ir::OpType::ADD, cmpR, new ir::ConstValue(1));
                loopIR->cond->InsertAfter(binaryInst, loopIR->loopCondVar->loopVarDefine);
                loopIR->loopCondVar->loopVarDefine->replaceWithDependence(binaryInst, true);
            } else if (loopDelta == -1 && cmpOP == ir::OpType::SGE) {
                auto * binaryInst = new ir::BinaryInst(ir::OpType::SUB, cmpR, new ir::ConstValue(1));
                loopIR->cond->InsertAfter(binaryInst, loopIR->loopCondVar->loopVarDefine);
                loopIR->loopCondVar->loopVarDefine->replaceWithDependence(binaryInst, true);
            }
        }
    }

}

void computeLoopVar(LoopIR *loopIR, bool loopDeltaAvailable, int loopDelta, bool loopCountAvailable, int loopCount) {
    for (auto &pair : loopIR->loopVarsByDefine) {
        auto *loopVar = pair.second;
        if (loopVar == loopIR->loopCondVar) continue;
        tryInferLoopVarDelta(loopVar, loopIR->body, loopVar->loopDelta);
        if (loopVar->loopDelta) {
            if (loopVar->loopDelta == 0) {
                // not changed
                loopVar->loopVarDefine->replaceWith(loopVar->loopVarInit);
            } else if (loopCountAvailable) {
                if (loopVar->deltaPresent && loopVar->loopVarDefine->uList.size() == 1) {
                    auto *add = new ir::BinaryInst(
                            ir::OpType::ADD,
                            loopVar->loopVarInit,
                            new ir::ConstValue(loopVar->loopDelta * loopCount)
                    );
                    loopVar->loopVarDefine->replaceWith(add);
                    loopIR->body->InsertBefore(loopVar->loopVarDefine, add);
                } else {
                    if (auto *x = dynamic_cast<ir::BinaryInst *>(loopVar->loopVarBody)) {
                        if (loopVar->loopVarDefine->uList.size() == 1) {
                            if (x->optype == ir::OpType::ADD) {
                                if (loopVar->loopVarBody == x->ValueL.value &&
                                    loopIR->loopVarsByBody.find(x->ValueR.value) != loopIR->loopVarsByBody.cend()) {
                                    auto &plusBy = loopIR->loopVarsByBody[x->ValueR.value];
                                    if (plusBy.size() != 1) continue;
                                    if (auto *plusByInit = dynamic_cast<ir::ConstValue *>(plusBy[0]->loopVarInit)) {

                                        //loopVar->loopVarDefine->replaceWith()
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void ir_passes::loop_val_compute(ir::Module *module) {
    for(auto * func : module->functionList) {
        if (func->is_extern() || func->bList.empty()) continue;
        for (auto * deepestLoop : func->deepestLoop) {
            ir::LoopIR loopIR;
            loopIR.init(deepestLoop);
            loopIR.build();
            int delta, count;
            bool deltaAvailable, countAvailable;
            if (ir::constLoopCondAnalysis(&loopIR, count, delta)) {
                deltaAvailable = true;
                countAvailable = true;
            } else if (ir::flexibleLoopAnalysis(&loopIR, delta)) {
                deltaAvailable = true;
                countAvailable = false;
            } else continue;
            computeCondVar(&loopIR, deltaAvailable, delta, countAvailable, count);
        }
    }
}