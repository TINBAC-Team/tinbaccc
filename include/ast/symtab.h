#ifndef TINBACCC_SYMTAB_H
#define TINBACCC_SYMTAB_H

#endif //TINBACCC_SYMTAB_H
#include <string>
#include <map>
#include <vector>
#include <unordered_map>
#include <set>
#include <vector>
#include <ast/ast.h>
typedef std::unordered_map <std::string, ast::Node*> symtab_elem;
class symtab
{
public:
    symtab();
    int InsertVar(const std::string &name,const ast::Decl* pointer);
    int InsertFunc(const std::string &name,const ast::Function* pointer);
    int EnterScope();
    int ExitScope();
    ast::Decl* GetVar(const std::string &name);
    ast::Function* GetFunc(const std::string &name);
private:
    std::vector <symtab_elem> scope_vec;
};