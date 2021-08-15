#include <ir/passes.h>
#include <ir/vectorization.h>

const int MAX_MEMORY_ADJACENT = 1 << 6;
const int FIRST_INST_POS = MAX_MEMORY_ADJACENT / 2;

static bool inferDelta(ir::GetElementPtrInst *inst, int &delta, ir::Value *&base) {
    auto &use = inst->dims.back();
    base = use.value;
    delta = 0;
    while (true) {
        if (auto x = dynamic_cast<ir::BinaryInst *>(base)) {
            if (x->optype == ir::OpType::ADD) {
                if (auto constL = dynamic_cast<ir::ConstValue *>(x->ValueL.value)) {
                    delta += constL->value;
                    base = x->ValueR.value;
                } else if (auto constR = dynamic_cast<ir::ConstValue *>(x->ValueR.value)) {
                    delta += constR->value;
                    base = x->ValueL.value;
                } else return true;
            } else if (x->optype == ir::OpType::SUB) {
                // inst.address = valueR - constL
                if (auto constR = dynamic_cast<ir::ConstValue *>(x->ValueR.value)) {
                    delta += constR->value;
                    base = x->ValueR.value;
                } else return true;
            } else return true;
        } else return true;
    }
}

static bool checkRange(int index) {
    if (index < 0 || index >= MAX_MEMORY_ADJACENT)
        return false;
    return true;
}



class AdjacentMemoryBuilder {
    ast::Decl *decl;
    ir::Value *baseVar;
    std::vector<ir::Value *> address;
    std::unordered_map<ir::Value *, int> indexes;
    std::vector<ir::GetElementPtrInst *> queue;
    int offset;

public:
    AdjacentMemoryBuilder(ir::GetElementPtrInst *inst) : address(MAX_MEMORY_ADJACENT){
        int delta;
        ir::Value *base;
        inferDelta(inst, delta, base);
        this->decl = inst->decl;
        this->baseVar = base;
        // FIRST_INST_POS = delta + offset
        this->offset = FIRST_INST_POS - delta;
        address[delta + offset] = inst;
    }

    bool put(ir::GetElementPtrInst *inst) {
        int delta;
        ir::Value *base;
        if (!inferDelta(inst, delta, base)) return false;
        if (base != baseVar) return false;
        int index = delta + offset;
        if (!checkRange(index)) return false;
        address[index] = inst;
        return true;
    }

    std::vector<ir::AdjacentMemory*> build() {
        std::vector<ir::AdjacentMemory*> result;
        std::vector<ir::Value*> adj;
        for(auto* ptr : address) {
            if (ptr)
                adj.push_back(ptr);
            else
                adj.clear();
            if (adj.size() == 4) {
                result.push_back(new ir::AdjacentMemory(adj));
                result.clear();
            }
        }
        return std::move(result);
    }


};



struct AdjacentMemoryKey {
    const ast::Decl *decl;
    const ir::Value *base;
};

struct HashFunc {
    std::size_t operator()(const AdjacentMemoryKey &key) const {
        return 31 * (std::size_t) key.decl + (std::size_t) key.base;
    }
};

struct EqualFunc {
    bool operator()(const AdjacentMemoryKey &lhs, const AdjacentMemoryKey &rhs) const {
        return lhs.decl == rhs.decl && lhs.base == rhs.base;
    }
};


class Vectorization {
    ir::BasicBlock *bb;
    std::unordered_map<AdjacentMemoryKey, AdjacentMemoryBuilder *, HashFunc, EqualFunc> builders;
    std::unordered_map<AdjacentMemoryKey, std::vector<ir::AdjacentMemory *>, HashFunc, EqualFunc> memories;
    std::vector<ir::GetElementPtrInst *> getPtrInstructions;
    std::vector<ir::AdjacentMemory *> adjacentMemories;
public:
    Vectorization(ir::BasicBlock *bb) : bb(bb) {}

    void analysisAdjacentMemory() {
        for (auto *inst : bb->iList) {
            if (auto *x = dynamic_cast<ir::GetElementPtrInst *>(inst)) {
                int delta;
                ir::Value *base;
                inferDelta(x, delta, base);
                AdjacentMemoryKey key{x->decl, base};
                auto iter = builders.find(key);
                if (iter == builders.cend()) {
                    builders[key] = new AdjacentMemoryBuilder(x);
                } else {
                    iter->second->put(x);
                }
            }
        }
        for (auto & pair : builders) {
            memories[pair.first] = pair.second->build();
        }
    };

};

void ir_passes::vectorize(ir::Module *module) {

    for (auto *func : module->functionList) {
        if (func->is_extern()) continue;
        if (func->bList.empty()) continue;
        for (auto *bb : func->bList) {
            Vectorization{bb}.analysisAdjacentMemory();
        }
    }
}
