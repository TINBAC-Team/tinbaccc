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
class tcc_parser_driver;
}
// The parsing context.
%param { tcc_parser_driver& driver }
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
# include <parser/driver.h>
}
%define api.token.prefix {TOK_}
%token
  END  0  "end of file"
  ASSIGN  ":="
  MINUS   "-"
  PLUS    "+"
  STAR    "*"
  SLASH   "/"
  LPAREN  "("
  RPAREN  ")"
;
%token <std::string> IDENTIFIER "identifier"
%token <int> NUMBER "number"
%type  <int> exp
%printer { yyoutput << $$; } <*>;
%%
%start unit;
unit: assignments exp  {  };

assignments:
  %empty                 {}
| assignments assignment {};

assignment:
  "identifier" ":=" exp {  };

%left "+" "-";
%left "*" "/";
exp:
  exp "+" exp   { $$ = $1 + $3; }
| exp "-" exp   { $$ = $1 - $3; }
| exp "*" exp   { $$ = $1 * $3; }
| exp "/" exp   { $$ = $1 / $3; }
| "(" exp ")"   { std::swap ($$, $2); }
| "identifier"  { $$ = 0; }
| "number"      { std::swap ($$, $1); };
%%
void
yy::tcc_sy_parser::error (const location_type& l,
                          const std::string& m)
{
  driver.error (l, m);
}
