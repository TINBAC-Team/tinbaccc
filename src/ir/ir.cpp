#include <ir/ir.h>
#include <ast/ast.h>

namespace ir {
    ConstPool ConstValue::const_pool;

    BasicBlock *IRBuilder::CreateBlock() {
        CurBlock = curFunction->CreateBlock();
        return CurBlock;
    }

    Function *IRBuilder::CreateFunction() {
        auto *pFunc = new Function();
        functionList.push_back(pFunc);
        curFunction = pFunc;
        return pFunc;
    }

    BasicBlock *IRBuilder::GetCurBlock() const {
        return CurBlock;
    }

    IRBuilder::IRBuilder() {
        CurBlock = nullptr;
    }

    Value *IRBuilder::CreateBinaryInst(Value *_ValueL, Value *_ValueR, OpType optype) {
        auto *constL = dynamic_cast<ConstValue *>(_ValueL);
        auto *constR = dynamic_cast<ConstValue *>(_ValueR);
        if (constL && constR) {
            return getConstant(constL->value, constR->value, optype);
        }
        auto *instp = new BinaryInst(optype, _ValueL, _ValueR);
        auto *curblock = GetCurBlock();
        curblock->InsertAtEnd(instp);
        return instp;
    }

    Value *IRBuilder::CreateGlobalVar(ast::Decl *decl) {
        GlobalVar *var = new GlobalVar(decl);
        globalVarList.push_back(var);
        return var;
    }

    BasicBlock *Function::CreateBlock() {
        auto *BasicBlockp = new BasicBlock();
        bList.push_back(BasicBlockp);
        return bList.back();
    }

    void Function::setupParams(const std::vector<ast::Decl *> decls) {
        for (auto i:decls)
            params.emplace_back(new FuncParam(i));
    }

    Value::Value(OpType _optype) : optype(_optype), bb(nullptr) {}

    int Value::addUse(Use *use) {
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
        value->bb = this;
        return 0;
    }

    int BasicBlock::InsertAtFront(Value *value) {
        iList.push_front(value);
        value->bb = this;
        return 0;
    }

    void BasicBlock::addParentInst(Inst *inst) {
        parentInsts.push_back(inst);
    }

    Value * BasicBlock::getVariable(ast::Decl *decl, IRBuilder &builder) {
        // Local Value Numbering: lookup variable's current definition and return it.
        ir::Value *val = decl->lookup_var_def(this);
        if (val)
            return val;

        // Global Value Numbering: Lookup its definitions in BB predecessors.
        // For BBs with a single predecessor, get Value from its parent block.
        if (parentInsts.size() == 1)
            return parentInsts.front()->bb->getVariable(decl, builder);
        // Otherwise we need to create a Phi for all its predecessors
        if(parentInsts.size() > 1) {
            // construct an operand-less Phi...
            val = builder.CreatePhi();
            // ...and record it as Decl's current definition.
            decl->set_var_def(this, val);
            // TODO: we need more infrastructure work...
        }
        // Variable used without assignment...
        return builder.getConstant(0);
    }

    ConstValue::ConstValue(int _value) : Value(OpType::CONST) {
        value = _value;
    }

    ConstValue::~ConstValue() {

    }

    Value *IRBuilder::getConstant(int _value) {
        if (ConstValue::const_pool.find(_value) != ConstValue::const_pool.end()) return ConstValue::const_pool[_value];
        auto constp = new ConstValue(_value);
        return constp;
    }

    Value *IRBuilder::CreateLoadInst(Value* ptr) {
        auto* instp = new LoadInst(ptr);
        auto *curblock = GetCurBlock();
        curblock->InsertAtEnd(instp);
        return instp;
    }

    Value *IRBuilder::CreateCallInst() {
        return nullptr;
    }

    Value *IRBuilder::getConstant(int valueL, int valueR, OpType optype) {
        switch (optype) {
            case OpType::ADD:
                return getConstant(valueL + valueR);
            case OpType::SUB:
                return getConstant(valueL - valueR);
            case OpType::MUL:
                return getConstant(valueL * valueR);
            case OpType::SDIV:
                return getConstant(valueL / valueR);
            case OpType::SREM:
                return getConstant(valueL % valueR);
            case OpType::SLT:
                return getConstant(valueL < valueR);
            case OpType::SLE:
                return getConstant(valueL <= valueR);
            case OpType::SGT:
                return getConstant(valueL > valueR);
            case OpType::SGE:
                return getConstant(valueL >= valueR);
            case OpType::EQ:
                return getConstant(valueL == valueR);
            case OpType::NE:
                return getConstant(valueL != valueR);
            default:
                throw std::runtime_error("Failed to get constant. OpType invalid.");
        }
    }

    Value *IRBuilder::CreateAllocaInst(int _size) {
        auto* instp = new AllocaInst(_size);
        auto *curblock = GetCurBlock();
        curblock->InsertAtEnd(instp);
        return instp;
    }

    PhiInst::PhiInst(const PhiParam &phiparam) : Inst(OpType::PHI) {
        for (auto &i:phiparam) {
            phicont.insert(i);
        }
    }

    Value *PhiInst::GetRelatedValue(BasicBlock *basicblock) {
        if (phicont.find(basicblock) == phicont.end()) return nullptr;
        return phicont[basicblock];
    }

    PhiInst::PhiInst() : Inst(OpType::PHI) {

    }

    int PhiInst::InsertElem(BasicBlock *basicblock, Value *value) {
        phicont[basicblock] = value;
        return 0;
    }

    PhiInst *IRBuilder::CreatePhi() {
        auto *instp = new PhiInst();
        auto *curblock = GetCurBlock();
        curblock->InsertAtFront(instp);
        return instp;
    }

    CallInst::CallInst(ast::Function *_function) : Inst(OpType::CALL) {
        function = _function;
    }

    AllocaInst::AllocaInst(int _size) :Inst(OpType::ALLOCA){
        size = _size;
    }

    GetElementPtrInst::GetElementPtrInst(Value *_arr, Value* _offset) : AccessInst(OpType::GETELEMPTR){
        arr = _arr;
        offset = _offset;
    }
    AccessInst::AccessInst(OpType _optype) : Inst(_optype) {

    }
    Value *IRBuilder::CreateGetElementPtrInst(Value* arr, Value* offset){
        auto instp = new GetElementPtrInst(arr,offset);
        auto *curblock = GetCurBlock();
        curblock->InsertAtEnd(instp);
        return instp;
    }

    LoadInst::LoadInst(Value *_ptr) : AccessInst(OpType::LOAD){
        ptr = _ptr;
    }
    StoreInst::StoreInst(Value *_ptr, Value *_val) :AccessInst(OpType::STORE) {
        ptr = _ptr;
        val = _val;
    }

    Value * IRBuilder::CreateStoreInst(Value *ptr, Value *val) {
        auto instp = new StoreInst(ptr,val);
        auto *curblock = GetCurBlock();
        curblock->InsertAtEnd(instp);
        return instp;
    }
}