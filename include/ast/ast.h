#ifndef TINBACCC_AST_H
#define TINBACCC_AST_H

#include <string>
#include <utility>
#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <ast/validation.h>

namespace ast {
    class Decl;

    class Function;

    class Node {
    public:
        virtual void print(std::ofstream &ofd) = 0;

        virtual void validate(ValidationContext &ctx);

        virtual ~Node() {}
    };

    class CompUnit : public Node {
    public:
        // Decl or Function. Runtime type checking required.
        // Using two arrays isn't possible as we need to preserve the order.
        std::vector<Node *> entries;

        void append_decls(std::vector<ast::Decl *> entries);

        void append_function(ast::Function *entry);

        void print(std::ofstream &ofd);
    };

    class Exp;

    class InitVal : public Node {
    public:
        Exp *exp; // invalid when it's nullptr
        std::vector<InitVal *> vals;

        InitVal(Exp *e = nullptr) : exp(e) {}

        InitVal(InitVal *init) {
            InitVal();
            vals.emplace_back(init);
        }

        void append_entry(InitVal *init) {
            vals.emplace_back(init);
        }

        void print(std::ofstream &ofd);
    };

    class LVal : public Node {
    public:
        std::string name;
        std::vector<Exp *> array_dims;

        LVal() {}

        LVal(const std::string n) : name(n) {}

        void add_dim(int dim);

        void add_dim(Exp *dim);

        void print(std::ofstream &ofd);
    };

    class Decl : public Node {
    public:
        bool is_const;
        bool is_fparam;
        std::string name;
        enum class VarType {
            INT,
            CHAR
        } type;
        InitVal *initval; //非空指针有效
        std::vector<Exp *> array_dims; //数组各维度长度，0维为单变量

        Decl(std::string n = "", InitVal *init = nullptr) : name(n), initval(init), is_const(false), is_fparam(false),
                                                            type(VarType::INT) {}

        bool is_array() {
            return !array_dims.empty();
        }

        void add_dim(Exp *dim) { array_dims.emplace_back(dim); }

        void print(std::ofstream &ofd);
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

        Exp() {}

        Exp(int val) : op(Exp::Op::CONST_VAL), const_val(val) {}

        Exp(LVal *l) : op(Exp::Op::LVAL), lval(l) {}

        Exp(FuncCall *f) : op(Exp::Op::FuncCall), funccall(f) {}

        Exp(Op o, Exp *l, Exp *r = nullptr) : op(o), lhs(l), rhs(r) {}

        void print(std::ofstream &ofd);

        std::string op_real();
    };

    class Cond : public Node {
    public:
        Exp *exp;

        Cond() {}

        Cond(Exp *e) : exp(e) {}

        void print(std::ofstream &ofd);
    };

    class FuncCall : public Node {
    public:
        std::string name;
        std::vector<Exp *> params;

        FuncCall() {}

        FuncCall(std::string n) : name(std::move(n)) {}

        void print(std::ofstream &ofd);
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

        void print(std::ofstream &ofd);
    };

    class Function : public Node {
    public:
        enum class Type {
            VOID,
            INT
        } type;
        std::string name;
        std::vector<FuncFParam *> params;
        Block *block;

        Function() {}

        Function(Type t, std::string n, Block *b) : type(t), name(n), block(b) {}

        Function(Type t, std::string n, std::vector<FuncFParam *> &ps, Block *b) {
            Function(t, n, b);
            std::swap(params, ps);
        }

        void print(std::ofstream &ofd);
    };

    class Stmt : public Node {
    };

    // Code block is a statement itself.
    class Block : public Stmt {
    public:
        // Decl or Stmt. Runtime type checking required.
        // Using two arrays isn't possible as we need to preserve the order.
        std::vector<Node *> entries;

        void append_nodes(std::vector<ast::Node *> entries);

        void print(std::ofstream &ofd);
    };

    class AssignmentStmt : public Stmt {
    public:
        LVal *lval;
        Exp *exp;

        AssignmentStmt(LVal *l = nullptr, Exp *e = nullptr) : lval(l), exp(e) {}

        void print(std::ofstream &ofd);
    };

    class EvalStmt : public Stmt {
    public:
        Exp *exp;

        EvalStmt(Exp *e = nullptr) : exp(e) {}

        void print(std::ofstream &ofd);
    };

    class IfStmt : public Stmt {
    public:
        Cond *cond;
        Stmt *true_block;
        Stmt *false_block;

        IfStmt(Cond *c = nullptr, Stmt *t = nullptr, Stmt *f = nullptr) : cond(c), true_block(t), false_block(f) {}

        void print(std::ofstream &ofd);
    };

    class WhileStmt : public Stmt {
    public:
        Cond *cond;
        Stmt *block;

        WhileStmt(Cond *c = nullptr, Stmt *b = nullptr) : cond(c), block(b) {}

        void print(std::ofstream &ofd);
    };

    class BreakStmt : public Stmt {
        void print(std::ofstream &ofd);
    };

    class ContinueStmt : public Stmt {
        void print(std::ofstream &ofd);
    };

    class ReturnStmt : public Stmt {
    public:
        Exp *ret;

        ReturnStmt(Exp *e = nullptr) : ret(e) {}

        void print(std::ofstream &ofd);
    };
}
#endif //TINBACCC_AST_H
