#include <ir/ir.h>
#include <ir/loop.h>
#include <ir/passes.h>
#include <algorithm>

using ir::LoopIR;
using ir::LoopVariable;

class LoopVal {
    ir::Module *module;

public:
    LoopVal(ir::Module *module) : module(module) {}

    void replaceValue(ir::LoopIR *loopIR, ir::LoopVariable *loopVar, ir::Value *base, int delta) {
        ir::Value *replace;
        if (delta == 0) {
            replace = base;
        } else {
            if (auto x = dynamic_cast<ir::ConstValue*>(base)) {
                replace = ir::IRBuilder::getConstant(x->value + delta, module);
            } else {
                replace = new ir::BinaryInst(ir::OpType::ADD, base, ir::IRBuilder::getConstant(delta, module));
                loopIR->cond->InsertBefore(replace, loopVar->loopVarDefine);
            }

        }
        std::set<ir::Use*> uList;
        for (auto * curr_use : loopVar->loopVarDefine->uList) {
            if (loopIR->cond == curr_use->user->bb || loopIR->body == curr_use->user->bb) {

                uList.insert(curr_use);
            } else {
                curr_use->use(replace, false);
            }
        }
        loopVar->loopVarDefine->uList = uList;
    }
    void computeCondVar(LoopIR *loopIR, bool loopDeltaAvailable, int loopDelta, bool loopCountAvailable, int loopCount) {
        // loopVar might be inferred
        if (!loopDeltaAvailable) return;
        auto *cmpL = loopIR->cmpInst->ValueL.value;
        auto *cmpR = loopIR->cmpInst->ValueR.value;
        auto cmpOP = loopIR->cmpInst->optype;
        // standard form
        if (cmpR == loopIR->loopCondVar->loopVarDefine) {
            std::swap(cmpL, cmpR);
            cmpOP = flipOperator(cmpOP);
        }
        if (cmpL == loopIR->loopCondVar->loopVarDefine) {
            if (loopCountAvailable &&
            (loopDelta == 1 && cmpOP == ir::OpType::SLT)
            || (loopDelta == -1 && cmpOP == ir::OpType::SGT)) {
                replaceValue(loopIR, loopIR->loopCondVar, cmpR, 0);
            } else if (loopCountAvailable && loopDelta == 1 && cmpOP == ir::OpType::SLE) {
                replaceValue(loopIR, loopIR->loopCondVar, cmpR, 1);
            } else if (loopCountAvailable && loopDelta == -1 &&  cmpOP == ir::OpType::SGE) {
                replaceValue(loopIR, loopIR->loopCondVar, cmpR, -1);
            } else if (cmpOP == ir::OpType::NE) {
                replaceValue(loopIR, loopIR->loopCondVar, cmpR, 0);
            } else return;
            std::cout << "Replace condVar with condVal." << std::endl;
        }

    }

    void computeLoopVar(LoopIR *loopIR, bool loopDeltaAvailable, int loopDelta, bool loopCountAvailable, int loopCount) {
        if (!loopCountAvailable || !loopDeltaAvailable) return;
        for (auto &pair : loopIR->loopVarsByDefine) {
            auto *loopVar = pair.second;
            tryInferLoopVarDelta(loopVar, loopIR->body, loopVar->loopDelta);
            if (loopVar->deltaPresent) {
                replaceValue(loopIR, loopVar, loopVar->loopVarInit, loopVar->loopDelta * loopCount);
            } else {
                // while (i < n) {s += i;} put(s);
                int d, delta;
                if (auto * x = dynamic_cast<ir::BinaryInst*>(loopVar->loopVarBody)) {
                    if (x->ValueL.value == loopIR->loopCondVar->loopVarDefine &&
                    x->ValueR.value == loopVar->loopVarDefine) {
                        // s = i + s
                        if (x->optype == ir::OpType::ADD) {
                            d = loopDelta;
                        } else continue;
                    } else if (x->ValueL.value == loopVar->loopVarDefine &&
                    x->ValueR.value == loopIR->loopCondVar->loopVarDefine) {
                        // s = s + i
                        if (x->optype == ir::OpType::ADD) {
                            d = loopDelta;
                        } else if (x->optype == ir::OpType::SUB) {
                            d = -loopDelta;
                        } else continue;
                    } else continue;
                } else continue;
                // s = nx + n(n-1)*d/2
                delta = (dynamic_cast<ir::ConstValue*>(loopIR->loopCondVar->loopVarInit)->value) * loopCount
                        + loopCount * (loopCount - 1) / 2 * d;
                replaceValue(loopIR, loopVar, loopVar->loopVarInit, delta);
            }
            std::cout << "Inferred loop delta." << std::endl;
        }
    }
};

void ir_passes::loop_val_compute(ir::Module *module) {
    LoopVal loopVal{module};
    for(auto * func : module->functionList) {
        if (func->is_extern() || func->bList.empty()) continue;
        for (auto * deepestLoop : func->deepestLoop) {
            if (deepestLoop->body.size() != 2) continue;
            ir::LoopIR loopIR;
            loopIR.init(deepestLoop);
            loopIR.build();
            int delta, count;
            bool deltaAvailable, countAvailable;
            if (ir::constLoopCondAnalysis(&loopIR, count, delta)) {
                if (count == 0) continue;
                deltaAvailable = true;
                countAvailable = true;
            } else if (ir::flexibleLoopAnalysis(&loopIR, delta)) {
                deltaAvailable = true;
                countAvailable = false;
            } else continue;
            loopVal.computeLoopVar(&loopIR, deltaAvailable, delta, countAvailable, count);
            if (!countAvailable) {
                loopVal.computeCondVar(&loopIR, deltaAvailable, delta, countAvailable, count);
            }

        }
    }
    int debug = 1;
}