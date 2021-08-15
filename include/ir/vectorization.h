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
        std::set<IterationAnalyst *> analyst;
        std::unordered_map<ir::Value *, std::pair<ir::VInst *, int>> associatedVInst;

    };

    class VInst : public Value {
    protected:
        bool analysis_(AutoVectorizationContext *context);

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

        AdjacentMemory(std::vector<ir::GetElementPtrInst *> address) :
                VInst(OpType::GETELEMPTR), address(std::move(address)) {}


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

        bool analysis(AutoVectorizationContext *context);

    };

    class VDupInst : public VInst, public IterationAnalyst {
    private:
        int size;
        Value *value;
    public:
        explicit VDupInst(Value *value, int size) : VInst(OpType::DUP), size(size), value(value) {}

        bool analysis(AutoVectorizationContext *context);

        ir::Value *getAssociatedComponent(int index) override {
            return value;
        }

        int getSize() const override {
            return size;
        }

        asm_arm::Operand *codegen(asm_arm::Builder &builder);

        void print(std::ostream &os) const;
    };

    class VBinaryInst : public VInst, public IterationAnalyst {
    public:
        std::vector<BinaryInst *> associated;
        Use ValueL;
        Use ValueR;

        VBinaryInst(OpType optype, Value *_ValueL, Value *_ValueR, std::vector<BinaryInst *> associated) :
                VInst(optype), ValueL(this, _ValueL), ValueR(this, _ValueR), associated(std::move(associated)) {

        }

        bool analysis(AutoVectorizationContext *context);

        ir::BinaryInst *getAssociatedComponent(int index) override {
            return associated[index];
        }

        int getSize() const override {
            return associated.size();
        }


        asm_arm::Operand *codegen(asm_arm::Builder &builder);

        void print(std::ostream &os) const;
    };

    class VLoadInst : public VInst, public IterationAnalyst {
    public:
        Use ptr;
        std::vector<LoadInst *> associated;

        VLoadInst(AdjacentMemory *ptr) : VInst(OpType::LOAD), ptr(this, ptr) {}

        bool analysis(AutoVectorizationContext *context) override;

        ir::LoadInst *getAssociatedComponent(int index) override {
            return associated[index];
        }

        int getSize() const override {
            return associated.size();
        }

        asm_arm::Operand *codegen(asm_arm::Builder &builder);

        void print(std::ostream &os) const;
    };

    class VStoreInst : public VInst, public IterationAnalyst {
        Use ptr;
        Use val;

        std::vector<StoreInst *> associated;
    public:

        bool analysis(AutoVectorizationContext *context);

        ir::StoreInst *getAssociatedComponent(int index) override {
            return associated[index];
        }

        int getSize() const override {
            return associated.size();
        }

        VStoreInst(AdjacentMemory *ptr, VInst *val) : VInst(OpType::STORE), ptr(this, ptr), val(this, val) {}

        asm_arm::Operand *codegen(asm_arm::Builder &builder);

        void print(std::ostream &os) const;
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
            case OpType::SDIV:
                return true;
            default:
                return false;
        }
    }

}
#endif //TINBACCC_VECTORIZATION_H
