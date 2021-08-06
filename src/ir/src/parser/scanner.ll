%{ /* -*- C++ -*- */
# include <cerrno>
# include <climits>
# include <cstdlib>
# include <string>
# include "parser/tcc-sy-driver.hh"
# include "tcc-sy-parser.hh"

# undef yywrap
# define yywrap() 1

// The location of the current token.
static yy::location loc;
%}
%option noyywrap nounput batch debug noinput
%x BLOCK_COMMENT
%x SINGLE_COMMENT
ident    [a-zA-Z_][a-zA-Z0-9_]*
HexConst "0"[xX][0-9a-fA-F]+
OctConst "0"[0-9]*
IntConst [1-9][0-9]*
blank [ \t]

%{
  // Code run each time a pattern is matched.
  # define YY_USER_ACTION  loc.columns (yyleng);
%}

%%

%{
  // Code run each time yylex is called.
  loc.step ();
%}

{blank}+   loc.columns(); loc.step ();
[\n]+      loc.lines (yyleng); loc.step ();
"const"    return yy::tcc_sy_parser::make_CONST(loc);
"int"      return yy::tcc_sy_parser::make_INT(loc);
","        return yy::tcc_sy_parser::make_COMMA(loc);
";"        return yy::tcc_sy_parser::make_SEMI(loc);
"["        return yy::tcc_sy_parser::make_LSQUARE(loc);
"]"        return yy::tcc_sy_parser::make_RSQUARE(loc);
"("        return yy::tcc_sy_parser::make_LBRACE(loc);
")"        return yy::tcc_sy_parser::make_RBRACE(loc);
"{"        return yy::tcc_sy_parser::make_LBBRACE(loc);
"}"        return yy::tcc_sy_parser::make_RBBRACE(loc);
"void"      return yy::tcc_sy_parser::make_VOID(loc);
"if"      return yy::tcc_sy_parser::make_IF(loc);
"else"      return yy::tcc_sy_parser::make_ELSE(loc);
"while"      return yy::tcc_sy_parser::make_WHILE(loc);
"break"      return yy::tcc_sy_parser::make_BREAK(loc);
"continue"      return yy::tcc_sy_parser::make_CONTINUE(loc);
"return"      return yy::tcc_sy_parser::make_RETURN(loc);

"+"        return yy::tcc_sy_parser::make_PLUS(loc);
"-"        return yy::tcc_sy_parser::make_MINUS(loc);
"*"        return yy::tcc_sy_parser::make_MUL(loc);
"/"        return yy::tcc_sy_parser::make_DIVIDE(loc);
"%"        return yy::tcc_sy_parser::make_MOD(loc);
">"        return yy::tcc_sy_parser::make_GREATER(loc);
">="        return yy::tcc_sy_parser::make_GREATEREQ(loc);
"<"        return yy::tcc_sy_parser::make_SMALLER(loc);
"<="        return yy::tcc_sy_parser::make_SMALLEREQ(loc);
"=="        return yy::tcc_sy_parser::make_EQUAL(loc);
"="        return yy::tcc_sy_parser::make_ASSIGN(loc);
"!="        return yy::tcc_sy_parser::make_NOTEQUAL(loc);


"&&"        return yy::tcc_sy_parser::make_AND(loc);
"||"        return yy::tcc_sy_parser::make_OR(loc);
"!"        return yy::tcc_sy_parser::make_NOT(loc);

"/*"            { BEGIN(BLOCK_COMMENT); }
<BLOCK_COMMENT>"*/" { BEGIN(INITIAL); }
<BLOCK_COMMENT>.    { }
<BLOCK_COMMENT>\n   { loc.lines (yyleng); loc.step ();}

"//"                { BEGIN(SINGLE_COMMENT); }
<SINGLE_COMMENT>.        { }
<SINGLE_COMMENT>\n  { loc.lines (yyleng); loc.step(); BEGIN(INITIAL);  }


{IntConst}   {
  long n = strtol (yytext, NULL, 10);
  return yy::tcc_sy_parser::make_INTCONST(n, loc);
}

{HexConst}   {
  long n = strtol (yytext, NULL, 0);
  return yy::tcc_sy_parser::make_INTCONST(n, loc);
}

{OctConst}   {
  long n = strtol (yytext, NULL, 0);
  return yy::tcc_sy_parser::make_INTCONST(n, loc);
}

{ident}       return yy::tcc_sy_parser::make_IDENTIFIER(yytext, loc);
.          driver.error (loc, "invalid character");
<<EOF>>    return yy::tcc_sy_parser::make_END(loc);
%%

void
tcc_sy_driver::scan_begin ()
{
  yy_flex_debug = trace_scanning;
  if (file.empty () || file == "-")
    yyin = stdin;
  else if (!(yyin = fopen (file.c_str (), "r")))
    {
      error ("cannot open " + file + ": " + strerror(errno));
      exit (EXIT_FAILURE);
    }
}



void
tcc_sy_driver::scan_end ()
{
  fclose (yyin);
}

