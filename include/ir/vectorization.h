#ifndef TINBACCC_VECTORIZATION_H
#define TINBACCC_VECTORIZATION_H
namespace ir {

    bool static isVectorization(ir::Value* value);

    class AdjacentMemory {
        std::vector<ir::Value *> address;

    public:
        AdjacentMemory(std::vector<ir::Value*> &&address) : address(address){}

        ir::Value *front() const {
            return address.front();
        }

        int length() const {
            return address.size();
        }
    };

    class VBinaryInst : BinaryInst {

    };

    class VLoadInst : LoadInst {

    };

    class VStoreInst : StoreInst {

    };


}
#endif //TINBACCC_VECTORIZATION_H
