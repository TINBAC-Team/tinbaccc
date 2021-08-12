#include <ir/passes.h>
#include <map>

namespace ir_passes {
    static void eliminate_load_in_bb(ir::BasicBlock *bb) {
        struct mem_ref {
            ir::Value *a;

            mem_ref(ir::Value *val) : a(val) {}

            bool operator<(const mem_ref &refb) const {
                ir::Value *b = refb.a;
                if (a->optype != b->optype)
                    return a->optype < b->optype;
                if (a->optype == ir::OpType::GETELEMPTR) {
                    auto a_gep = dynamic_cast<const ir::GetElementPtrInst *>(a);
                    auto b_gep = dynamic_cast<const ir::GetElementPtrInst *>(b);
                    if (a_gep->arr.value != b_gep->arr.value)
                        return a_gep->arr.value < b_gep->arr.value;
                    if (a_gep->dims.size() != b_gep->dims.size())
                        return a_gep->dims.size() < b_gep->dims.size();
                    for (int i = 0; i < a_gep->dims.size(); i++)
                        if (a_gep->dims[i].value != b_gep->dims[i].value)
                            return a_gep->dims[i].value < b_gep->dims[i].value;
                    return false;
                }
                return a < b;
            }
        };
        std::map<mem_ref, ir::Value *> mem_ref_map;
        std::map<mem_ref, ir::Value *> mem_ref_map_idom;
        auto get_val_of_ref = [&](ir::Value *ref) -> ir::Value * {
            auto it = mem_ref_map.find(ref);
            if (it != mem_ref_map.end())
                return it->second;
            return nullptr;
        };
        auto clear_alias = [&](ir::Value *ref) {
            if (ref->optype != ir::OpType::GETELEMPTR)
                return;
            auto ref_gep = dynamic_cast<ir::GetElementPtrInst *>(ref);
            for (auto it = mem_ref_map.cbegin(); it != mem_ref_map.cend();) {
                if (it->first.a->optype != ir::OpType::GETELEMPTR) {
                    it++;
                    continue;
                }
                auto map_gep = dynamic_cast<ir::GetElementPtrInst *>(it->first.a);
                if ((ref_gep->arr.value == map_gep->arr.value) ||
                    (ref_gep->arr.value->optype == ir::OpType::GLOBAL &&
                     map_gep->arr.value->optype == ir::OpType::GLOBAL)) {
                    it++;
                    continue;
                }
                if (ref_gep->dims.size() == map_gep->dims.size())
                    it = mem_ref_map.erase(it);
                else
                    it++;
            }
        };
        if (bb->parentInsts.size() == 1 && bb->idom == bb->parentInsts.front()->bb) {
            for (auto inst:bb->idom->iList) {
                if (inst->optype == ir::OpType::STORE) {
                    auto strinst = dynamic_cast<ir::StoreInst *>(inst);
                    mem_ref_map_idom[strinst->ptr.value] = strinst->val.value;
                    clear_alias(strinst->ptr.value);
                } else if (inst->optype == ir::OpType::LOAD) {
                    auto ldrinst = dynamic_cast<ir::LoadInst *>(inst);
                    mem_ref_map_idom[ldrinst->ptr.value] = ldrinst;
                } else if (inst->optype == ir::OpType::CALL) {
                    // don't bring defs across function calls for now.
                    mem_ref_map_idom.clear();
                }
            }
        }
        bool changed = true;
        int inst_cnt = 0;
        while (changed) {
            changed = false;
            mem_ref_map = mem_ref_map_idom;
            for (auto inst_it = bb->iList.begin(); inst_it != bb->iList.end();) {
                auto inst = *inst_it;
                if (inst->optype == ir::OpType::STORE) {
                    auto strinst = dynamic_cast<ir::StoreInst *>(inst);
                    mem_ref_map[strinst->ptr.value] = strinst->val.value;
                    clear_alias(strinst->ptr.value);
                } else if (inst->optype == ir::OpType::LOAD) {
                    auto ldrinst = dynamic_cast<ir::LoadInst *>(inst);
                    auto ldrdef = get_val_of_ref(ldrinst->ptr.value);
                    if (ldrdef) {
                        inst->replaceWith(ldrdef);
                        changed = true;
                        inst_cnt++;
                        bb->iList.erase(inst_it++);
                        delete inst;
                        continue;
                    }
                    mem_ref_map[ldrinst->ptr.value] = ldrinst;
                } else if (inst->optype == ir::OpType::CALL) {
                    // don't bring defs across function calls for now.
                    mem_ref_map.clear();
                }
                inst_it++;
            }
        }
        if (inst_cnt)
            std::cerr << "eliminate_load: " << inst_cnt << " duplicated loads eliminated in " << bb->name << "."
                      << std::endl;
    }

    void eliminate_load(ir::Module *module) {
        for (auto &f:module->functionList)
            for (auto &b:f->bList)
                eliminate_load_in_bb(b);
    }
}