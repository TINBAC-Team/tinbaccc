#pragma clang diagnostic push
#pragma ide diagnostic ignored "bugprone-reserved-identifier"
#ifndef TINBACCC_IR_H
#define TINBACCC_IR_H

#include <set>
#include <vector>
#include <list>
#include <unordered_map>
#include <memory>
#include <string>
#include <iostream>

namespace ast {
    class Function;

    class Decl;

    class FuncCall;

    class Exp;
}

namespace ir {
    class Inst;

    class Use;

    class BasicBlock;

    class Value;

    class ConstValue;

    class GlobalVar;

    class Function;

    class PhiInst;

    typedef std::unordered_map<int, ConstValue *> ConstPool;
    typedef std::unordered_map<BasicBlock *, std::unique_ptr<Use>> PhiContent;

    typedef std::list<Value *> instList;
    typedef std::set<Use *> UseList;
    typedef std::list<GlobalVar *> GlobalVarList;
    typedef std::list<Function *> FunctionList;
    typedef std::list<BasicBlock *> BlockList;
    typedef std::list<PhiInst *> PhiList;

    enum class OpType {
#include "allop.inc"
    };

    class Module {
    public:
        FunctionList functionList;
        GlobalVarList globalVarList;
    };

    std::ostream &operator<<(std::ostream &os, const Module &dt);

    class IRBuilder {
    public:
        IRBuilder(Module *m);

        Module *module;
        Function *curFunction;
        BasicBlock *CurBlock;
        // Blocks for control flow instructions
        // True/FalseBlock are for condition translation
        // ContBlock is the block after current if
        // EntryBlock is the while condition entry
        BasicBlock *TrueBlock, *FalseBlock, *ContBlock, *WhileContBlock, *EntryBlock;

        BasicBlock *CreateBlock(); //create and enter the created block

        void appendBlock(BasicBlock *block); // append a manually created block

        Function *CreateFunction(std::string name, bool return_int);

        BasicBlock *GetCurBlock() const;

        Value *CreateBinaryInst(Value *_ValueL, Value *_ValueR, OpType optype);

        Value *CreateAllocaInst(int _size);

        Value *CreateGetElementPtrInst(Value *arr, Value *offset);

        Value *CreateLoadInst(Value *ptr);

        Value *CreateStoreInst(Value *ptr, Value *val);

        Value *CreateReturnInst(Value *val);

        Value *CreateJumpInst(BasicBlock *_to);

        Value *CreateBranchInst(Value *c, BasicBlock *t, BasicBlock *f);

        Value *CreateGlobalVar(ast::Decl *decl);

        PhiInst *CreatePhi(BasicBlock *tgt_block);

        Value *CreateFuncCall(std::string name, bool is_void, std::vector<ast::Exp *> &params);

        static Value *getConstant(int _value);

        Value *getConstant(int valueL, int valueR, OpType optype);
    };

    class Value {
    public:
        UseList uList;
        OpType optype;
        BasicBlock *bb;

        Value(OpType _optype);

        int addUse(Use *use);

        virtual void print(std::ostream &os) const;

        virtual ~Value();
    };

    class Use {
    public:
        Value *user;
        Value *value;

        Use(Value *_user, Value *_value = nullptr);

        void use(Value *v);
    };

    class GlobalVar : public Value {
    public:
        ast::Decl *decl;
        std::string name;
        // number of i32 elements
        int len;
        bool is_const;
        std::vector<int> initval;

        explicit GlobalVar(ast::Decl *d);

        void print(std::ostream &os) const;
    };

    class FuncParam : public Value {
    public:
        ast::Decl *decl;

        FuncParam(ast::Decl *d) : Value(OpType::PARAM), decl(d) {}
    };

    class Function {
    public:
        std::string name;
        bool return_int;
        BlockList bList;
        std::vector<FuncParam *> params;

        explicit Function(std::string n, bool ret) : name(std::move(n)), return_int(ret) {}

        /**
         * create a new block, add it to current function and return it.
         * @return pointer to the created block.
         */
        BasicBlock *CreateBlock();

        void appendBlock(BasicBlock *block); // append a manually created block

        void setupParams(const std::vector<ast::Decl *> decls);

        void addParamsToBB(BasicBlock *block);

        void print(std::ostream &os) const;

        bool is_extern() const { return bList.empty(); }
    };

    class BasicBlock {
    public:
        BasicBlock();

        instList iList;
        instList parentInsts;
        // true when constructed. It should be manually unsealed for while entry.
        bool sealed;
        std::unordered_map<ast::Decl *, PhiInst *> incompletePhis;

        int InsertAtEnd(Value *value);

        int InsertAtFront(Value *value);

        void addParentInst(Inst *inst);

        Value *getVariable(ast::Decl *decl, IRBuilder &builder);

        void sealBlock(IRBuilder &builder);

        void eraseInst(Inst *inst);

        void print(std::ostream &os) const;

    private:
        Value *addPhiOperands(ast::Decl *decl, PhiInst *phi, IRBuilder &builder);

        Value *tryRemoveTrivialPhi(PhiInst *phi, IRBuilder &builder);
    };

    class Inst : public Value {
    public:
        explicit Inst(OpType _optype);

        ~Inst() override;
    };

    class BinaryInst : public Inst {
    public:
        Use ValueL, ValueR;

        BinaryInst(OpType _optype, Value *_ValueL, Value *_ValueR);

        void print(std::ostream &os) const;
    };

    class PhiInst : public Inst {
    public:
        PhiContent phicont;

        PhiInst();

        Value *GetRelatedValue(BasicBlock *basicblock);

        int InsertElem(BasicBlock *basicblock, Value *value);

        void print(std::ostream &os) const;
    };

    class CallInst : public Inst {
    public:
        std::string fname;
        bool is_void;
        std::vector<Use> params;

        explicit CallInst(std::string n, bool _is_void);

        void print(std::ostream &os) const;
    };

    class BranchInst : public Inst {
    public:
        Use cond;
        BasicBlock *true_block, *false_block;

        BranchInst(Value *c, BasicBlock *t, BasicBlock *f) :
                Inst(OpType::BRANCH), cond(this, c), true_block(t), false_block(f) {
            if (true_block)
                true_block->addParentInst(this);
            if (false_block)
                false_block->addParentInst(this);
        }

        void print(std::ostream &os) const;
    };

    class JumpInst : public Inst {
    public:
        BasicBlock *to;

        JumpInst(BasicBlock *_to) : Inst(OpType::JUMP), to(_to) {
            to->addParentInst(this);
        };

        void print(std::ostream &os) const;
    };

    class ReturnInst : public Inst {
    public:
        Use val;

        ReturnInst(Value *v) : Inst(OpType::RETURN), val(this, v) {}

        void print(std::ostream &os) const;
    };

    class AccessInst : public Inst {
    public:
        AccessInst(OpType _optype);

    };

    class LoadInst : public AccessInst {
    public:
        Use ptr;

        LoadInst(Value *_ptr);

        void print(std::ostream &os) const;
    };

    class StoreInst : public AccessInst {
    public:
        Use ptr, val;

        StoreInst(Value *_ptr, Value *_val);

        void print(std::ostream &os) const;
    };

    class AllocaInst : public Inst {
    public:
        int size;

        AllocaInst(int _size);

        void print(std::ostream &os) const;
    };

    class GetElementPtrInst : public AccessInst {
    public:
        Use arr;
        Use offset;

        GetElementPtrInst(Value *_arr, Value *_offset);

        void print(std::ostream &os) const;
    };

    class ConstValue : public Value {
    public:
        int value;
        static ConstPool const_pool;

        explicit ConstValue(int _value);

        ~ConstValue() override;

    };
}
#pragma clang diagnostic pop
#endif //TINBACCC_IR_H
