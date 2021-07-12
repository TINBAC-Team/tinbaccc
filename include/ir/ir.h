#ifndef TINBACCC_IR_H
#define TINBACCC_IR_H

#endif //TINBACCC_IR_H
#include "ast/ast.h"
#include <set>
#include <vector>
#include <list>
class Inst;
class Use;
class BasicBlock;
class Value;

typedef std::vector<Value*> ValueContainer;
typedef std::vector<BasicBlock*> BasicBlockContainer;

typedef std::vector<Value*> instList;
typedef std::set<Use*> UseList;
typedef std::list<BasicBlock*> BlockList;


enum class OpType{
#include "allop.inc"
};


class IRBuilder
{
public:
    ValueContainer ValCont;
    BasicBlockContainer BBCont;
    IRBuilder();
    BlockList bList;
    BasicBlock* CurBlock;
    BasicBlock* CreateBlock();
    BasicBlock* GetCurBlock() const;
};
class Value
{
public:
    UseList uList;
    Value();
    int addUse(Use* use);
};


class BasicBlock
{
public:
    explicit BasicBlock(IRBuilder &irbuilder);
    instList iList;
    ValueContainer *ValCont;
    Value* CreateBinaryInst(OpType _instop, Value* ValueL, Value* ValueR);
};

class Inst : public Value
{
public:
    Inst(OpType _optype);
    OpType instop;
};

class BinaryInst : public Inst
{
public:
    Value *ValueL, *ValueR;
    BinaryInst(OpType _optype, Value* ValueL, Value* ValueR);
};

class CallInst : public Inst
{

};

class BranchInst : public Inst{

};

class JumpInst: public Inst{

};

class ReturnInst: public Inst{

};

class Use
{
public:
    Inst* user;
    Value* value;
    Use(Inst* _user, Value* _value);
};



