#include "ir/ir.h"

BasicBlock *IRBuilder::CreateBlock() {
    auto* BasicBlockp = new BasicBlock(*this);
    BBCont.push_back(BasicBlockp);
    bList.push_back(BasicBlockp);
    return bList.back();
}

BasicBlock *IRBuilder::GetCurBlock() const {
    return CurBlock;
}

IRBuilder::IRBuilder() {
    CurBlock = nullptr;
}



Value::Value() {

}

int Value::addUse(Use * use) {
    uList.insert(use);
    return 0;
}

Use::Use(Inst *_user, Value *_value) {
    user = _user;
    value = _value;
    user->addUse(this);
}



Inst::Inst(OpType _optype) {
    instop = _optype;
}

BinaryInst::BinaryInst(OpType _optype, Value *ValueL, Value *ValueR) : Inst(_optype) {

}

Value *BasicBlock::CreateBinaryInst(OpType _instop, Value *ValueL, Value *ValueR) {
    auto *instp = new BinaryInst(_instop,ValueL,ValueR);
    ValCont->push_back(instp);
    iList.push_back(instp);
    return instp;
}

BasicBlock::BasicBlock(IRBuilder &irbuilder) {
    ValCont = &irbuilder.ValCont;
}
