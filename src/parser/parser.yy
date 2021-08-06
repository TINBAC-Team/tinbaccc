%skeleton "lalr1.cc" /* -*- C++ -*- */
%require "3.0.4"
%defines
%define api.parser.class {tcc_sy_parser}
%define api.token.constructor
%define api.value.type variant
%define parse.assert
%code requires
{
# include <string>
# include <vector>
# include <algorithm>
# include <ast/ast.h>
using std::string;
using std::vector;
class tcc_sy_driver;
}
// The parsing context.
%param { tcc_sy_driver & driver }
%locations
%initial-action
{
  // Initialize the initial location.
  @$.begin.filename = @$.end.filename = &driver.file;
};
%define parse.trace
%define parse.error verbose
%code
{
# include "parser/tcc-sy-driver.hh"
}
%nonassoc "then"
%nonassoc "else"
%define api.token.prefix {TOK_}
%token <int> INTCONST "intconst"
%token <string> IDENTIFIER "identifier"
%token
  END 0 "end of file"
  CONST "const"
  INT "int"
  COMMA ","
  SEMI ";"
  LSQUARE "["
  RSQUARE "]"
  LBRACE "("
  RBRACE ")"
  ASSIGN "="
  LBBRACE "{"
  RBBRACE "}"
  VOID "void"
  IF "if"
  ELSE "else"
  WHILE "while"
  BREAK "break"
  CONTINUE "continue"
  RETURN "return"
  PLUS "+"
  MINUS "-"
  MUL "*"
  DIVIDE "/"
  MOD "%"
  GREATER ">"
  GREATEREQ ">="
  SMALLER "<"
  SMALLEREQ "<="
  EQUAL "=="
  NOTEQUAL "!="
  AND "&&"
  OR "||"
  NOT "!"

  SINGLECOMMENT "//"

;

%type <vector<ast::Decl *>> Decl ConstDecl VarDecl
%type <ast::Decl::VarType> BType
%type <ast::Decl *> VarDef DefSingleElem DefArray ArrayBody ConstDef ConstDefSingleElem ConstDefArray
%type <ast::InitVal *> InitVal InitValArray InitValArrayInner
%type <ast::Exp *> Exp LOrExp LAndExp EqExp RelExp AddExp MulExp UnaryExp PrimaryExp ArrayItem
%type <ast::FuncCall *> FuncCall
%type <ast::LVal *> LVal
%type <ast::Function *> FuncDef
%type <vector<ast::FuncFParam *>> FuncFParams
%type <ast::FuncFParam *> FuncFParam FuncFSingleParam FuncFParamArray
%type <vector<ast::Exp *>> FuncRParams
%type <ast::Block *> Block BlockItems
%type <vector<ast::Node *>> BlockItem
%type <ast::Stmt *> Stmt
%type <ast::IfStmt *> IfStmt
%type <ast::ReturnStmt *> ReturnStmt
%type <ast::WhileStmt *>  WhileStmt
%type <ast::BreakStmt *> BreakStmt
%type <ast::ContinueStmt *> ContinueStmt
%type <ast::Cond *> Cond
%type <ast::Exp *> Number
%type <ast::Exp::Op> AddOp MulOp UnaryOp RelOp
%type <string> IDENT

%start CompUnit;

%%

CompUnit: CompUnit Decl { driver.comp_unit->append_decls($2); }
        | CompUnit FuncDef { driver.comp_unit->append_function($2); }
        | Decl { driver.comp_unit->append_decls($1); }
        | FuncDef { driver.comp_unit->append_function($1); }
        ;

Decl: ConstDecl ";" { std::swap($$, $1); }
    | VarDecl ";" { std::swap($$, $1); }
    ;

BType: "int" { $$ = ast::Decl::VarType::INT; };

ConstDecl: "const" BType ConstDef { $3->is_const = true; $$.emplace_back($3); }
         | ConstDecl "," ConstDef { std::swap($$, $1); $3->is_const = true; $$.emplace_back($3); }
    ;

VarDecl: BType VarDef { $$.emplace_back($2); }
       | VarDecl "," VarDef { std::swap($$, $1); $$.emplace_back($3); }
       ;

VarDef: DefSingleElem { $$ = $1; }
      | DefArray { $$ = $1; }
      ;

DefSingleElem: IDENT "=" InitVal { $$ = new ast::Decl($1, $3); }
             | IDENT { $$ = new ast::Decl($1); }
             ;

DefArray: ArrayBody "=" InitValArray { $$ = $1; $$->initval = $3; }
        | ArrayBody { $$ = $1; }
        ;

ArrayBody: ArrayBody "[" Exp "]" { $$ = $1; $$->add_dim($3); }
         | IDENT "[" Exp "]" { $$ = new ast::Decl($1); $$->add_dim($3); }
         ;

ConstDef: ConstDefSingleElem { $$ = $1; }
        | ConstDefArray { $$ = $1; }
        ;

ConstDefSingleElem: IDENT "=" InitVal { $$ = new ast::Decl($1, $3); }
                  ;
ConstDefArray: ArrayBody "=" InitValArray { $$ = $1; $$->initval = $3; }
             ;

InitVal: AddExp { $$ = new ast::InitVal($1); };

InitValArray: "{" InitValArrayInner "}" { $$ = $2; }
            | "{" "}" { $$ = new ast::InitVal(); }
            ;

InitValArrayInner: InitValArrayInner "," InitValArray { $$ = $1; $$->append_entry($3); }
                 | InitValArrayInner "," InitVal { $$ = $1; $$->append_entry($3); }
                 | InitValArray { $$ = new ast::InitVal($1); }
                 | InitVal { $$ = new ast::InitVal($1); }
                 ;

Exp: AddExp { $$ = $1; };

LOrExp: LOrExp "||" LAndExp { $$ = new ast::Exp(ast::Exp::Op::LOGIC_OR, $1, $3); }
      | LAndExp { $$ = $1; }
      ;

LAndExp: EqExp { $$ = $1; }
       | LAndExp "&&" EqExp { $$ = new ast::Exp(ast::Exp::Op::LOGIC_AND, $1, $3); }
       ;

EqExp: RelExp { $$ = $1; }
     | RelExp "==" RelExp { $$ = new ast::Exp(ast::Exp::Op::EQ, $1, $3); }
     | RelExp "!=" RelExp { $$ = new ast::Exp(ast::Exp::Op::INEQ, $1, $3); }
     ;

RelExp: AddExp { $$ = $1; }
      | RelExp RelOp AddExp { $$ = new ast::Exp($2, $1, $3); }
      ;

AddExp: MulExp { $$ = $1; }
      | AddExp AddOp MulExp { $$ = new ast::Exp($2, $1, $3); }
      ;

MulExp: UnaryExp { $$ = $1; }
      | MulExp MulOp UnaryExp { $$ = new ast::Exp($2, $1, $3); }
      ;

UnaryExp: PrimaryExp { $$ = $1; }
        | FuncCall { $$ = new ast::Exp($1); }
        | UnaryOp UnaryExp { $$ = new ast::Exp($1, $2); }
        ;

FuncCall: IDENT "(" FuncRParams ")" { $$ = new ast::FuncCall($1, @$.end.line); std::swap($$->params, $3); }
        | IDENT "(" ")" { $$ = new ast::FuncCall($1, @$.end.line); }
        ;

PrimaryExp: "(" Exp ")" { $$ = $2; }
          | LVal { $$ = new ast::Exp($1); }
          | Number { $$ = $1; }
          ;

ArrayItem: "[" Exp "]" { $$ = $2; };

LVal: LVal ArrayItem { $$ = $1; $$->add_dim($2); }
    | IDENT { $$ = new ast::LVal($1); }
    ;

FuncDef: "void" IDENT "(" FuncFParams ")" Block { $$ = new ast::Function(ast::Function::Type::VOID, $2, $4, $6); }
       | "void" IDENT "(" ")" Block { $$ = new ast::Function(ast::Function::Type::VOID, $2, $5); }
       | BType IDENT "(" FuncFParams ")" Block { $$ = new ast::Function(ast::Function::Type::INT, $2, $4, $6); }
       | BType IDENT "(" ")" Block { $$ = new ast::Function(ast::Function::Type::INT, $2, $5); }
       ;

FuncFParams: FuncFParams "," FuncFParam { std::swap($$, $1); $$.emplace_back($3); }
           | FuncFParam { $$.emplace_back($1); }
           ;

FuncFParam: FuncFSingleParam { $$ = $1; }
           | FuncFParamArray { $$ = $1; }
           ;

FuncRParams: FuncRParams "," AddExp { std::swap($$, $1); $$.emplace_back($3); }
           | AddExp { $$.emplace_back($1); }
           ;

FuncFSingleParam: BType IDENT { $$ = new ast::FuncFParam($1, $2); };

FuncFParamArray: FuncFSingleParam "[" "]" { $$ = $1; $$->signature->add_dim(0); }
               | FuncFParamArray "[" Exp "]" { $$ = $1; $$->signature->add_dim($3); }
               ;

Block: "{" "}" { $$ = new ast::Block(); }
     | "{" BlockItems "}" { $$ = $2; }
     ;

BlockItems: BlockItem { $$ = new ast::Block(); $$->append_nodes($1); }
          | BlockItems BlockItem { $$ = $1; $$->append_nodes($2); }
          ;

BlockItem: Decl { $$.insert(std::end($$), std::begin($1), std::end($1)); }
         | Stmt { $$.emplace_back($1); }
         ;

Stmt: LVal "=" Exp ";" { $$ = new ast::AssignmentStmt($1, $3); }
    | Exp ";" { $$ = new ast::EvalStmt($1); }
    | ";" { $$ = new ast::Block(); }
    | Block { $$ = $1; }
    | IfStmt { $$ = $1; }
    | WhileStmt { $$ = $1; }
    | BreakStmt { $$ = $1; }
    | ReturnStmt { $$ = $1; }
    | ContinueStmt { $$ = $1; }
    ;

IfStmt: "if" "(" Cond ")" Stmt "else" Stmt { $$ = new ast::IfStmt($3, $5, $7); }
      | "if" "(" Cond ")" Stmt { $$ = new ast::IfStmt($3, $5); } %prec "then"
      ;

ReturnStmt: "return" Exp ";" { $$ = new ast::ReturnStmt($2); }
          | "return" ";" { $$ = new ast::ReturnStmt; }
          ;

WhileStmt: "while" "(" Cond ")" Stmt { $$ = new ast::WhileStmt($3, $5); };

BreakStmt: "break" ";" { $$ = new ast::BreakStmt; };

ContinueStmt: "continue" ";" { $$ = new ast::ContinueStmt; }

Cond: LOrExp { $$ = new ast::Cond($1); };

Number: INTCONST { $$ = new ast::Exp($1); }

AddOp: "+" { $$ = ast::Exp::Op::PLUS; }
     | "-" { $$ = ast::Exp::Op::MINUS; }
     ;

MulOp: "*" { $$ = ast::Exp::Op::MUL; }
     | "/" { $$ = ast::Exp::Op::DIV; }
     | "%" { $$ = ast::Exp::Op::MOD; }
     ;

UnaryOp: "+" { $$ = ast::Exp::Op::UNARY_PLUS; }
       | "-" { $$ = ast::Exp::Op::UNARY_MINUS; }
       | "!" { $$ = ast::Exp::Op::LOGIC_NOT; }
       ;

RelOp: ">" { $$ = ast::Exp::Op::GREATER_THAN; }
     | ">=" { $$ = ast::Exp::Op::GREATER_EQ; }
     | "<" { $$ = ast::Exp::Op::LESS_THAN; }
     | "<=" { $$ = ast::Exp::Op::LESS_EQ; }
     ;

IDENT: IDENTIFIER;

%left "+" "-";
%left "*" "/";

%%
void
yy::tcc_sy_parser::error (const location_type& l,
                          const std::string& m)
{
  driver.error (l, m);
}
