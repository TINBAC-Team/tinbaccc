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
            ir::BasicBlock *pad = loop->head;
            body_front = *(++std::find(func->bList.begin(), func->bList.end(), pad));

            std::vector<ir::PhiInst *> phis;
            phis.reserve(body_front->iList.size());
            for (auto &inst:body_front->iList)
                if (auto phiinst = dynamic_cast<ir::PhiInst *>(inst)) phis.emplace_back(phiinst);

            ir::PhiInst *phi = nullptr;
            ir::BinaryInst *bin_step_inst;
            std::map<ir::BasicBlock *, std::unique_ptr<ir::Use>>::iterator feedback_pair;
            std::map<ir::BasicBlock *, std::unique_ptr<ir::Use>>::iterator comming_pair;
            int inc_step;
            //Find PHI
            for (auto i = phis.begin(); i != phis.end(); i++) {
                if ((*i)->phicont.size() != 2) continue;
                feedback_pair = std::find_if((*i)->phicont.begin(), (*i)->phicont.end(),
                                             [this](std::map<ir::BasicBlock *, std::unique_ptr<ir::Use>>::value_type &pair) {
                                                 return pair.first == body_front ||
                                                        loop->body.find(pair.first) != loop->body.end();
                                             });
                comming_pair = std::find_if((*i)->phicont.begin(), (*i)->phicont.end(),
                                            [this](std::map<ir::BasicBlock *, std::unique_ptr<ir::Use>>::value_type &pair) {
                                                return pair.first != body_front &&
                                                       loop->body.find(pair.first) == loop->body.end();
                                            });
                if (feedback_pair == (*i)->phicont.end() || comming_pair == (*i)->phicont.end()) continue;
                bool found_inc_step = false;

                for (auto &usage:(*i)->uList) {
                    if (auto addinst = dynamic_cast<ir::BinaryInst *>(usage->user)) {
                        if (addinst->optype != ir::OpType::ADD || addinst->ValueR.value->optype != ir::OpType::CONST ||
                            addinst->ValueL.value != (*i) || feedback_pair->second->value != addinst)
                            continue;
                        found_inc_step = true;
                        bin_step_inst = addinst;
                        inc_step = (dynamic_cast<ir::ConstValue *>(addinst->ValueR.value))->value;
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
                std::cerr << inc_step << std::endl;
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
                        if(gepinst->dims.size()<=2) continue;
                        break;
                        int index_sub = std::distance(gepinst->dims.begin(),index);
                        int multiplier =
                                index_sub == gepinst->dims.size()-1 ? 1 : gepinst->decl->array_multipliers[index_sub + 1];
                        ir::PhiInst *phi_new = new ir::PhiInst();
                        phi->bb->InsertBefore(phi_new, phi);
                        ir::BinaryInst *new_feedback_value = new ir::BinaryInst(ir::OpType::ADD, phi_new,
                                                                                ir::IRBuilder::getConstant(inc_step *
                                                                                                           multiplier*4,
                                                                                                           module));
                        bin_step_inst->bb->InsertBefore(new_feedback_value, bin_step_inst);
                        ir::BinaryInst *new_comming_value = new ir::BinaryInst(ir::OpType::MUL,
                                                                               comming_pair->second->value,
                                                                               ir::IRBuilder::getConstant(
                                                                                       multiplier*4,
                                                                                       module));
                        phi_new->bb->InsertBefore(new_comming_value, phi_new);
                        phi_new->InsertElem(feedback_pair->first, new_feedback_value);
                        phi_new->InsertElem(comming_pair->first, new_comming_value);
                        (*index).removeFromUList();
                        *index = ir::Use(gepinst, ir::IRBuilder::getConstant(0, module));
                        ir::BinaryInst *ptr = new ir::BinaryInst(ir::OpType::ADD, gepinst, phi_new);
                        gepinst->bb->InsertAfter(ptr, gepinst);
                        gepinst->replaceWith(ptr, true);
                        ptr->ValueL.use(gepinst, true);
                    }
                }
            }
        }
    };


    void strength_reduction(ir::Module *module) {
        for (auto &i:module->functionList)
            for (auto &loop:i->loops) {
                StrengthREDPass(loop, i, module).run_pass();
            }


    }
}