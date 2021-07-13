#include "ir/ir.h"

ConstPool ConstValue::const_pool;

BasicBlock *IRBuilder::CreateBlock() {
    auto* BasicBlockp = new BasicBlock();
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



Value::Value(OpType _optype) {
    optype = _optype;
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



Inst::Inst(OpType _optype) : Value(_optype) {
}

BinaryInst::BinaryInst(OpType _optype, Value *_ValueL, Value *_ValueR) : Inst(_optype) {
    ValueL = _ValueL;
    ValueR = _ValueR;
}


BasicBlock::BasicBlock() {

}

int BasicBlock::InsertAtEnd(Value *value) {
    iList.push_back(value);
    return 0;
}

int BasicBlock::InsertAtFront(Value *value) {
    iList.push_front(value);
    return 0;
}


ConstValue::ConstValue(int _value) : Value(OpType::CONST) {
    value = _value;
}

ConstValue *ConstValue::getConstant(int _value) {
    if(const_pool.find(_value) != const_pool.end()) return const_pool[_value];
    auto constp = new ConstValue(_value);
    return constp;
}

PhiInst::PhiInst(const PhiParam& phiparam) : Inst(OpType::PHI){
    for(auto &i:phiparam)
    {
        phicont.insert(i);
    }
}

Value *PhiInst::GetRelatedValue(BasicBlock* basicblock) {
    if(phicont.find(basicblock)==phicont.end()) return nullptr;
    return phicont[basicblock];
}

PhiInst::PhiInst() : Inst(OpType::PHI) {

}

int PhiInst::InsertElem(BasicBlock *basicblock, Value *value) {
    phicont[basicblock] = value;
    return 0;
}

CallInst::CallInst(Function *_function) : Inst(OpType::CALL) {
    function = _function;
}
