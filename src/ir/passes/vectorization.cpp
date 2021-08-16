#include <ir/passes.h>
#include <ir/vectorization.h>
#include <algorithm>

const int MAX_MEMORY_ADJACENT = 64;
const int FIRST_INST_POS = MAX_MEMORY_ADJACENT / 2;

static bool inferDelta(ir::GetElementPtrInst *inst, int &delta, ir::Value *&base) {
    auto &use = inst->dims.back();
    base = use.value;
    if (auto x = dynamic_cast<ir::ConstValue*>(base)) {
        base = (ir::Value*)1;
        delta = x->value;
        return true;
    }
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

static bool valueEquals(ir::Value* lhs, ir::Value *rhs) {
    if (lhs == rhs) return true;
    auto x = dynamic_cast<ir::ConstValue*>(lhs);
    auto y = dynamic_cast<ir::ConstValue*>(rhs);
    if (x && y && x->value == y->value) return true;
    return false;
}


class AdjacentMemoryBuilder {
    ast::Decl *decl;
    ir::Value *baseVar;
    std::vector<ir::GetElementPtrInst *> address;
    std::unordered_map<ir::Value *, int> indexes;
    std::vector<ir::GetElementPtrInst *> queue;
    int offset;

public:
    AdjacentMemoryBuilder(ir::GetElementPtrInst *inst) : address(MAX_MEMORY_ADJACENT) {
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

    std::vector<ir::AdjacentMemory *> build() {
        std::vector<ir::AdjacentMemory *> result;
        std::vector<ir::GetElementPtrInst *> adj;
        for (auto *ptr : address) {
            if (ptr)
                adj.push_back(ptr);
            else
                adj.clear();
            if (adj.size() == 4) {
                result.push_back(new ir::AdjacentMemory(adj));
                adj.clear();
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

    void analysisAdjacentMemory(ir::AutoVectorizationContext *context) {
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
        for (auto &pair : builders) {
            auto v = pair.second->build();
            for(auto & adj : v) {
                adj->insertToBB(context);
            }
            memories[pair.first] = std::move((v));
        }
        int debug = 1;
    };

    void tryVectorize(ir::AutoVectorizationContext *context) {
        bool changed = true;
        for (auto &pair : memories) {
            for (auto *adj : pair.second) context->analyst.insert(adj);
        }
        while (changed) {
            changed = false;
            for (auto * analyst : std::set<ir::IterationAnalyst*>{context->analyst}) {
                if (analyst->analysis(context)) {
                    changed = true;
                }
                //
            }
        }
    }

    // determine whether an inst matches the following case:
    // it's only used in inst OR vInst
    bool cleanupInst(ir::AutoVectorizationContext *context, bool clean) {
        for (auto iter = context->bb->iList.cbegin(); iter != context->bb->iList.cend();) {
            auto * inst = *iter;
            if (ir::isVector(inst) || context->associatedVInst.find(inst) == context->associatedVInst.cend()) {
                // ignore
                iter++;
                continue;
            }
            // might be replaced with a vInst
            std::vector<ir::Use*> uListCopy;
            uListCopy.assign(inst->uList.cbegin(),  inst->uList.cend());
            for(auto & use : uListCopy) {
                if (context->associatedVInst.find(use->user) != context->associatedVInst.cend()) {
                    // user is associated with a vInst, ok
                } else {
                    return false;
                }
            }
            if (clean) {
                iter = context->bb->iList.erase(iter);
                delete inst;
            } else {
                iter++;
            }
        }
        return true;
    }

    void cleanVInst(ir::AutoVectorizationContext *context) {
        for (auto iter = context->bb->iList.cbegin(); iter != context->bb->iList.cend();) {
            if (ir::isVector(*iter)) {
                iter = context->bb->iList.erase(iter);
            } else {
                iter++;
            }
        }
    }


};

void ir_passes::vectorize(ir::Module *module) {

    for (auto *func : module->functionList) {
        if (func->is_extern()) continue;
        if (func->bList.empty()) continue;
        for (auto *bb : func->bList) {
            Vectorization v{bb};
            ir::AutoVectorizationContext context{bb};
            v.analysisAdjacentMemory(&context);
            v.tryVectorize(&context);
            if (v.cleanupInst(&context, false)) {
                std::cout << "Successfully Vectorize!" << std::endl;
                v.cleanupInst(&context, true);
            } else {
                std::cout << "Fail to Vectorize!" << std::endl;
                v.cleanVInst(&context);
            }
            int debug = 3;
        }
    }
}


bool ir::AdjacentMemory::analysis(ir::AutoVectorizationContext *context) {
    bool change = false;
    auto *front = this->front();
    for (auto instPtr = front->bb->iList.cbegin(); instPtr != front->bb->iList.cend(); instPtr++) {
        auto *inst = *instPtr;
        if (auto *loadInst = dynamic_cast<LoadInst *>(inst)) {
            int index = this->indexOf(loadInst->ptr.value);
            if (index < 0) continue;
            // ok, we find one, and we must find other loadInst before storeInst

            std::vector<ir::LoadInst*> associated{this->address.size()};
            std::set<GetElementPtrInst *> copyAddress;
            copyAddress.insert(this->address.cbegin(), this->address.cend());
            copyAddress.erase(this->address[index]);
            associated[index] = loadInst;

            auto findOtherLoadInstPtr = instPtr;
            for (++findOtherLoadInstPtr;
                 findOtherLoadInstPtr != front->bb->iList.cend() && !copyAddress.empty(); findOtherLoadInstPtr++) {
                auto *maybeLoadInst = *findOtherLoadInstPtr;
                auto *findLoadInst = dynamic_cast<LoadInst *>(maybeLoadInst);
                if (findLoadInst) {
                    if (copyAddress.erase(dynamic_cast<GetElementPtrInst *>(findLoadInst->ptr.value))) {
                        // fine, found one
                        associated[indexOf(findLoadInst->ptr.value)] = findLoadInst;
                    }
                    continue;
                }
                auto *findStoreInst = dynamic_cast<StoreInst *>(maybeLoadInst);
                if (findStoreInst && copyAddress.find(dynamic_cast<GetElementPtrInst *>(findStoreInst->ptr.value)) !=
                                     copyAddress.cend()) {
                    // RW
                    break;
                }
            }
            // now we check if loadInst is complete
            if (copyAddress.empty()) {
                // analysis success
                auto *vloadInst = new ir::VLoadInst(this, associated);
                instPtr = front->bb->iList.insert(instPtr, vloadInst);
                instPtr++;
                change = true;
                context->analyst.insert(vloadInst);
                for (int i = 0; i < vloadInst->associated.size(); i++) {
                    context->associatedVInst[vloadInst->associated[i]] = {vloadInst, i};
                }
            } else {
                // fail

            }
        }
    }
    context->analyst.erase(this);
    return change;
}

void ir::AdjacentMemory::insertToBB(ir::AutoVectorizationContext *context) {
    auto iter = std::find(bb->iList.begin(), bb->iList.end(), address[0]);
    bb->iList.insert(iter , this);
    for(int i=0;i<address.size();i++) {
        context->associatedVInst[address[i]] = {this, i};
    }
}


struct VectorizeResult {
    bool satisfyVector = false;
    bool satisfyScalar = false;
    ir::VInst* pre = nullptr;
    ir::VInst* vector = nullptr;
};


bool tryCombine(ir::AutoVectorizationContext *context, ir::VInst* knownVectorL, ir::Value* base, VectorizeResult &result) {
    if (context->associatedVInst.find(base) != context->associatedVInst.cend()) return false;
    if(auto *binaryInst = dynamic_cast<ir::BinaryInst *>(base)) {
        std::vector<ir::BinaryInst *> associatedBinaryInst{(unsigned)knownVectorL->getSize()};
        std::vector<ir::ConstValue *> associatedConstInst{(unsigned)knownVectorL->getSize()};
        if (!ir::isValidNeonOpType(binaryInst)) return false;
        // assume:  Vector OP unknown, this value indicates whether to flip


        bool isLeftKnownVector;
        if (knownVectorL->getAssociatedComponent(0) == binaryInst->ValueL.value) {
            isLeftKnownVector = true;
        } else if (knownVectorL->getAssociatedComponent(0) == binaryInst->ValueR.value) {
            isLeftKnownVector = false;
        } else {
            return false;
        }

        ir::VInst *mightSameVectorR = nullptr;
        auto findMightSameVectorR = context->associatedVInst.find(ir::getValue(binaryInst, !isLeftKnownVector).value);
        if (result.satisfyVector  // check whether to find vector
            && findMightSameVectorR != context->associatedVInst.cend()  // find a result
            && findMightSameVectorR->second.second == 0              // the same index
        ) {
            result.satisfyVector = true;
            mightSameVectorR = findMightSameVectorR->second.first;
        } else {
            result.satisfyVector = false;
        }

        // try to determine shared scalar, first we need to find out my scalarR
        ir::Value *mightSameScalar = nullptr;
        if (result.satisfyScalar) {
            mightSameScalar = ir::getValue(binaryInst, !isLeftKnownVector).value;
        }

        if (!result.satisfyVector && !result.satisfyScalar) return false;


        associatedBinaryInst[0] = binaryInst;

        for (int i = 1; i < knownVectorL->getSize(); i++) {
            // only consider the first one that satisfy the requirement
            bool findVector = false;
            bool findScalar = false;
            for (auto *useOther : knownVectorL->getAssociatedComponent(i)->uList) {
                auto *otherBinaryInst = dynamic_cast<ir::BinaryInst *>(useOther->user);
                if (!otherBinaryInst) continue;
                if (binaryInst->optype != otherBinaryInst->optype) continue;
                // check lhs
                if (knownVectorL->getAssociatedComponent(i) != ir::getValue(otherBinaryInst, isLeftKnownVector).value)
                    continue;

                auto &unknownValueR = ir::getValue(otherBinaryInst, !isLeftKnownVector);
                if (result.satisfyScalar && valueEquals(mightSameScalar, unknownValueR.value)) {
                    // ok, scalarR
                    associatedBinaryInst[i] = otherBinaryInst;
                    associatedConstInst[i] = dynamic_cast<ir::ConstValue *>(unknownValueR.value);
                    findScalar = true;
                    break;
                } else if (result.satisfyVector && knownVectorL->getAssociatedComponent(i) == unknownValueR.value) {
                    // ok, vector
                    associatedBinaryInst[i] = otherBinaryInst;
                    findVector = true;
                    break;
                }
            }
            result.satisfyVector &= findVector;
            result.satisfyScalar &= findScalar;

            if (!result.satisfyVector && !result.satisfyScalar) {
                // this associated binaryinst satisfy none of them, don't need to check next.
                break;
            }
        }


        if (!result.satisfyScalar && !result.satisfyVector)
            return false;
        if (result.satisfyScalar) {
            auto *dup = new ir::VDupInst(associatedConstInst);
            result.pre = dup;
            for (int i = 0; i < result.pre->getSize(); i++) {
                context->associatedVInst[result.pre->getAssociatedComponent(i)] = {result.pre, i};
            }
        }

        auto *valueL = knownVectorL;
        auto *valueR = mightSameVectorR;
        if (!isLeftKnownVector) std::swap(valueL, valueR);
        auto * vbinaryInst = new ir::VBinaryInst(binaryInst->optype, valueL, valueR, associatedBinaryInst);
        result.vector = vbinaryInst;

        for (int i = 0; i < result.vector->getSize(); i++) {
            context->associatedVInst[result.vector->getAssociatedComponent(i)] = {result.vector, i};
        }

        return true;
    } else if (auto storeInst = dynamic_cast<ir::StoreInst*>(base)) {
        if (!result.satisfyVector) return false;
        // determine whether knownVector is adjacentMemory
        auto * adj = dynamic_cast<AdjacentMemoryKey*>(knownVectorL);
        bool isLeftKnownVector;
        if (knownVectorL->getAssociatedComponent(0) == storeInst->ptr.value && adj) {
            isLeftKnownVector = true;
        } else if (knownVectorL->getAssociatedComponent(0) == storeInst->val.value && !adj) {
            isLeftKnownVector = false;
        } else {
            return false;
        }

        std::vector<ir::StoreInst *> associatedStoreInst{(unsigned)knownVectorL->getSize()};

        ir::VInst *mightSameVectorR = nullptr;
        auto findSameVectorR = context->associatedVInst.find(ir::getValue(storeInst, !isLeftKnownVector).value);
        if (result.satisfyVector  // check whether to find vector
            && findSameVectorR != context->associatedVInst.cend()  // find a result
            && findSameVectorR->second.second == 0              // the same index
        ) {
            mightSameVectorR = findSameVectorR->second.first;
        } else {
            return false;
        }

        associatedStoreInst[0] = storeInst;

        auto * ptrVector = knownVectorL;
        auto * valVector = mightSameVectorR;
        if (!isLeftKnownVector) std::swap(ptrVector, valVector);

        for (int i = 1; i < knownVectorL->getSize(); i++) {
            // only consider the first one that satisfy the requirement
            bool findVector = false;
            for (auto *useOther : knownVectorL->getAssociatedComponent(i)->uList) {
                auto * otherStoreInst = dynamic_cast<ir::StoreInst*>(useOther->user);
                if (!otherStoreInst) continue;
                if (otherStoreInst->ptr.value != ptrVector->getAssociatedComponent(i)) continue;
                if (otherStoreInst->val.value != valVector->getAssociatedComponent(i)) continue;
                associatedStoreInst[i] = otherStoreInst;
                findVector = true;
                break;
            }
            result.satisfyVector &= findVector;
            if (!result.satisfyVector) break;
        }

        if (result.satisfyVector) {
            auto * vstoreInst = new ir::VStoreInst(dynamic_cast<ir::AdjacentMemory*>(ptrVector), valVector, associatedStoreInst);
            result.vector = vstoreInst;
            for (int i = 0; i < vstoreInst->getSize(); i++) {
                context->associatedVInst[vstoreInst->getAssociatedComponent(i)] = {vstoreInst, i};
            }
            return true;
        }
        return false;
    }
    return false;
}

bool ir::VLoadInst::analysis(ir::AutoVectorizationContext *context) {
    return analysis_(context);
}

bool ir::VStoreInst::analysis(ir::AutoVectorizationContext *context) {
    return analysis_(context, true, false);
}

bool ir::VDupInst::analysis(AutoVectorizationContext *context) {
    return analysis_(context);
}



bool ir::VInst::analysis_(ir::AutoVectorizationContext *context, bool satisfyVector, bool satisfyScalar) {
    bool changed = false;
    auto head = getAssociatedComponent(0);
    for (auto & base : head->uList) {
        VectorizeResult result{satisfyVector, satisfyScalar};
        if (tryCombine(context, this, base->user, result)) {
            if (auto * analyst = dynamic_cast<IterationAnalyst*>(result.vector)) {
                context->analyst.insert(analyst);
            }
            auto iter = std::find(context->bb->iList.begin(), context->bb->iList.end(), result.vector->getAssociatedComponent(0));
            iter = context->bb->iList.insert(iter, result.vector);
            if (result.pre) {
                context->bb->iList.insert(iter, result.pre);
            }
            changed = true;
        }
    }
    return changed;
}


bool ir::VBinaryInst::analysis(ir::AutoVectorizationContext *context) {
    return analysis_(context);
}


