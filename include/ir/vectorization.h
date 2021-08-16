#include <utility>

#ifndef TINBACCC_VECTORIZATION_H
#define TINBACCC_VECTORIZATION_H
namespace ir {
    class VInst;

    class VLoadInst;

    class VStoreInst;

    class IterationAnalyst;

    class AutoVectorizationContext {
    public:
        BasicBlock *bb;
        std::set<IterationAnalyst *> analyst;
        std::unordered_map<ir::Value *, std::pair<ir::VInst *, int>> associatedVInst;

        AutoVectorizationContext(BasicBlock *bb): bb(bb) {}
    };

    class VInst : public Value {
    protected:
        bool analysis_(AutoVectorizationContext *context, bool satisfyVector=true, bool satisfyScalar=true);

    public:
        explicit VInst(OpType optype) : Value(optype) {};

        virtual ir::Value *getAssociatedComponent(int index) = 0;

        virtual int getSize() const = 0;

    };

    class IterationAnalyst {
    public:
        virtual bool analysis(AutoVectorizationContext *context) = 0;
    };

    class AdjacentMemory : public VInst, public IterationAnalyst {
    public:
        std::vector<ir::GetElementPtrInst *> address;
        Use arr;
        ast::Decl *decl;
        std::vector<Use> dims;
        std::vector<int> multipliers;

        AdjacentMemory(std::vector<ir::GetElementPtrInst *> &address) :
                arr(this, address[0]->arr.value), VInst(OpType::GETELEMPTR), address(address) {
            this->decl = {this->address[0]->decl};
            this->dims.reserve(this->address[0]->dims.size());
            for (auto &use : this->address[0]->dims) {
                this->dims.emplace_back(this, use.value);
            }
            this->multipliers = this->address[0]->multipliers;
            this->bb = this->address[0]->bb;
        }


        ir::Value *front() const {
            return address.front();
        }

        GetElementPtrInst *getAssociatedComponent(int index) override {
            return address[index];
        }

        int getSize() const override {
            return address.size();
        }

        int indexOf(ir::Value *value) const {
            for (int i = 0; i < address.size(); i++)
                if (address[i] == value) return i;
            return -1;
        }

        bool analysis(AutoVectorizationContext *context) override;

        asm_arm::Operand *codegen(asm_arm::Builder &builder) override;
        void insertToBB(ir::AutoVectorizationContext* context);

        std::vector<Value *> uses() override;

        void print(std::ostream &os) const;
    };

    class VDupInst : public VInst, public IterationAnalyst {
    private:
        Use scalar;
        std::vector<Value *> associated;
    public:
        explicit VDupInst(ir::Value* scalar, std::vector<Value *> associated) : VInst(OpType::DUP), scalar(this, scalar), associated(std::move(associated)) {}

        bool analysis(AutoVectorizationContext *context) override;

        ir::Value *getAssociatedComponent(int index) override {
            return associated[index];
        }

        int getSize() const override {
            return associated.size();
        }

        asm_arm::Operand *codegen(asm_arm::Builder &builder) override;

        void print(std::ostream &os) const override;

        std::vector<Value *> uses() override;
    };

    class VBinaryInst : public VInst, public IterationAnalyst {
    public:
        std::vector<BinaryInst *> associated;
        Use ValueL;
        Use ValueR;

        VBinaryInst(OpType optype, Value *_ValueL, Value *_ValueR, std::vector<BinaryInst *> associated) :
                VInst(optype), ValueL(this, _ValueL), ValueR(this, _ValueR), associated(std::move(associated)) {
            std::cout << "VBinaryInst #" << static_cast<int>(optype) << " Create" << std::endl;
            this->bb = this->associated[0]->bb;
        }

        bool analysis(AutoVectorizationContext *context) override;

        ir::BinaryInst *getAssociatedComponent(int index) override {
            return associated[index];
        }

        int getSize() const override {
            return associated.size();
        }


        asm_arm::Operand *codegen(asm_arm::Builder &builder) override;

        void print(std::ostream &os) const override;

        std::vector<Value *> uses() override;
    };

    class VLoadInst : public VInst, public IterationAnalyst {
    public:
        Use ptr;
        std::vector<LoadInst *> associated;

        VLoadInst(AdjacentMemory *ptr, std::vector<LoadInst *> associated) : VInst(OpType::LOAD),
                                                                             ptr(this, ptr),
                                                                             associated(std::move(associated)) {
            this->bb = this->associated[0]->bb;
            std::cout << "VLoadInst Create" << std::endl;
        }

        bool analysis(AutoVectorizationContext *context) override;

        ir::LoadInst *getAssociatedComponent(int index) override {
            return associated[index];
        }

        int getSize() const override {
            return associated.size();
        }

        asm_arm::Operand *codegen(asm_arm::Builder &builder) override;

        void print(std::ostream &os) const override;

        std::vector<Value *> uses() override;
    };

    class VStoreInst : public VInst, public IterationAnalyst {
        Use ptr;
        Use val;

        std::vector<StoreInst *> associated;
    public:

        bool analysis(AutoVectorizationContext *context) override;

        ir::StoreInst *getAssociatedComponent(int index) override {
            return associated[index];
        }

        int getSize() const override {
            return associated.size();
        }

        VStoreInst(AdjacentMemory *ptr, VInst *val, std::vector<StoreInst *> associated) : VInst(OpType::STORE), ptr(this, ptr),
                                                      val(this, val), associated(std::move(associated)) {
            this->bb = this->associated[0]->bb;
            std::cout << "VStoreInst Create" << std::endl;
        }

        asm_arm::Operand *codegen(asm_arm::Builder &builder) override;

        void print(std::ostream &os) const override;

        std::vector<Value *> uses() override;
    };


    static bool isVector(ir::Value *value) {
        return dynamic_cast<VInst *>(value);
    }

    static Use &getValue(BinaryInst *binaryInst, bool isLeft) {
        if (isLeft)
            return binaryInst->ValueL;
        else
            return binaryInst->ValueR;
    }

    static Use &getValue(StoreInst *storeInst, bool isLeft) {
        if (isLeft)
            return storeInst->ptr;
        else
            return storeInst->val;
    }

    static bool isValidNeonOpType(BinaryInst *inst) {
        switch (inst->optype) {
            case OpType::ADD:
            case OpType::SUB:
            case OpType::MUL:
                return true;
            default:
                return false;
        }
    }

}
#endif //TINBACCC_VECTORIZATION_H
