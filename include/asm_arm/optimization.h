//
// Created by colors_wind on 2021/8/3.
//

#ifndef TINBACCC_OPTIMIZATION_H
#define TINBACCC_OPTIMIZATION_H

#include <asm_arm/instructions.h>
namespace asm_arm {
    typedef std::set<Inst*> InstSet;
    typedef std::set<Operand *> OperandSet;
    typedef std::list<Inst*> InstLinkedList;
    typedef std::unordered_map<Operand*, InstSet> Operand2InstSet;
    typedef std::unordered_map<Operand*, InstLinkedList::iterator> Operand2Position;

    void livenessAnalysis(Function* function);

    void architecture_optimize(asm_arm::Module *module);

    class ArchitectureOptimizer {
    public:
        explicit ArchitectureOptimizer(BasicBlock* bb);
        void process();
    private:
        BasicBlock* bb;
        // Operand -> {Inst | use the operand}
        Operand2InstSet use;
        // Operand -> Inst that generate the operand
        Operand2Position def;

        InstSet* getUse(Operand* key);

        void build();

        void tryCombineMLA(InstLinkedList::iterator &iter);

        InstLinkedList::iterator getDef(Operand *n) {
            auto iter = def.find(n);
            if (iter != def.cend())
                return iter->second;
            else
                return bb->insts.end();
        }


    };
}

#endif //TINBACCC_OPTIMIZATION_H
