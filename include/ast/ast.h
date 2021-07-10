#ifndef TINBACCC_AST_H
#define TINBACCC_AST_H

#include <string>
#include <vector>

namespace ast {
    class Node {
    public:
        virtual ~Node() {}
    };

    class CompUnit : public Node {
    public:
        // Decl or Function. Runtime type checking required.
        // Using two arrays isn't possible as we need to preserve the order.
        std::vector<Node *> entries;
    };

    class Exp;

    class InitVal : public Node {
        Exp *exp; // invalid when it's nullptr
        std::vector<InitVal *> vals;
    };

    class LVal : public Node {
        std::string name;
        std::vector<Exp *> array_dims;
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
        bool is_array() {
            return array_dims.size();
        }
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
    };

    class Cond : public Node {
    public:
        Exp *exp;
    };

    class FuncCall : public Node {
        std::string name;
        std::vector<Exp *> params;
    };

    class Block;

    class FuncFParam : public Node {
    public:
        Decl::VarType type;
        LVal *signature;
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
    };

    class Stmt : public Node {
    };

    // Code block is a statement itself.
    class Block : public Stmt {
    public:
        // Decl or Stmt. Runtime type checking required.
        // Using two arrays isn't possible as we need to preserve the order.
        std::vector<Node *> entries;
    };

    class AssignmentStmt : public Stmt {
    public:
        LVal *lval;
        Exp *exp;
    };

    class EvalStmt : public Stmt {
    public:
        Exp *exp;
    };

    class IfStmt : public Stmt {
    public:
        Cond *cond;
        Stmt *true_block;
        Stmt *false_block;
    };

    class WhileStmt : public Stmt {
    public:
        Cond *cond;
        Stmt *block;
    };

    class BreakStmt : public Stmt {
    };

    class ContinueStmt : public Stmt {
    };

    class ReturnStmt : public Stmt {
        Exp *ret;
    };
}
#endif //TINBACCC_AST_H
