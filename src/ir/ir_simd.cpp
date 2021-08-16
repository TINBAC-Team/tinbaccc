#include <ir/ir.h>
#include <ir/vectorization.h>
// SIMD
namespace ir {
    std::vector<Value *> AdjacentMemory::uses() {
        std::vector<Value *> ret;
        ret.reserve(dims.size() + 1);
        for (auto &i:dims)
            ret.emplace_back(i.value);
        ret.emplace_back(arr.value);
        return ret;
    }

    std::vector<Value *> VDupInst::uses() {
        return {};
    }

    std::vector<Value *> VBinaryInst::uses() {
        return {ValueL.value, ValueR.value};
    }

    std::vector<Value *> VLoadInst::uses() {
        return {ptr.value};
    }

    std::vector<Value *> VStoreInst::uses() {
        return {ptr.value, val.value};
    }
}
