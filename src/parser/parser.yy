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
using std::string;
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
  END "end of file"
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
;

%start CompUnit;

%%

CompUnit: CompUnit Decl {}
        | CompUnit FuncDef {}
        | Decl {}
        | FuncDef {}
        ;

Decl: ConstDecl ";" {}
    | VarDecl ";" {}
    ;

BType: "int";

ConstDecl: "const" BType ConstDef {}
         | ConstDecl "," ConstDef {}
    ;

VarDecl: BType VarDef {}
       | VarDecl "," VarDef {}
       ;

VarDef: DefSingleElem
      | DefArray
      ;

DefSingleElem: IDENT "=" InitVal {}
             | IDENT {}
             ;

DefArray: ArrayBody "=" InitValArray {}
        | ArrayBody {}
        ;

ArrayBody: ArrayBody "[" Exp "]" {}
         | IDENT "[" Exp "]" {}
         ;

ConstDef: ConstDefSingleElem
        | ConstDefArray
        ;

ConstDefSingleElem: IDENT "=" InitVal {}
                  | IDENT {}
                  ;
ConstDefArray: ArrayBody "=" InitValArray {}
             ;

InitVal: AddExp;

InitValArray: "{" InitValArrayInner "}" {}
            | "{" "}" {}
            ;

InitValArrayInner: InitValArrayInner "," InitValArray {}
                 | InitValArrayInner "," InitVal {}
                 | InitValArray {}
                 | InitVal {}
                 ;

Exp: AddExp;

LOrExp: LOrExp "||" LAndExp {}
      | LAndExp {}
      ;

LAndExp: EqExp {}
       | LAndExp "&&" EqExp {}
       ;

EqExp: RelExp {}
     | RelExp "==" RelExp {}
     | RelExp "!=" RelExp {}
     ;

RelExp: AddExp {}
      | RelExp RelOp AddExp {}
      ;

AddExp: MulExp {}
      | AddExp AddOp MulExp {}
      ;

MulExp: UnaryExp {}
      | MulExp MulOp UnaryExp {}
      ;

UnaryExp: PrimaryExp {}
        | FuncCall {}
        | UnaryOp UnaryExp {}
        ;

FuncCall: IDENT "(" FuncRParams ")" {}
        | IDENT "(" ")" {}
        ;

PrimaryExp: "(" Exp ")" {}
          | LVal {}
          | Number {}
          ;

ArrayItem: "[" Exp "]";

LVal: LVal ArrayItem
    | IDENT
    ;

FuncDef: "void" IDENT "(" FuncFParams ")" Block {}
       | "void" IDENT "(" ")" Block {}
       | BType IDENT "(" FuncFParams ")" Block {}
       | BType IDENT "(" ")" Block {}
       ;

FuncFParams: FuncFSingleParam
           | FuncFParamArray
           ;

FuncRParams: FuncRParams "," AddExp {}
           | AddExp {}
           ;

FuncFSingleParam: BType IDENT {};

FuncFParamArray: FuncFSingleParam "[" "]" {}
               | FuncFParamArray "[" Exp "]" {}
               ;

Block: "{" "}" {}
     | "{" BlockItems "}" {}
     ;

BlockItems: BlockItem {}
          | BlockItems BlockItem {}
          ;

BlockItem: Decl
         | Stmt
         ;

Stmt: LVal "=" Exp ";"
    | Exp ";"
    | ";"
    | Block
    | IfStmt
    | WhileStmt
    | BreakStmt
    | ReturnStmt
    | ContinueStmt
    ;

IfStmt: "if" "(" Cond ")" Stmt "else" Stmt {}
      | "if" "(" Cond ")" Stmt {} %prec "then"
      ;

ReturnStmt: "return" Exp ";"
          | "return" ";"
          ;

WhileStmt: "while" "(" Cond ")" Stmt {};

BreakStmt: "break" ";" {};

ContinueStmt: "continue" ";" {}

Cond: LOrExp;

Number: INTCONST {}

AddOp: "+"
     | "-"
     ;

MulOp: "*"
     | "/"
     | "%"
     ;

UnaryOp: "+"
       | "-"
       | "!"
       ;

RelOp: ">"
     | ">="
     | "<"
     | "<="
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
