#pragma clang diagnostic push
#pragma ide diagnostic ignored "bugprone-reserved-identifier"
#ifndef TINBACCC_IR_H
#define TINBACCC_IR_H

#include <set>
#include <vector>
#include <list>
#include <unordered_map>
#include <map>
#include <memory>
#include <string>
#include <iostream>

namespace ast {
    class Function;

    class Decl;

    class FuncCall;

    class Exp;
}

namespace asm_arm {
    class Builder;

    class Operand;
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
    typedef std::map<BasicBlock *, std::unique_ptr<Use>> PhiContent;

    typedef std::list<Value *> instList;
    typedef std::set<Use *> UseList;
    typedef std::list<GlobalVar *> GlobalVarList;
    typedef std::list<Function *> FunctionList;
    typedef std::list<BasicBlock *> BlockList;
    typedef std::list<PhiInst *> PhiList;

    enum class OpType {
#include "allop.inc"
    };

    struct Loop {
        BasicBlock* head = nullptr;
        std::set<BasicBlock*> body;
        std::vector<Loop*> nested;
        Loop* external = nullptr;
        int depth = 1;

        Loop();

        void updateBasicBlocks();
    };

    class Module {
    public:
        FunctionList functionList, unusedFunctionList;
        GlobalVarList globalVarList;

        void codegen(asm_arm::Builder &builder);
    };

    std::ostream &operator<<(std::ostream &os, const Module &dt);

    class IRBuilder {
    public:
        ConstPool const_pool;
        std::unordered_map<std::string, int> namePool;
        IRBuilder(Module *m);

        Module *module;
        Function *curFunction;
        BasicBlock *CurBlock;
        // Blocks for control flow instructions
        // True/FalseBlock are for condition translation
        // ContBlock is the block after current if
        // EntryBlock is the while condition entry
        BasicBlock *TrueBlock, *FalseBlock, *ContBlock, *WhileContBlock, *EntryBlock;

        BasicBlock *CreateBlock(std::string name = "bb"); //create and enter the created block

        void appendBlock(BasicBlock *block); // append a manually created block

        Function *CreateFunction(std::string name, bool return_int);

        BasicBlock *GetCurBlock() const;

        Value *CreateBinaryInst(Value *_ValueL, Value *_ValueR, OpType optype);

        Value *CreateAllocaInst(ast::Decl* decl);

        Value *CreateGetElementPtrInst(Value *arr, std::vector<Value*> dims, std::vector<int> muls);

        Value *CreateLoadInst(Value *ptr);

        Value *CreateStoreInst(Value *ptr, Value *val);

        Value *CreateReturnInst(Value *val);

        Value *CreateJumpInst(BasicBlock *_to);

        Value *CreateBranchInst(Value *c, BasicBlock *t, BasicBlock *f);

        Value *CreateGlobalVar(ast::Decl *decl);

        PhiInst *CreatePhi(BasicBlock *tgt_block);

        Value *CreateFuncCall(std::string name, bool is_void, std::vector<ast::Exp *> &params);

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

        void replaceWith(Value *val, bool clear_ulist = true);

        virtual void print(std::ostream &os) const;

        bool sideEffect() const;

        /**
         *
         * @param builder
         * @return The last generated reg operand from this SSA value
         */
        virtual asm_arm::Operand* codegen(asm_arm::Builder &builder);

        virtual std::vector<Value *> uses() { return {}; }

        virtual ~Value();
    };

    class Use {
    public:
        Value *user;
        Value *value;

        Use(Value *_user, Value *_value = nullptr);

        ~Use() { removeFromUList(); }

        void use(Value *v, bool remove_from_user = true);

        void removeFromUList();
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

        void print_llvm_type(std::ostream &os) const;
    };

    class Function {
    public:
        std::string name;
        bool return_int;
        BlockList bList;
        BlockList rpoBList;
        std::vector<FuncParam *> params;

        explicit Function(std::string n, bool ret) : name(std::move(n)), return_int(ret) {}

        /**
         * create a new block, add it to current function and return it.
         * @return pointer to the created block.
         */
        BasicBlock *CreateBlock(std::string name);

        void appendBlock(BasicBlock *block); // append a manually created block

        void setupParams(const std::vector<ast::Decl *> decls);

        void addParamsToBB(BasicBlock *block);

        void print(std::ostream &os) const;

        void codegen(asm_arm::Builder &builder);

        bool is_extern() const { return bList.empty(); }

        int getInstCount();
    };

    class BasicBlock {
    public:

        static std::unordered_map<std::string,int> namePool;
        BasicBlock(std::string _name = "bb");

        ~BasicBlock();

        instList iList;
        instList parentInsts;
        // true when constructed. It should be manually unsealed for while entry.
        bool sealed;
        std::unordered_map<ast::Decl *, PhiInst *> incompletePhis;
        // the index of this BB in function reverse-postorder traversal list. Used by dominator calculation.
        int rpo_id;
        // The depth in dominator tree of current node.
        int dom_tree_depth;
        // immediate dominator
        BasicBlock *idom;
        // The depth in depth first search tree of current node.
        int dfs_tree_depth;
        // the depth of loop
        int loop_depth;

        std::string name;


        int InsertAtEnd(Value *value);

        int InsertBefore(Value *value, std::_List_const_iterator<Value *> it);

        int InsertAtFront(Value *value);

        void addParentInst(Inst *inst);

        Value *getVariable(ast::Decl *decl, IRBuilder &builder);

        void sealBlock(IRBuilder &builder);

        void eraseInst(Inst *inst);


        void print(std::ostream &os) const;

        void codegen(asm_arm::Builder &builder);

        std::vector<BasicBlock *> succ();

        void removeParent(BasicBlock *bb);

        // replace predecessor in PHIs and remove oldbb from parentInsts
        void replacePred(BasicBlock *oldbb, BasicBlock *newbb);

        // replace successor in branch instructions and add branch into newbb.parentInsts
        void replaceSucc(BasicBlock *oldbb, BasicBlock *newbb);

        bool operator < (const BasicBlock &x) const;

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

        asm_arm::Operand* codegen(asm_arm::Builder &builder);

        asm_arm::Operand* codegen_value(asm_arm::Builder &builder);

        bool is_icmp() const;

    private:
        asm_arm::Operand* codegen_mod(asm_arm::Builder &builder);

        asm_arm::Operand* codegen_mul(asm_arm::Builder &builder);

        std::vector<Value *> uses();
    };

    class PhiInst : public Inst {
    public:
        PhiContent phicont;

        PhiInst();

        Value *GetRelatedValue(BasicBlock *basicblock);

        int InsertElem(BasicBlock *basicblock, Value *value);

        void replaceBB(BasicBlock *oldbb, BasicBlock *newbb);

        void removeBB(BasicBlock *bb);

        void print(std::ostream &os) const;

        virtual asm_arm::Operand* codegen(asm_arm::Builder &builder);

        std::vector<Value *> uses();
    };

    class CallInst : public Inst {
    public:
        std::string fname;
        bool is_void;
        std::vector<Use> params;

        explicit CallInst(std::string n, bool _is_void);

        void print(std::ostream &os) const;

        virtual asm_arm::Operand* codegen(asm_arm::Builder &builder);

        std::vector<Value *> uses();
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

        ~BranchInst();

        void print(std::ostream &os) const;

        /**
         * generate conditional branch with its corresponding CMP
         * @param builder
         * @return
         */
        virtual asm_arm::Operand* codegen(asm_arm::Builder &builder);

        std::vector<Value *> uses();
    };

    class JumpInst : public Inst {
    public:
        BasicBlock *to;

        JumpInst(BasicBlock *_to) : Inst(OpType::JUMP), to(_to) {
            to->addParentInst(this);
        };

        ~JumpInst();

        void print(std::ostream &os) const;

        virtual asm_arm::Operand* codegen(asm_arm::Builder &builder);
    };

    class ReturnInst : public Inst {
    public:
        Use val;

        ReturnInst(Value *v) : Inst(OpType::RETURN), val(this, v) {}

        void print(std::ostream &os) const;

        asm_arm::Operand* codegen(asm_arm::Builder &builder);

        std::vector<Value *> uses();
    };

    class AccessInst : public Inst {
    public:
        AccessInst(OpType _optype);

        static asm_arm::Operand* genptr(asm_arm::Builder &builder, Value *val);
    };

    class LoadInst : public AccessInst {
    public:
        Use ptr;

        LoadInst(Value *_ptr);

        void print(std::ostream &os) const;

        virtual asm_arm::Operand* codegen(asm_arm::Builder &builder);

        std::vector<Value *> uses();
    };

    class StoreInst : public AccessInst {
    public:
        Use ptr, val;

        StoreInst(Value *_ptr, Value *_val);

        void print(std::ostream &os) const;

        virtual asm_arm::Operand* codegen(asm_arm::Builder &builder);

        std::vector<Value *> uses();
    };

    class AllocaInst : public Inst {
    public:
        // number of i32s in this allocation
        int size;
        ast::Decl* decl;

        AllocaInst(ast::Decl* _decl);

        void print(std::ostream &os) const;

        virtual asm_arm::Operand* codegen(asm_arm::Builder &builder);
    };

    class GetElementPtrInst : public AccessInst {
    public:
        Use arr;
        ast::Decl *decl;
        std::vector<Use> dims;
        std::vector<int> multipliers;

        GetElementPtrInst(Value *_arr, const std::vector<Value*>& _dims, std::vector<int> muls);

        void print(std::ostream &os) const;

        void print_llvm_type(std::ostream &os, int start_dim) const;

        virtual asm_arm::Operand* codegen(asm_arm::Builder &builder);

        std::vector<Value *> uses();
    };

    class ConstValue : public Value {
    public:
        int value;

        explicit ConstValue(int _value);

        asm_arm::Operand* genimm(asm_arm::Builder &builder);

        asm_arm::Operand* genop2(asm_arm::Builder &builder);

        asm_arm::Operand* genreg(asm_arm::Builder &builder);

        asm_arm::Operand* codegen(asm_arm::Builder &builder) override;

        ~ConstValue() override;

    };
}
#pragma clang diagnostic pop
#endif //TINBACCC_IR_H
