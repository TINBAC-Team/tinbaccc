#include <ast/ast.h>

namespace ast {
    void CompUnit::append_decls(std::vector<ast::Decl *> entries) {
        this->entries.insert(std::end(this->entries), std::begin(entries), std::end(entries));
    }

    void CompUnit::append_function(ast::Function *entry) {
        this->entries.emplace_back(entry);
    }

    void LVal::add_dim(int dim) {
        array_dims.emplace_back(new Exp(dim));
    }

    void LVal::add_dim(Exp *dim) {
        array_dims.emplace_back(dim);
    }

    Decl::Decl(FuncFParam *param) : initval(nullptr), is_const(false), is_fparam(true), type(param->type) {
        name = std::move(param->signature->name);
        array_dims = std::move(param->signature->array_dims);
        delete param;
    }

    void Block::append_nodes(std::vector<ast::Node *> entries) {
        this->entries.insert(std::end(this->entries), std::begin(entries), std::end(entries));
    }
}