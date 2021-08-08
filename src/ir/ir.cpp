#include <ir/ir.h>
#include <ast/ast.h>
#include <utility>
#include <iostream>


namespace ir {

    std::unordered_map<std::string,int> BasicBlock::namePool;

    GlobalVar::GlobalVar(ast::Decl *d) :
            Value(OpType::GLOBAL), decl(d), name(decl->name), is_const(decl->is_const) {
        if (decl->is_array())
            len = decl->array_multipliers[0];
        else
            len = 1;

        for (auto val:decl->initval_expanded)
            initval.emplace_back(val ? val->get_value() : 0);

        decl->addr = this;
    }

    BasicBlock *IRBuilder::CreateBlock(std::string name) {
        CurBlock = curFunction->CreateBlock(name);
        return CurBlock;
    }

    void IRBuilder::appendBlock(BasicBlock *block) {
        curFunction->appendBlock(block);
        CurBlock = block;
    }

    Function *IRBuilder::CreateFunction(std::string name, bool return_int) {
        auto *pFunc = new Function(name, return_int);
        module->functionList.push_back(pFunc);
        curFunction = pFunc;
        return pFunc;
    }

    BasicBlock *IRBuilder::GetCurBlock() const {
        return CurBlock;
    }

    IRBuilder::IRBuilder(Module *m) : module(m) {
        CurBlock = nullptr;
        TrueBlock = nullptr;
        FalseBlock = nullptr;
        ContBlock = nullptr;
        EntryBlock = nullptr;
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
        module->globalVarList.push_back(var);
        return var;
    }

    BasicBlock *Function::CreateBlock(std::string name="bb") {
        auto *BasicBlockp = new BasicBlock(name);
        appendBlock(BasicBlockp);
        return BasicBlockp;
    }

    void Function::appendBlock(BasicBlock *block) {
        bList.push_back(block);
    }

    void Function::setupParams(const std::vector<ast::Decl *> decls) {
        for (auto i:decls)
            params.emplace_back(new FuncParam(i));
    }

    void Function::addParamsToBB(BasicBlock *block) {
        for (auto i:params) {
            if (i->decl->is_array())
                i->decl->addr = i;
            else
                i->decl->set_var_def(block, i);
        }
    }

    Value::Value(OpType _optype) : optype(_optype), bb(nullptr) {}

    int Value::addUse(Use *use) {
        uList.insert(use);
        return 0;
    }

    void Value::replaceWith(Value *val, bool clear_ulist) {
        for (auto cur_use:uList) {
            if (cur_use->user == this || cur_use->value != this)
                continue;
            cur_use->use(val, false);
        }
        if (clear_ulist)
            uList.clear();
    }

    bool Value::sideEffect() const {
        switch (optype) {
            case ir::OpType::RETURN:
            case ir::OpType::BRANCH:
            case ir::OpType::JUMP:
            case ir::OpType::STORE:
                return true;
            case ir::OpType::CALL:
                // TODO: determine its side-effect from callee
                return true;
            default:
                return false;
        }
    }

    Value::~Value() {
        for (auto &u:uList)
            if (u->value == this)
                u->value = nullptr;
    }

    Use::Use(Value *_user, Value *_value) : value(nullptr) {
        user = _user;
        if (_value)
            use(_value);
    }

    void Use::use(Value *v, bool remove_from_user) {
        if (remove_from_user)
            removeFromUList();
        value = v;
        v->addUse(this);
    }

    void Use::removeFromUList() {
        if (value)
            value->uList.erase(this);
    }

    Inst::Inst(OpType _optype) : Value(_optype) {
    }

    Inst::~Inst() {

    }

    BinaryInst::BinaryInst(OpType _optype, Value *_ValueL, Value *_ValueR) :
            Inst(_optype), ValueL(this, _ValueL), ValueR(this, _ValueR) {}

    BasicBlock::BasicBlock(std::string _name) : sealed(true), idom(nullptr), dom_tree_depth(-1) {
        if (namePool.find(_name) == namePool.end()) namePool[_name] = 0;
        else namePool[_name]++;
        name =  "_" + _name + (namePool[_name] ? std::to_string(namePool[_name]) : "");
    }

    BranchInst::~BranchInst() {
        if (true_block)
            true_block->removeParent(this->bb);
        if (false_block)
            false_block->removeParent(this->bb);
    }

    JumpInst::~JumpInst() {
        if (to)
            to->removeParent(this->bb);
    }

    BasicBlock::~BasicBlock() {
        if (!parentInsts.empty())
            std::cerr << "ir: WARN! BasicBlock with predecessors destructed!" << std::endl;
        for (auto &suc:succ())
            suc->removeParent(this);
        for (auto &inst:iList)
            if (inst->bb == this)
                delete inst;
    }

    int BasicBlock::InsertAtEnd(Value *value) {
        iList.push_back(value);
        value->bb = this;
        return 0;
    }

    int BasicBlock::InsertBeforeLast(Value *value){
        if(iList.empty())
        {
            return InsertAtEnd(value);
        }
        auto it = iList.end();
        it--;
        iList.insert(it,value);
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

    Value *BasicBlock::getVariable(ast::Decl *decl, IRBuilder &builder) {
        // Local Value Numbering: lookup variable's current definition and return it.
        ir::Value *val = decl->lookup_var_def(this);
        if (val)
            return val;

        // Global Value Numbering: Lookup its definitions in BB predecessors.
        // CFG incomplete
        if (!sealed) {
            // construct an operand-less Phi, fill its operands later during sealing.
            PhiInst *phi = builder.CreatePhi(this);
            // record it as the current definition.
            incompletePhis[decl] = phi;
            val = phi;
        } else if (parentInsts.size() == 1) {
            // CFG complete. There won't be unknown predecessors:
            // For BBs with a single predecessor, get Value from its parent block.
            val = parentInsts.front()->bb->getVariable(decl, builder);
        } else {
            // Otherwise we need to create a Phi for all its predecessors
            // construct an operand-less Phi...
            PhiInst *phi = builder.CreatePhi(this);
            // ...and record it as Decl's current definition to break possible endless loop.
            decl->set_var_def(this, phi);
            val = addPhiOperands(decl, phi, builder);
            // phi may be invalid at this point. Reassign it below.
        }
        decl->set_var_def(this, val);
        return val;
    }

    Value *BasicBlock::addPhiOperands(ast::Decl *decl, PhiInst *phi, IRBuilder &builder) {
        for (auto pred:parentInsts)
            phi->InsertElem(pred->bb, pred->bb->getVariable(decl, builder));
        return tryRemoveTrivialPhi(phi, builder);
    }

    Value *BasicBlock::tryRemoveTrivialPhi(PhiInst *phi, IRBuilder &builder) {
        Value *same = nullptr;
        for (auto &op_it:phi->phicont) {
            Value *op = op_it.second->value;
            if (op == same || op == phi)
                continue; // first unique value or self-reference
            if (same)
                return phi; // The phi merges at least two values: not trivial
            same = op;
        }

        if (!same) // The phi is unreachable or in the start block
            same = builder.getConstant(0); // FIXME: Do we have an Undef?

        // Replace all users of this Phi with same
        phi->replaceWith(same, false);

        // Try to recursively remove all phi users, which might have become trivial
        for (auto cur_use:phi->uList) {
            if (cur_use->user == phi || cur_use->value != phi)
                continue;
            PhiInst *curphi = dynamic_cast<ir::PhiInst *>(cur_use->user);
            if (curphi)
                curphi->bb->tryRemoveTrivialPhi(curphi, builder);
        }

        phi->uList.clear();

        // remove use
        for (auto &op_it:phi->phicont)
            op_it.second->removeFromUList();

        phi->bb->eraseInst(phi);
        delete phi;
        return same;
    }

    void BasicBlock::sealBlock(IRBuilder &builder) {
        for (auto phi_it:incompletePhis)
            addPhiOperands(phi_it.first, phi_it.second, builder);
        sealed = true;
    }

    void BasicBlock::eraseInst(Inst *inst) {
        iList.remove(inst);
    }

    std::vector<BasicBlock *> BasicBlock::succ() {
        if (iList.empty())
            return {};
        if (auto branch = dynamic_cast<BranchInst *>(iList.back()))
            return {branch->true_block, branch->false_block};
        else if (auto jump = dynamic_cast<JumpInst *>(iList.back()))
            return {jump->to};
        return {};
    }

    void BasicBlock::removeParent(BasicBlock *bb) {
        for (auto &inst:iList) {
            auto x = dynamic_cast<ir::PhiInst *>(inst);
            if (!x)
                break;
            x->removeBB(bb);
        }
        parentInsts.remove_if(([&](Value *inst) { return inst->bb == bb; }));
    }

    void BasicBlock::replacePred(BasicBlock *oldbb, BasicBlock *newbb) {
        for (auto &inst:iList) {
            auto x = dynamic_cast<ir::PhiInst *>(inst);
            if (!x)
                break;
            x->replaceBB(oldbb, newbb);
        }
        parentInsts.remove_if(([&](Value *inst) { return inst->bb == oldbb; }));
    }

    void BasicBlock::replaceSucc(BasicBlock *oldbb, BasicBlock *newbb) {
        if (auto branch = dynamic_cast<BranchInst *>(iList.back())) {
            if ((branch->true_block != oldbb) && (branch->false_block != oldbb))
                throw std::runtime_error("old bb isn't a branch target of current bb.");
            oldbb->parentInsts.remove(branch);
            if (branch->true_block == oldbb)
                branch->true_block = newbb;
            if (branch->false_block == oldbb)
                branch->false_block = newbb;
            if (branch->true_block == branch->false_block) {
                std::cerr << "useless conditional branch eliminated." << std::endl;
                eraseInst(branch);
                newbb->parentInsts.remove(branch);
                delete branch;
                InsertAtEnd(new JumpInst(newbb));
            } else {
                newbb->addParentInst(branch);
            }
        } else if (auto jump = dynamic_cast<JumpInst *>(iList.back())) {
            if (jump->to != oldbb)
                throw std::runtime_error("old bb isn't a jump target of current bb.");
            jump->to = newbb;
            newbb->addParentInst(jump);
        } else {
            throw std::runtime_error("this bb doesn't go elsewhere.");
        }
        oldbb->removeParent(this);
    }

    bool BasicBlock::operator<(const BasicBlock &x) const {
        return dom_tree_depth < x.dom_tree_depth;
    }



    ConstValue::ConstValue(int _value) : Value(OpType::CONST) {
        value = _value;
    }

    ConstValue::~ConstValue() {

    }

    Value *IRBuilder::getConstant(int _value) {
        if (const_pool.find(_value) != const_pool.end()) return const_pool[_value];
        auto constp = new ConstValue(_value);
        const_pool[_value] = constp;
        return constp;
    }

    Value *IRBuilder::CreateLoadInst(Value *ptr) {
        auto *instp = new LoadInst(ptr);
        auto *curblock = GetCurBlock();
        curblock->InsertAtEnd(instp);
        return instp;
    }

    Value *IRBuilder::CreateReturnInst(Value *val) {
        auto *instp = new ReturnInst(val);
        auto *curblock = GetCurBlock();
        curblock->InsertAtEnd(instp);
        return instp;
    }

    Value *IRBuilder::CreateJumpInst(BasicBlock *_to) {
        auto *instp = new JumpInst(_to);
        auto *curblock = GetCurBlock();
        curblock->InsertAtEnd(instp);
        return instp;
    }

    Value *IRBuilder::CreateBranchInst(Value *c, BasicBlock *t, BasicBlock *f) {
        auto *instp = new BranchInst(c, t, f);
        auto *curblock = GetCurBlock();
        curblock->InsertAtEnd(instp);
        return instp;
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

    Value *IRBuilder::CreateAllocaInst(ast::Decl *decl) {
        auto *instp = new AllocaInst(decl);
        auto *curblock = GetCurBlock();
        curblock->InsertAtEnd(instp);
        return instp;
    }

    Value *PhiInst::GetRelatedValue(BasicBlock *basicblock) {
        if (phicont.find(basicblock) == phicont.end()) return nullptr;
        return phicont[basicblock]->value;
    }

    PhiInst::PhiInst() : Inst(OpType::PHI) {

    }

    int PhiInst::InsertElem(BasicBlock *basicblock, Value *value) {
        phicont[basicblock] = std::make_unique<Use>(this, value);
        return 0;
    }

    PhiInst *IRBuilder::CreatePhi(BasicBlock *tgt_block) {
        auto *instp = new PhiInst();
        tgt_block->InsertAtFront(instp);
        return instp;
    }

    void PhiInst::replaceBB(BasicBlock *oldbb, BasicBlock *newbb) {
        if (phicont.find(oldbb) != phicont.end()) {
            phicont[oldbb]->removeFromUList();
            InsertElem(newbb, phicont[oldbb]->value);
            phicont.erase(oldbb);
        }
    }

    void PhiInst::removeBB(BasicBlock *bb) {
        phicont.erase(bb);
    }

    CallInst::CallInst(std::string n, bool _is_void) : Inst(OpType::CALL), fname(std::move(n)), is_void(_is_void) {}

    AllocaInst::AllocaInst(ast::Decl *_decl) : Inst(OpType::ALLOCA) {
        decl = _decl;
        if (!decl->is_array())
            throw std::runtime_error("We don't alloca for variables!");
        size = decl->array_multipliers[0];
    }

    GetElementPtrInst::GetElementPtrInst(Value *_arr, const std::vector<Value *> &_dims, std::vector<int> muls) :
            AccessInst(OpType::GETELEMPTR), arr(this, _arr) {
        if (auto arr_val = dynamic_cast<AllocaInst *>(arr.value))
            decl = arr_val->decl;
        else if (auto arr_val = dynamic_cast<GlobalVar *>(arr.value))
            decl = arr_val->decl;
        else if (auto arr_val = dynamic_cast<FuncParam *>(arr.value))
            decl = arr_val->decl;
        else
            decl = nullptr;

        dims.reserve(_dims.size() + 1);
        for (auto &i:_dims) {
            dims.emplace_back(this, i);
        }
        if (muls.size() > 1)
            multipliers.insert(multipliers.end(), std::next(muls.begin()), muls.end());
        multipliers.emplace_back(1);
    }

    AccessInst::AccessInst(OpType _optype) : Inst(_optype) {

    }


    Value *IRBuilder::CreateGetElementPtrInst(Value *arr, std::vector<Value *> dims, std::vector<int> muls) {
        auto instp = new GetElementPtrInst(arr, dims, std::move(muls));
        auto *curblock = GetCurBlock();
        curblock->InsertAtEnd(instp);
        return instp;
    }

    LoadInst::LoadInst(Value *_ptr) : AccessInst(OpType::LOAD), ptr(this, _ptr) {}

    StoreInst::StoreInst(Value *_ptr, Value *_val) :
            AccessInst(OpType::STORE), ptr(this, _ptr), val(this, _val) {}

    Value *IRBuilder::CreateStoreInst(Value *ptr, Value *val) {
        auto instp = new StoreInst(ptr, val);
        auto *curblock = GetCurBlock();
        curblock->InsertAtEnd(instp);
        return instp;
    }

    Value *IRBuilder::CreateFuncCall(std::string name, bool is_void, std::vector<ast::Exp *> &params) {

        auto instp = new ir::CallInst(name, is_void);
        auto *curblock = GetCurBlock();
        // XXX: should we convert it to unique_ptr instead?
        instp->params.reserve(params.size());
        for (auto &i:params) {
            auto par = i->codegen(*this);
            instp->params.emplace_back(instp, par);
            //for pointer, llvm ir requires a "i32 0" in the back of the GEP
            auto gep = dynamic_cast<GetElementPtrInst *>(par);
            if (gep) {
                gep->dims.emplace_back(gep, getConstant(0));
            }
        }

        curblock->InsertAtEnd(instp);
        return instp;
    }

    int Function::getInstCount() {
        int ret = 0;
        for (auto &bb:bList) {
            ret += (int) bb->iList.size();
        }
        return ret;
    }

    bool BinaryInst::is_icmp() const {
        return optype == OpType::EQ || optype == OpType::NE || optype == OpType::SGE || optype == OpType::SGT ||
               optype == OpType::SLE || optype == OpType::SLT;
    }

    std::vector<Value *> BinaryInst::uses() {
        return {ValueL.value, ValueR.value};
    }

    std::vector<Value *> PhiInst::uses() {
        std::vector<Value *> ret;
        ret.reserve(phicont.size());
        for (auto &i:phicont)
            ret.emplace_back(i.second->value);
        return ret;
    }

    std::vector<Value *> CallInst::uses() {
        std::vector<Value *> ret;
        ret.reserve(params.size());
        for (auto &i:params)
            ret.emplace_back(i.value);
        return ret;
    }

    std::vector<Value *> BranchInst::uses() {
        return {cond.value};
    }

    std::vector<Value *> ReturnInst::uses() {
        if (val.value)
            return {val.value};
        return {};
    }

    std::vector<Value *> LoadInst::uses() {
        return {ptr.value};
    }

    std::vector<Value *> StoreInst::uses() {
        return {ptr.value, val.value};
    }

    std::vector<Value *> GetElementPtrInst::uses() {
        std::vector<Value *> ret;
        ret.reserve(dims.size() + 1);
        for (auto &i:dims)
            ret.emplace_back(i.value);
        ret.emplace_back(arr.value);
        return ret;
    }

    Loop::Loop() {}

    void Loop::updateBasicBlocks() {
        for (auto * bb : body)
            bb->loop_depth = this->depth;
    }

}
