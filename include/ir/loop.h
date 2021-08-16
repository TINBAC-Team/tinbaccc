#ifndef TINBACCC_LOOP_H
#define TINBACCC_LOOP_H

#include <ir/ir.h>

const int K = 4;
const int DELTA_LIMIT = 64;

namespace ir {

    /**
    * Represent a loop variable which is defined outside the loop body and use inside the loop body.
    */
    class LoopVariable {
    public:
        ir::Loop *loop = nullptr;
        ir::Value *loopVarInit = nullptr;
        ir::PhiInst *loopVarDefine = nullptr;
        ir::Value *loopVarBody = nullptr;
        int loopDelta = 0;
        bool deltaPresent = false;

        void init(ir::Loop *loop, ir::BranchInst *branchInst, ir::PhiInst *defineInst);

    };

    /**
     * Represent a loop with single basic block (IR form)
     */
    class LoopIR {
    public:
        ir::Loop *loop = nullptr;
        ir::BasicBlock *cond = nullptr;
        ir::BasicBlock *body = nullptr;
        ir::BinaryInst *cmpInst = nullptr;
        ir::BranchInst *branchInst = nullptr;

        LoopVariable *loopCondVar = nullptr;

        std::vector<ir::PhiInst *> phiInst;
        std::unordered_map<ir::Value *, LoopVariable *> loopVarsByDefine;
        // a loopVarEnter may correspond to multiple variables (it may be use in one more phiInst)
        std::unordered_map<ir::Value *, std::vector<LoopVariable *>> loopVarsByBody;

        ir::PhiInst *getCorrespondingPhiInst(const LoopIR *loopIR, const ir::PhiInst *phiInst);

        void build();

        void init(ir::Loop *loop);

        virtual ~LoopIR();
    };

    bool tryInferLoopVarDelta(ir::LoopVariable *loopVar, ir::BasicBlock *scope, int &delta);

    // loopVar OP cond
    bool tryInferLoopCount(ir::OpType opType, int init, int cond, long delta, int &cnt);

    bool constLoopCondAnalysis(LoopIR *loopIR, int &loopCount, int &loopDelta);

    bool flexibleLoopAnalysis(LoopIR *loopIR, int &loopCount, int &loopDelta);
}
#endif //TINBACCC_LOOP_H
