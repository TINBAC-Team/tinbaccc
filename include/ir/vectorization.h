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
        std::set<IterationAnalyst*> analyst;
        std::unordered_map<ir::Value*, std::pair<ir::VInst*, int>> associatedVInst;

    };

    class VInst : public virtual Value {
    public:
        virtual ir::Value* getAssociatedComponent(int index);
        virtual int getSize() const;
    };

    class IterationAnalyst {
    public:
        virtual bool analysis(AutoVectorizationContext* context) = 0;
    };

    class AdjacentMemory : public virtual GetElementPtrInst, public IterationAnalyst {
    public:
        std::vector<ir::GetElementPtrInst *> address;
        AdjacentMemory(std::vector<ir::GetElementPtrInst *> address)
                : GetElementPtrInst(address.front()->arr.value,
                                    [address]() {
                                        std::vector<Value *> dims;
                                        for (auto &use : address.front()->dims)
                                            dims.push_back(
                                                    use.value);
                                        return dims;
                                    }(),
                                    address.front()->multipliers),
                  address(std::move(address)) {}


        ir::Value *front() const {
            return address.front();
        }

        int length() const {
            return address.size();
        }

        int indexOf(ir::Value* value) const {
            for(int i=0;i< address.size();i++)
                if (address[i] == value) return i;
            return -1;
        }

        bool analysis(AutoVectorizationContext* context);

    };

    class VDupInst : public VInst {
    public:
        explicit VDupInst(Value* value);
    };

    class VBinaryInst : public BinaryInst, public IterationAnalyst {
    public:
        std::vector<BinaryInst*> associated;
        VBinaryInst(OpType optype, Value *valueL, Value *valueR, std::vector<BinaryInst*> associated) :
            BinaryInst(optype, valueL, valueR), associated(std::move(associated)) {}

        bool analysis(AutoVectorizationContext* context);

        static bool convert(BinaryInst *inst) {
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

        static Use & getValue(BinaryInst *binaryInst, bool isLeft) {
            if (isLeft)
                return binaryInst->ValueL;
            else
                return binaryInst->ValueR;
        }
    };

    class VLoadInst : public virtual LoadInst, public virtual VInst, public IterationAnalyst {
    public:
        std::vector<LoadInst*> associated;
        VLoadInst(AdjacentMemory *adj);

        bool analysis(AutoVectorizationContext* context) override;

        int indexOfAssociated(ir::Value* inst) {
            for(int i=0;i<associated.size();i++)
                if (associated[i] == inst) return i;
            return -1;
        }

    };

    class VStoreInst : public virtual StoreInst, public virtual VInst, public IterationAnalyst {
        VStoreInst(AdjacentMemory *adj);
    };

    bool static isVector(ir::Value *value) {
        return dynamic_cast<VDupInst*>(value) || dynamic_cast<VBinaryInst*>(value) || dynamic_cast<VLoadInst*>(value);
    }


}
#endif //TINBACCC_VECTORIZATION_H
