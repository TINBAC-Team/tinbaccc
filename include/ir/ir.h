#pragma clang diagnostic push
#pragma ide diagnostic ignored "bugprone-reserved-identifier"
#ifndef TINBACCC_IR_H
#define TINBACCC_IR_H

#include "ast/ast.h"
#include <set>
#include <vector>
#include <list>
#include <unordered_map>
using ast::Function;

class Inst;

class Use;

class BasicBlock;

class Value;

class ConstValue;

typedef std::vector<Value *> ValueContainer;
typedef std::vector<BasicBlock *> BasicBlockContainer;
typedef std::unordered_map<int, ConstValue *> ConstPool;
typedef std::vector <std::pair<BasicBlock*,Value*> > PhiParam;
typedef std::unordered_map <BasicBlock*, Value* > PhiContent;

typedef std::list<Value *> instList;
typedef std::set<Use *> UseList;
typedef std::list<BasicBlock *> BlockList;


enum class OpType {
#include "allop.inc"
};


class IRBuilder {
public:
    IRBuilder();

    BlockList bList;
    BasicBlock *CurBlock;

    BasicBlock *CreateBlock(); //create and enter the created block

    BasicBlock *GetCurBlock() const;

    Value* CreateBinaryInst(Value* _ValueL, Value* _ValueR, OpType optype);
    Value* CreateLoadInst();
    Value* CreateCallInst();
    Value *getConstant(int _value);
    Value *getConstant(int valueL, int valueR, OpType optype);
};

class Value {
public:
    UseList uList;
    OpType optype;
    Value(OpType _optype);

    int addUse(Use *use);
    virtual ~Value();
};


class BasicBlock {
public:
    BasicBlock();

    instList iList;
    int InsertAtEnd(Value* value);
    int InsertAtFront(Value* value);
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
    explicit PhiInst(const PhiParam&);
    PhiInst();
    Value* GetRelatedValue(BasicBlock* basicblock);
    int InsertElem(BasicBlock* basicblock, Value* value);
};

class CallInst : public Inst {
public:
    Function *function;
    explicit CallInst(Function *_function);
};

class BranchInst : public Inst {

};

class JumpInst : public Inst {

};

class ReturnInst : public Inst {

};

class AccessInst: public Inst{

};

class LoadInst: public AccessInst{

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

#pragma clang diagnostic pop
#endif //TINBACCC_IR_H
