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
}