#include <ast/ast.h>

namespace ast {
    CompUnit::CompUnit() {
        entries.emplace_back(Function::create_extern(Function::Type::INT, "getint", {}));
        entries.emplace_back(Function::create_extern(Function::Type::INT, "getch", {}));
        entries.emplace_back(Function::create_extern(Function::Type::INT,
                                                     "getarray",
                                                     {Decl::create_param("a", {new Exp(0)})})
        );
        entries.emplace_back(Function::create_extern(Function::Type::VOID,
                                                     "putint",
                                                     {Decl::create_param("a", {})})
        );
        entries.emplace_back(Function::create_extern(Function::Type::VOID,
                                                     "putch",
                                                     {Decl::create_param("a", {})})
        );
        entries.emplace_back(Function::create_extern(Function::Type::VOID,
                                                     "putarray",
                                                     {Decl::create_param("n", {}),
                                                      Decl::create_param("a", {new Exp(0)})})
        );
        entries.emplace_back(Function::create_extern(Function::Type::VOID,
                                                     "memset",
                                                     {Decl::create_param("arr", {new Exp(0)}),
                                                     Decl::create_param("num", {}),
                                                      Decl::create_param("count", {})})
        );
        // TODO: printf
        entries.emplace_back(Function::create_extern(Function::Type::VOID,
                                                     "_sysy_starttime",
                                                     {Decl::create_param("lineno", {})})
        );
        entries.emplace_back(Function::create_extern(Function::Type::VOID,
                                                     "_sysy_stoptime",
                                                     {Decl::create_param("lineno", {})})
        );
    }

    CompUnit::~CompUnit() {
        for (auto i:entries)
            delete i;
    }

    void CompUnit::append_decls(std::vector<ast::Decl *> entries) {
        this->entries.insert(std::end(this->entries), std::begin(entries), std::end(entries));
    }

    void CompUnit::append_function(ast::Function *entry) {
        this->entries.emplace_back(entry);
    }

    InitVal::~InitVal() {
        if (exp)
            delete exp;
        for (auto i:vals)
            delete i;
    }

    LVal::~LVal() {
        for (auto i:array_dims)
            delete i;
    }

    void LVal::add_dim(int dim) {
        array_dims.emplace_back(new Exp(dim));
    }

    void LVal::add_dim(Exp *dim) {
        array_dims.emplace_back(dim);
    }

    Decl::Decl(FuncFParam *param) : initval(nullptr), is_const(false), is_fparam(true),is_global(false), type(param->type) {
        name = std::move(param->signature->name);
        array_dims = std::move(param->signature->array_dims);
        delete param;
    }

    Decl::~Decl() {
        if (initval)
            delete initval;
        for (auto i:array_dims)
            delete i;
    }

    Decl *Decl::create_param(const std::string n, std::vector<Exp *> d, VarType t) {
        Decl *ret = new Decl(n);
        ret->type = t;
        ret->array_dims = std::move(d);
        ret->is_fparam = true;
        return ret;
    }

    int Decl::get_value(int offset) {
        if (!is_const)
            throw std::runtime_error(name + "isn't a constant.");
        if (initval_expanded[offset])
            return initval_expanded[offset]->get_value();
        return 0;
    }

    ir::Value *Decl::lookup_var_def(const ir::BasicBlock *b) {
        auto got = var_defs.find(b);
        if (got != var_defs.end())
            return got->second->value;
        return nullptr;
    }

    void Decl::set_var_def(const ir::BasicBlock *b, ir::Value *v) {
        var_defs[b] = std::make_unique<ir::Use>(nullptr, v);
    }

    Exp::~Exp() {
        if (lval)
            delete lval;
        if (funccall)
            delete funccall;
        if (lhs)
            delete lhs;
        if (rhs)
            delete rhs;
    }

    int Exp::get_value() {
        if (op != Op::CONST_VAL)
            throw std::runtime_error("expression isn't a constant.");
        return const_val;
    }

    Function::~Function() {
        for (auto i:params)
            delete i;
        delete block;
    }

    Function *Function::create_extern(Type t, std::string n, std::vector<Decl *> p) {
        Function *ret = new Function(t, n, nullptr);
        ret->is_extern = true;
        ret->params = std::move(p);
        return ret;
    }

    FuncCall::FuncCall(std::string n, int l) : name(std::move(n)), lineno(l) {
        mangle_params();
    }

    void FuncCall::mangle_params() {
        if (name == "starttime") {
            name = "_sysy_starttime";
            params.emplace_back(new Exp(lineno));
        } else if (name == "stoptime") {
            name = "_sysy_stoptime";
            params.emplace_back(new Exp(lineno));
        } else if (name == "putf") {
            name = "printf";
            throw std::runtime_error("putf isn't implemented yet.");
        }
    }

    void Block::append_nodes(std::vector<ast::Node *> entries) {
        this->entries.insert(std::end(this->entries), std::begin(entries), std::end(entries));
    }
}