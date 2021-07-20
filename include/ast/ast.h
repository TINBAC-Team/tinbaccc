#ifndef TINBACCC_AST_H
#define TINBACCC_AST_H


#include <string>
#include <utility>
#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <memory>
#include <ir/ir.h>

namespace ir {
    class Use;

    class Value;

    class BasicBlock;

    class IRBuilder;
}

namespace ast {
    class ValidationContext;

    class Decl;

    class Function;

    class Node {
    public:
        virtual void print(std::ofstream &ofd) = 0;

        virtual void validate(ValidationContext &ctx);

        virtual ir::Value *codegen(ir::IRBuilder &builder) { return nullptr; }

        virtual ~Node() {}
    };

    class CompUnit : public Node {
    public:
        // Decl or Function. Runtime type checking required.
        // Using two arrays isn't possible as we need to preserve the order.
        std::vector<Node *> entries;

        CompUnit();

        ~CompUnit();

        void append_decls(std::vector<ast::Decl *> entries);

        void append_function(ast::Function *entry);

        void print(std::ofstream &ofd);

        void validate(ValidationContext &ctx) override;

        ir::Value *codegen(ir::IRBuilder &builder);
    };

    class Exp;

    class InitVal : public Node {
    public:
        Exp *exp; // invalid when it's nullptr
        std::vector<InitVal *> vals;
        bool is_const;

        InitVal(Exp *e = nullptr) : exp(e) {}

        InitVal(InitVal *init) : exp(nullptr) {
            vals.emplace_back(init);
        }

        ~InitVal();

        void append_entry(InitVal *init) {
            vals.emplace_back(init);
        }

        void print(std::ofstream &ofd);

        /**
         * fill_array: 填充数组初始值
         *
         * @param dim 当前填充第几层的数组
         * @param offset vals待填充的偏移量
         * @param dims 数组展开后各维度
         * @param dst_vals 展开数值列表
         */
        void fill_array(int dim, int &offset, const std::vector<int> &dims, std::vector<Exp *> &dst_vals);

        void validate(ValidationContext &ctx) override;
    };

    class LVal : public Node {
    public:
        Decl *decl;
        std::string name;
        std::vector<Exp *> array_dims;
        std::vector<ir::Value *> dim_value;

        LVal(const std::string n = "") : name(n) {}

        ~LVal();

        void add_dim(int dim);

        void add_dim(Exp *dim);

        void print(std::ofstream &ofd);

        void validate(ValidationContext &ctx) override;

        bool is_const();

        int get_value();

        ir::Value *resolve_addr(ir::IRBuilder &builder);

        ir::Value *codegen(ir::IRBuilder &builder);
    };

    class FuncFParam;

    class Decl : public Node {
    public:
        bool is_const;
        bool is_fparam;
        bool is_global;

        //codegen生成后数组的地址
        ir::Value* addr;

        // 数组展开后的数组各维度乘数
        std::vector<int> array_multipliers;
        // 数组展开后的初始值
        std::vector<Exp *> initval_expanded;

        std::string name;
        enum class VarType {
            INT,
            CHAR
        } type;
        InitVal *initval; //非空指针有效
        std::vector<Exp *> array_dims; //数组各维度长度，0维为单变量
        std::unordered_map<const ir::BasicBlock *, std::unique_ptr<ir::Use>> var_defs; // variable definition in BBs

        Decl(std::string n = "", InitVal *init = nullptr) : name(n), initval(init), is_const(false), is_fparam(false),
                                                            is_global(false), type(VarType::INT) {}

        Decl(FuncFParam *param);

        static Decl *create_param(const std::string n, std::vector<Exp *> d, VarType t = VarType::INT);

        ~Decl();

        bool is_array() {
            return !array_dims.empty();
        }

        void add_dim(Exp *dim) { array_dims.emplace_back(dim); }

        void set_global() { is_global = true; }

        void print(std::ofstream &ofd);

        int get_value(int offset);

        void validate(ValidationContext &ctx) override;

        void expand_array();

        ir::Value *codegen(ir::IRBuilder &builder);

        ir::Value *lookup_var_def(const ir::BasicBlock *b);

        void set_var_def(const ir::BasicBlock *b, ir::Value *v);
    };

    class FuncCall;

    class Exp : public Node {
    public:
        enum class Op {
            UNARY_PLUS,
            UNARY_MINUS,
            LOGIC_NOT,

            PLUS,
            MINUS,
            MUL,
            DIV,
            MOD,

            LESS_THAN,
            LESS_EQ,
            GREATER_THAN,
            GREATER_EQ,
            EQ,
            INEQ,

            LOGIC_AND,
            LOGIC_OR,

            CONST_VAL,
            LVAL,
            FuncCall
        } op;
        LVal *lval;
        FuncCall *funccall;
        int const_val;
        Exp *lhs, *rhs;

        Exp() : lval(nullptr), funccall(nullptr), lhs(nullptr), rhs(nullptr) {}

        Exp(int val) : op(Exp::Op::CONST_VAL), const_val(val), lval(nullptr), funccall(nullptr), lhs(nullptr),
                       rhs(nullptr) {}

        Exp(LVal *l) : op(Exp::Op::LVAL), lval(l), funccall(nullptr), lhs(nullptr), rhs(nullptr) {}

        Exp(FuncCall *f) : op(Exp::Op::FuncCall), funccall(f), lval(nullptr), lhs(nullptr), rhs(nullptr) {}

        Exp(Op o, Exp *l, Exp *r = nullptr) : lval(nullptr), funccall(nullptr), op(o), lhs(l), rhs(r) {}

        ~Exp();

        void print(std::ofstream &ofd);

        int get_value();

        bool is_const() {
            return op == Op::CONST_VAL;
        }

        std::string op_real();

        void validate(ValidationContext &ctx) override;

        ir::Value *codegen(ir::IRBuilder &builder);

    private:
        ir::Value *codegen_and(ir::IRBuilder &builder);

        ir::Value *codegen_or(ir::IRBuilder &builder);
    };

    class Cond : public Node {
    public:
        Exp *exp;

        Cond() {}

        Cond(Exp *e) : exp(e) {}

        ~Cond() {
            delete exp;
        }

        void print(std::ofstream &ofd);

        void validate(ValidationContext &ctx) override;

        ir::Value *codegen(ir::IRBuilder &builder);
    };

    class FuncCall : public Node {
    public:
        std::string name;
        std::vector<Exp *> params;
        int lineno;
        bool is_void;

        FuncCall() {}

        FuncCall(std::string n, int l = 0);

        ~FuncCall() {
            for (auto i:params)
                delete i;
        }

        void print(std::ofstream &ofd);

        void validate(ValidationContext &ctx) override;

        ir::Value *codegen(ir::IRBuilder &builder);

    private:
        void mangle_params();
    };

    class Block;

    class FuncFParam : public Node {
    public:
        Decl::VarType type;
        LVal *signature;

        FuncFParam() {}

        FuncFParam(Decl::VarType t, const std::string name) : type(t) {
            signature = new LVal(name);
        }

        ~FuncFParam() {
            delete signature;
        }

        void print(std::ofstream &ofd);
    };

    class Function : public Node {
    public:
        enum class Type {
            VOID,
            INT
        } type;
        std::string name;
        std::vector<Decl *> params;
        Block *block;

        Function() {}

        Function(Type t, std::string n, Block *b) : type(t), name(n), block(b) {}

        Function(Type t, std::string n, std::vector<FuncFParam *> &ps, Block *b) : Function(t, n, b) {
            for (auto i : ps)
                params.emplace_back(new Decl(i));
        }

        ~Function();

        static Function *create_extern(Type t, std::string n, std::vector<Decl *> p);

        void print(std::ofstream &ofd);

        void validate(ValidationContext &ctx) override;

        ir::Value *codegen(ir::IRBuilder &builder);
    };

    class Stmt : public Node {
    };

    // Code block is a statement itself.
    class Block : public Stmt {
    public:
        // Decl or Stmt. Runtime type checking required.
        // Using two arrays isn't possible as we need to preserve the order.
        std::vector<Node *> entries;

        ~Block() {
            for (auto i:entries)
                delete i;
        }

        void append_nodes(std::vector<ast::Node *> entries);

        void print(std::ofstream &ofd);

        void validate(ValidationContext &ctx) override;

        ir::Value *codegen(ir::IRBuilder &builder);
    };

    class AssignmentStmt : public Stmt {
    public:
        LVal *lval;
        Exp *exp;

        AssignmentStmt(LVal *l = nullptr, Exp *e = nullptr) : lval(l), exp(e) {}

        ~AssignmentStmt() {
            delete lval;
            delete exp;
        }

        void print(std::ofstream &ofd);

        void validate(ValidationContext &ctx) override;

        ir::Value *codegen(ir::IRBuilder &builder);
    };

    class EvalStmt : public Stmt {
    public:
        Exp *exp;

        EvalStmt(Exp *e = nullptr) : exp(e) {}

        ~EvalStmt() {
            delete exp;
        }

        void print(std::ofstream &ofd);

        void validate(ValidationContext &ctx) override;

        ir::Value *codegen(ir::IRBuilder &builder);
    };

    class IfStmt : public Stmt {
    public:
        Cond *cond;
        Stmt *true_block;
        Stmt *false_block;

        IfStmt(Cond *c = nullptr, Stmt *t = nullptr, Stmt *f = nullptr) : cond(c), true_block(t), false_block(f) {}

        ~IfStmt() {
            delete cond;
            if (true_block)
                delete true_block;
            if (false_block)
                delete false_block;
        }

        void print(std::ofstream &ofd);

        void validate(ValidationContext &ctx) override;

        ir::Value *codegen(ir::IRBuilder &builder);
    };

    class WhileStmt : public Stmt {
    public:
        Cond *cond;
        Stmt *block;

        WhileStmt(Cond *c = nullptr, Stmt *b = nullptr) : cond(c), block(b) {}

        ~WhileStmt() {
            delete cond;
            if (block)
                delete block;
        }

        void print(std::ofstream &ofd);

        void validate(ValidationContext &ctx) override;

        ir::Value *codegen(ir::IRBuilder &builder);
    };

    class BreakStmt : public Stmt {
    public:
        void print(std::ofstream &ofd);

        ir::Value *codegen(ir::IRBuilder &builder);
    };

    class ContinueStmt : public Stmt {
    public:
        void print(std::ofstream &ofd);

        ir::Value *codegen(ir::IRBuilder &builder);
    };

    class ReturnStmt : public Stmt {
    public:
        Exp *ret;

        ReturnStmt(Exp *e = nullptr) : ret(e) {}

        ~ReturnStmt() {
            if (ret)
                delete ret;
        }

        void print(std::ofstream &ofd);

        void validate(ValidationContext &ctx) override;

        ir::Value *codegen(ir::IRBuilder &builder);
    };
}
#endif //TINBACCC_AST_H
