#pragma clang diagnostic push
#pragma ide diagnostic ignored "bugprone-reserved-identifier"
#ifndef TINBACCC_IR_H
#define TINBACCC_IR_H

#include <set>
#include <vector>
#include <list>
#include <unordered_map>

namespace ast {
    class Function;

    class Decl;
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
    typedef std::vector<std::pair<BasicBlock *, Value *> > PhiParam;
    typedef std::unordered_map<BasicBlock *, Value *> PhiContent;

    typedef std::list<Value *> instList;
    typedef std::set<Use *> UseList;
    typedef std::list<GlobalVar *> GlobalVarList;
    typedef std::list<Function *> FunctionList;
    typedef std::list<BasicBlock *> BlockList;
    typedef std::list<PhiInst *> PhiList;

    enum class OpType {
#include "allop.inc"
    };


    class IRBuilder {
    public:
        IRBuilder();

        FunctionList functionList;
        GlobalVarList globalVarList;
        Function *curFunction;
        BasicBlock *CurBlock;

        BasicBlock *CreateBlock(); //create and enter the created block

        Function *CreateFunction();

        BasicBlock *GetCurBlock() const;

        Value *CreateBinaryInst(Value *_ValueL, Value *_ValueR, OpType optype);

        Value *CreateAllocaInst(int _size);

        Value *CreateGetElementPtrInst(Value* arr, Value* offset);

        Value *CreateLoadInst(Value* ptr);

        Value *CreateStoreInst(Value* ptr, Value* val);

        Value *CreateCallInst();

        Value *CreateGlobalVar(ast::Decl *decl);

        PhiInst *CreatePhi();

        Value *getConstant(int _value);

        Value *getConstant(int valueL, int valueR, OpType optype);
    };

    class Value {
    public:
        UseList uList;
        OpType optype;
        BasicBlock *bb;

        Value(OpType _optype);

        int addUse(Use *use);

        virtual ~Value();
    };

    class GlobalVar : public Value {
    public:
        ast::Decl *decl;

        explicit GlobalVar(ast::Decl *d) : Value(OpType::GLOBAL), decl(d) {}
    };

    class FuncParam : Value {
    public:
        ast::Decl *decl;

        FuncParam(ast::Decl *d) : Value(OpType::PARAM), decl(d) {}
    };

    class Function {
    public:
        BlockList bList;
        std::vector<FuncParam *> params;

        /**
         * create a new block, add it to current function and return it.
         * @return pointer to the created block.
         */
        BasicBlock *CreateBlock();

        void setupParams(const std::vector<ast::Decl*> decls);
    };

    class BasicBlock {
    public:
        BasicBlock();

        instList iList;
        instList parentInsts;

        int InsertAtEnd(Value *value);

        int InsertAtFront(Value *value);

        void addParentInst(Inst *inst);

        Value *getVariable(ast::Decl *decl, IRBuilder &builder);
    };

    class Inst : public Value {
    public:
        explicit Inst(OpType _optype);

        ~Inst() override;
    };

    class BinaryInst : public Inst {
    public:
        Value *ValueL, *ValueR;

        BinaryInst(OpType _optype, Value *_ValueL, Value *_ValueR);
    };

    class PhiInst : public Inst {
    public:
        PhiContent phicont;

        explicit PhiInst(const PhiParam &);

        PhiInst();

        Value *GetRelatedValue(BasicBlock *basicblock);

        int InsertElem(BasicBlock *basicblock, Value *value);
    };

    class CallInst : public Inst {
    public:
        ast::Function *function;

        explicit CallInst(ast::Function *_function);
    };

    class BranchInst : public Inst {

    };

    class JumpInst : public Inst {

    };

    class ReturnInst : public Inst {

    };

    class AccessInst : public Inst {
    public:
        AccessInst(OpType _optype);
    };

    class LoadInst : public AccessInst {
    public:
        Value* ptr;
        LoadInst(Value* _ptr);
    };

    class StoreInst : public AccessInst{
    public:
        Value* ptr, *val;
        StoreInst(Value* _ptr, Value* _val);
    };

    class AllocaInst : public Inst{
    public:
        int size;
        AllocaInst(int _size);
    };

    class GetElementPtrInst : public AccessInst{
    public:
        Value* arr;
        Value* offset;
        GetElementPtrInst(Value* _arr, Value* _offset);
    };

    class Use {
    public:
        Inst *user;
        Value *value;

        Use(Inst *_user, Value *_value);
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
