#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <algorithm>
#include <ir/ir.h>
#include <ast/ast.h>


namespace ir_passes {
    class StrengthREDPass {
    public:
        ir::Loop *loop;
        ir::Function *func;
        ir::BasicBlock *body_front;
        ir::Module *module;

        explicit StrengthREDPass(ir::Loop *_loop, ir::Function *_func, ir::Module *_module) : loop(_loop), func(_func),
                                                                                              module(_module) {}

        void run_pass() {
            ir::BasicBlock *pad = loop->prehead;
            body_front = *(++std::find(func->bList.begin(), func->bList.end(), pad));

            std::vector<ir::PhiInst *> phis;
            phis.reserve(body_front->iList.size());
            for (auto &inst:body_front->iList)
                if (auto phiinst = dynamic_cast<ir::PhiInst *>(inst)) phis.emplace_back(phiinst);

            ir::PhiInst *phi = nullptr;
            ir::BinaryInst *bin_step_inst;
            std::map<ir::BasicBlock *, std::unique_ptr<ir::Use>>::iterator retreat_pair;
            std::map<ir::BasicBlock *, std::unique_ptr<ir::Use>>::iterator comming_pair;
            int inc_step;
            //Find PHI
            for (auto i = phis.begin(); i != phis.end(); i++) {
                if ((*i)->phicont.size() != 2) continue;
                retreat_pair = std::find_if((*i)->phicont.begin(), (*i)->phicont.end(),
                                            [this](std::map<ir::BasicBlock *, std::unique_ptr<ir::Use>>::value_type &pair) {
                                                return pair.first == body_front ||
                                                       loop->body.find(pair.first) != loop->body.end();
                                            });
                comming_pair = std::find_if((*i)->phicont.begin(), (*i)->phicont.end(),
                                            [this](std::map<ir::BasicBlock *, std::unique_ptr<ir::Use>>::value_type &pair) {
                                                return pair.first != body_front &&
                                                       loop->body.find(pair.first) == loop->body.end();
                                            });
                if (retreat_pair == (*i)->phicont.end() || comming_pair == (*i)->phicont.end()) continue;
                bool found_inc_step = false;

                for (auto &usage:(*i)->uList) {
                    if (auto addinst = dynamic_cast<ir::BinaryInst *>(usage->user)) {
                        if (addinst->optype != ir::OpType::ADD || addinst->ValueR.value->optype != ir::OpType::CONST ||
                            addinst->ValueL.value != (*i) || retreat_pair->second->value != addinst)
                            continue;
                        found_inc_step = true;
                        bin_step_inst = addinst;
                        break;
                    }
                }
                if (found_inc_step) {
                    phi = (*i);
                    break;
                }
            }
            //found phi and increase step
            if (phi) {
                //find gep
                for (auto &bb:loop->body) {
                    std::vector<ir::GetElementPtrInst *> geps;
                    geps.reserve(bb->iList.size());
                    for (auto &inst:bb->iList) {
                        if (dynamic_cast<ir::GetElementPtrInst *>(inst)) {
                            geps.emplace_back(dynamic_cast<ir::GetElementPtrInst *>(inst));
                        }
                    }
                    for (auto &gepinst:geps) {
                        auto index = std::find_if(gepinst->dims.begin(), gepinst->dims.end(),
                                                  [&](std::vector<ir::Use>::value_type &val) {
                                                      return val.value == phi;
                                                  });
                        if (index == gepinst->dims.end()) continue;
                        if (gepinst->dims.size() < 2) continue;
                        int index_sub = std::distance(gepinst->dims.begin(), index);
                        if (index_sub + 1 == gepinst->dims.size()) continue;


                        //split GEP into outer GEP and inner GEP
                        std::vector<ir::Value *> dims_outer;
                        for (int i = 0; i <= index_sub; i++) {
                            dims_outer.push_back(gepinst->dims[i].value);
                        }
                        std::vector<int> muls_outer;
                        for (int i = 0; i <= index_sub; i++) {
                            muls_outer.push_back(gepinst->multipliers[i]);
                        }
                        auto outer_gep = new ir::GetElementPtrInst(gepinst->arr.value, dims_outer, {},
                                                                   gepinst->unpack, gepinst->decl);
                        outer_gep->multipliers = std::move(muls_outer);
                        std::vector<ir::Value *> dims_inner;
                        for (int i = index_sub + 1; i < gepinst->dims.size(); i++) {
                            dims_inner.push_back(gepinst->dims[i].value);
                        }
                        std::vector<int> muls_inner;
                        for (int i = index_sub + 1; i < gepinst->dims.size(); i++) {
                            muls_inner.push_back(gepinst->multipliers[i]);
                        }
                        auto inner_gep = new ir::GetElementPtrInst(outer_gep, dims_inner, {},
                                                                   index_sub + gepinst->unpack + 1);
                        inner_gep->multipliers = std::move(muls_inner);
                        gepinst->bb->InsertBefore(inner_gep, gepinst);
                        inner_gep->bb->InsertBefore(outer_gep, inner_gep);
                        gepinst->replaceWith(inner_gep);
                        gepinst->bb->eraseInst(gepinst);
                        delete gepinst;
                        std::cerr << "Performed Strength RED on a GEP." << std::endl;
                    }
                }
            }
        }
    };


    void strength_reduction(ir::Module *module) {
        int cnt = 0;
        for (auto &i:module->functionList)
            for (auto &loop:i->loops) {
                StrengthREDPass(loop, i, module).run_pass();
            }
    }
}