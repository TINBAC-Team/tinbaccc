#include <asm_arm/builder.h>

namespace asm_arm {
    Builder::Builder(Module *m) : module(m) {}

    Function *Builder::createFunction(ir::Function *f) {
        auto *ret = new Function(f);
        module->functionList.push_back(ret);
        curFunction = ret;
        return ret;
    }
    BasicBlock * Builder::createBlock() {
        auto *bb = new BasicBlock();
        curFunction->appendBlock(bb);
        curBlock = bb;
        return bb;
    }

    LDRInst * Builder::createLDR(int v) {
        auto ret = new LDRInst(v);
        curBlock->insertAtEnd(ret);
        return ret;
    }

    LDRInst * Builder::createLDR(std::string s) {
        auto ret = new LDRInst(s);
        curBlock->insertAtEnd(ret);
        return ret;
    }
}