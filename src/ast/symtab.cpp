#include "ast/symtab.h"

int symtab::InsertVar(const std::string &name, const ast::Decl *pointer) {
    symtab_elem &cur_elem = scope_vec.back();
    if(cur_elem.find(name)!=cur_elem.end()) return 1;
    cur_elem[name] = (ast::Node *) pointer;
    return 0;
}

int symtab::InsertFunc(const std::string &name, const ast::Function *pointer) {
    symtab_elem &cur_elem = scope_vec.back();
    if(cur_elem.find(name)!=cur_elem.end()) return 1;
    cur_elem[name] = (ast::Node *) pointer;
    return 0;
}

int symtab::EnterScope() {
    scope_vec.emplace_back();
    return 0;
}

int symtab::ExitScope() {
    if(scope_vec.size() <= 1) return 1;
    scope_vec.pop_back();
    return 0;
}

ast::Decl *symtab::GetVar(const std::string &name) {
    for(auto i = scope_vec.rbegin(); i!=scope_vec.rend(); i++)
    {
        if(i->find(name) != i->end())
        {
            if(dynamic_cast<ast::Decl*> ((*i)[name]))
                return dynamic_cast<ast::Decl*> ((*i)[name]);
            else return nullptr;
        }
    }
    return nullptr;
}

ast::Function *symtab::GetFunc(const std::string &name) {
    for(auto i = scope_vec.rbegin(); i!=scope_vec.rend(); i++)
    {
        if(i->find(name) != i->end())
        {
            if(dynamic_cast<ast::Function*> ((*i)[name]))
                return dynamic_cast<ast::Function*> ((*i)[name]);
            else return nullptr;
        }
    }
    return nullptr;
}

symtab::symtab() {
    scope_vec.emplace_back();
}
