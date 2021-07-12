#ifndef TINBACCC_VALIDATION_H
#define TINBACCC_VALIDATION_H
#include <ast/symtab.h>
#include <memory>

namespace ast {
    class ValidationContext {
    public:
        symtab symbol_table;
    };
}
#endif //TINBACCC_VALIDATION_H
