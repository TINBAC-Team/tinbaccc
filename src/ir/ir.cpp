#include <ir/ir.h>
#include <ast/ast.h>

ConstPool ConstValue::const_pool;

BasicBlock *IRBuilder::CreateBlock() {
    auto* BasicBlockp = new BasicBlock();
    bList.push_back(BasicBlockp);
    CurBlock = BasicBlockp;
    return bList.back();
}

BasicBlock *IRBuilder::GetCurBlock() const {
    return CurBlock;
}

IRBuilder::IRBuilder() {
    CurBlock = nullptr;
}

Value *IRBuilder::CreateBinaryInst(Value* _ValueL, Value* _ValueR, OpType optype) {
    auto *constL = dynamic_cast<ConstValue*>(_ValueL);
    auto *constR = dynamic_cast<ConstValue*>(_ValueR);
    if(constL && constR)
    {
        return getConstant(constL->value, constR->value, optype);
    }
    auto* instp = new BinaryInst(optype, _ValueL, _ValueR);
    auto* curblock = GetCurBlock();
    curblock->InsertAtEnd(instp);
    return instp;
}


Value::Value(OpType _optype) {
    optype = _optype;
}



int Value::addUse(Use * use) {
    uList.insert(use);
    return 0;
}

Value::~Value() {

}

Use::Use(Inst *_user, Value *_value) {
    user = _user;
    value = _value;
    user->addUse(this);
}



Inst::Inst(OpType _optype) : Value(_optype) {
}

Inst::~Inst() {

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

ConstValue::~ConstValue() {

}

Value *IRBuilder::getConstant(int _value) {
    if(ConstValue::const_pool.find(_value) != ConstValue::const_pool.end()) return ConstValue::const_pool[_value];
    auto constp = new ConstValue(_value);
    return constp;
}

Value *IRBuilder::CreateLoadInst() {
    return nullptr;
}

Value *IRBuilder::CreateCallInst() {
    return nullptr;
}

Value *IRBuilder::getConstant(int valueL, int valueR, OpType optype) {
    switch(optype){
        case OpType::ADD:
            return getConstant(valueL+valueR);
        case OpType::SUB:
            return getConstant(valueL-valueR);
        case OpType::MUL:
            return getConstant(valueL*valueR);
        case OpType::SDIV:
            return getConstant(valueL/valueR);
        case OpType::SREM:
            return getConstant(valueL%valueR);
        case OpType::SLT:
            return getConstant(valueL<valueR);
        case OpType::SLE:
            return getConstant(valueL<=valueR);
        case OpType::SGT:
            return getConstant(valueL>valueR);
        case OpType::SGE:
            return getConstant(valueL>=valueR);
        case OpType::EQ:
            return getConstant(valueL==valueR);
        case OpType::NE:
            return getConstant(valueL!=valueR);
        default:
            throw std::runtime_error("Failed to get constant. OpType invalid.");
    }
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

CallInst::CallInst(ast::Function *_function) : Inst(OpType::CALL) {
    function = _function;
}
