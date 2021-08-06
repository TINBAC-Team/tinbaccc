// A Bison parser, made by GNU Bison 3.7.4.

// Skeleton implementation for Bison LALR(1) parsers in C++

// Copyright (C) 2002-2015, 2018-2020 Free Software Foundation, Inc.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// As a special exception, you may create a larger work that contains
// part or all of the Bison parser skeleton and distribute that work
// under terms of your choice, so long as that work isn't itself a
// parser generator using the skeleton or a modified version thereof
// as a parser skeleton.  Alternatively, if you modify or redistribute
// the parser skeleton itself, you may (at your option) remove this
// special exception, which will cause the skeleton and the resulting
// Bison output files to be licensed under the GNU General Public
// License without this special exception.

// This special exception was added by the Free Software Foundation in
// version 2.2 of Bison.

// DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
// especially those whose name start with YY_ or yy_.  They are
// private implementation details that can be changed or removed.





#include "tcc-sy-parser.hh"


// Unqualified %code blocks.
#line 29 "src/parser/parser.yy"

# include "parser/tcc-sy-driver.hh"

#line 50 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"


#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> // FIXME: INFRINGES ON USER NAME SPACE.
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif


// Whether we are compiled with exception support.
#ifndef YY_EXCEPTIONS
# if defined __GNUC__ && !defined __EXCEPTIONS
#  define YY_EXCEPTIONS 0
# else
#  define YY_EXCEPTIONS 1
# endif
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K].location)
/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

# ifndef YYLLOC_DEFAULT
#  define YYLLOC_DEFAULT(Current, Rhs, N)                               \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).begin  = YYRHSLOC (Rhs, 1).begin;                   \
          (Current).end    = YYRHSLOC (Rhs, N).end;                     \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).begin = (Current).end = YYRHSLOC (Rhs, 0).end;      \
        }                                                               \
    while (false)
# endif


// Enable debugging if requested.
#if YYDEBUG

// A pseudo ostream that takes yydebug_ into account.
# define YYCDEBUG if (yydebug_) (*yycdebug_)

# define YY_SYMBOL_PRINT(Title, Symbol)         \
  do {                                          \
    if (yydebug_)                               \
    {                                           \
      *yycdebug_ << Title << ' ';               \
      yy_print_ (*yycdebug_, Symbol);           \
      *yycdebug_ << '\n';                       \
    }                                           \
  } while (false)

# define YY_REDUCE_PRINT(Rule)          \
  do {                                  \
    if (yydebug_)                       \
      yy_reduce_print_ (Rule);          \
  } while (false)

# define YY_STACK_PRINT()               \
  do {                                  \
    if (yydebug_)                       \
      yy_stack_print_ ();                \
  } while (false)

#else // !YYDEBUG

# define YYCDEBUG if (false) std::cerr
# define YY_SYMBOL_PRINT(Title, Symbol)  YYUSE (Symbol)
# define YY_REDUCE_PRINT(Rule)           static_cast<void> (0)
# define YY_STACK_PRINT()                static_cast<void> (0)

#endif // !YYDEBUG

#define yyerrok         (yyerrstatus_ = 0)
#define yyclearin       (yyla.clear ())

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYRECOVERING()  (!!yyerrstatus_)

namespace yy {
#line 142 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"

  /// Build a parser object.
  tcc_sy_parser::tcc_sy_parser (tcc_sy_driver & driver_yyarg)
#if YYDEBUG
    : yydebug_ (false),
      yycdebug_ (&std::cerr),
#else
    :
#endif
      driver (driver_yyarg)
  {}

  tcc_sy_parser::~tcc_sy_parser ()
  {}

  tcc_sy_parser::syntax_error::~syntax_error () YY_NOEXCEPT YY_NOTHROW
  {}

  /*---------------.
  | symbol kinds.  |
  `---------------*/



  // by_state.
  tcc_sy_parser::by_state::by_state () YY_NOEXCEPT
    : state (empty_state)
  {}

  tcc_sy_parser::by_state::by_state (const by_state& that) YY_NOEXCEPT
    : state (that.state)
  {}

  void
  tcc_sy_parser::by_state::clear () YY_NOEXCEPT
  {
    state = empty_state;
  }

  void
  tcc_sy_parser::by_state::move (by_state& that)
  {
    state = that.state;
    that.clear ();
  }

  tcc_sy_parser::by_state::by_state (state_type s) YY_NOEXCEPT
    : state (s)
  {}

  tcc_sy_parser::symbol_kind_type
  tcc_sy_parser::by_state::kind () const YY_NOEXCEPT
  {
    if (state == empty_state)
      return symbol_kind::S_YYEMPTY;
    else
      return YY_CAST (symbol_kind_type, yystos_[+state]);
  }

  tcc_sy_parser::stack_symbol_type::stack_symbol_type ()
  {}

  tcc_sy_parser::stack_symbol_type::stack_symbol_type (YY_RVREF (stack_symbol_type) that)
    : super_type (YY_MOVE (that.state), YY_MOVE (that.location))
  {
    switch (that.kind ())
    {
      case symbol_kind::S_Block: // Block
      case symbol_kind::S_BlockItems: // BlockItems
        value.YY_MOVE_OR_COPY< ast::Block * > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_BreakStmt: // BreakStmt
        value.YY_MOVE_OR_COPY< ast::BreakStmt * > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_Cond: // Cond
        value.YY_MOVE_OR_COPY< ast::Cond * > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_ContinueStmt: // ContinueStmt
        value.YY_MOVE_OR_COPY< ast::ContinueStmt * > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_VarDef: // VarDef
      case symbol_kind::S_DefSingleElem: // DefSingleElem
      case symbol_kind::S_DefArray: // DefArray
      case symbol_kind::S_ArrayBody: // ArrayBody
      case symbol_kind::S_ConstDef: // ConstDef
      case symbol_kind::S_ConstDefSingleElem: // ConstDefSingleElem
      case symbol_kind::S_ConstDefArray: // ConstDefArray
        value.YY_MOVE_OR_COPY< ast::Decl * > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_BType: // BType
        value.YY_MOVE_OR_COPY< ast::Decl::VarType > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_Exp: // Exp
      case symbol_kind::S_LOrExp: // LOrExp
      case symbol_kind::S_LAndExp: // LAndExp
      case symbol_kind::S_EqExp: // EqExp
      case symbol_kind::S_RelExp: // RelExp
      case symbol_kind::S_AddExp: // AddExp
      case symbol_kind::S_MulExp: // MulExp
      case symbol_kind::S_UnaryExp: // UnaryExp
      case symbol_kind::S_PrimaryExp: // PrimaryExp
      case symbol_kind::S_ArrayItem: // ArrayItem
      case symbol_kind::S_Number: // Number
        value.YY_MOVE_OR_COPY< ast::Exp * > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_AddOp: // AddOp
      case symbol_kind::S_MulOp: // MulOp
      case symbol_kind::S_UnaryOp: // UnaryOp
      case symbol_kind::S_RelOp: // RelOp
        value.YY_MOVE_OR_COPY< ast::Exp::Op > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_FuncCall: // FuncCall
        value.YY_MOVE_OR_COPY< ast::FuncCall * > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_FuncFParam: // FuncFParam
      case symbol_kind::S_FuncFSingleParam: // FuncFSingleParam
      case symbol_kind::S_FuncFParamArray: // FuncFParamArray
        value.YY_MOVE_OR_COPY< ast::FuncFParam * > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_FuncDef: // FuncDef
        value.YY_MOVE_OR_COPY< ast::Function * > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_IfStmt: // IfStmt
        value.YY_MOVE_OR_COPY< ast::IfStmt * > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_InitVal: // InitVal
      case symbol_kind::S_InitValArray: // InitValArray
      case symbol_kind::S_InitValArrayInner: // InitValArrayInner
        value.YY_MOVE_OR_COPY< ast::InitVal * > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_LVal: // LVal
        value.YY_MOVE_OR_COPY< ast::LVal * > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_ReturnStmt: // ReturnStmt
        value.YY_MOVE_OR_COPY< ast::ReturnStmt * > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_Stmt: // Stmt
        value.YY_MOVE_OR_COPY< ast::Stmt * > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_WhileStmt: // WhileStmt
        value.YY_MOVE_OR_COPY< ast::WhileStmt * > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_INTCONST: // "intconst"
        value.YY_MOVE_OR_COPY< int > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_IDENT: // IDENT
        value.YY_MOVE_OR_COPY< string > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_Decl: // Decl
      case symbol_kind::S_ConstDecl: // ConstDecl
      case symbol_kind::S_VarDecl: // VarDecl
        value.YY_MOVE_OR_COPY< vector<ast::Decl *> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_FuncRParams: // FuncRParams
        value.YY_MOVE_OR_COPY< vector<ast::Exp *> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_FuncFParams: // FuncFParams
        value.YY_MOVE_OR_COPY< vector<ast::FuncFParam *> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_BlockItem: // BlockItem
        value.YY_MOVE_OR_COPY< vector<ast::Node *> > (YY_MOVE (that.value));
        break;

      default:
        break;
    }

#if 201103L <= YY_CPLUSPLUS
    // that is emptied.
    that.state = empty_state;
#endif
  }

  tcc_sy_parser::stack_symbol_type::stack_symbol_type (state_type s, YY_MOVE_REF (symbol_type) that)
    : super_type (s, YY_MOVE (that.location))
  {
    switch (that.kind ())
    {
      case symbol_kind::S_Block: // Block
      case symbol_kind::S_BlockItems: // BlockItems
        value.move< ast::Block * > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_BreakStmt: // BreakStmt
        value.move< ast::BreakStmt * > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_Cond: // Cond
        value.move< ast::Cond * > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_ContinueStmt: // ContinueStmt
        value.move< ast::ContinueStmt * > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_VarDef: // VarDef
      case symbol_kind::S_DefSingleElem: // DefSingleElem
      case symbol_kind::S_DefArray: // DefArray
      case symbol_kind::S_ArrayBody: // ArrayBody
      case symbol_kind::S_ConstDef: // ConstDef
      case symbol_kind::S_ConstDefSingleElem: // ConstDefSingleElem
      case symbol_kind::S_ConstDefArray: // ConstDefArray
        value.move< ast::Decl * > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_BType: // BType
        value.move< ast::Decl::VarType > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_Exp: // Exp
      case symbol_kind::S_LOrExp: // LOrExp
      case symbol_kind::S_LAndExp: // LAndExp
      case symbol_kind::S_EqExp: // EqExp
      case symbol_kind::S_RelExp: // RelExp
      case symbol_kind::S_AddExp: // AddExp
      case symbol_kind::S_MulExp: // MulExp
      case symbol_kind::S_UnaryExp: // UnaryExp
      case symbol_kind::S_PrimaryExp: // PrimaryExp
      case symbol_kind::S_ArrayItem: // ArrayItem
      case symbol_kind::S_Number: // Number
        value.move< ast::Exp * > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_AddOp: // AddOp
      case symbol_kind::S_MulOp: // MulOp
      case symbol_kind::S_UnaryOp: // UnaryOp
      case symbol_kind::S_RelOp: // RelOp
        value.move< ast::Exp::Op > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_FuncCall: // FuncCall
        value.move< ast::FuncCall * > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_FuncFParam: // FuncFParam
      case symbol_kind::S_FuncFSingleParam: // FuncFSingleParam
      case symbol_kind::S_FuncFParamArray: // FuncFParamArray
        value.move< ast::FuncFParam * > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_FuncDef: // FuncDef
        value.move< ast::Function * > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_IfStmt: // IfStmt
        value.move< ast::IfStmt * > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_InitVal: // InitVal
      case symbol_kind::S_InitValArray: // InitValArray
      case symbol_kind::S_InitValArrayInner: // InitValArrayInner
        value.move< ast::InitVal * > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_LVal: // LVal
        value.move< ast::LVal * > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_ReturnStmt: // ReturnStmt
        value.move< ast::ReturnStmt * > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_Stmt: // Stmt
        value.move< ast::Stmt * > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_WhileStmt: // WhileStmt
        value.move< ast::WhileStmt * > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_INTCONST: // "intconst"
        value.move< int > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_IDENT: // IDENT
        value.move< string > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_Decl: // Decl
      case symbol_kind::S_ConstDecl: // ConstDecl
      case symbol_kind::S_VarDecl: // VarDecl
        value.move< vector<ast::Decl *> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_FuncRParams: // FuncRParams
        value.move< vector<ast::Exp *> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_FuncFParams: // FuncFParams
        value.move< vector<ast::FuncFParam *> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_BlockItem: // BlockItem
        value.move< vector<ast::Node *> > (YY_MOVE (that.value));
        break;

      default:
        break;
    }

    // that is emptied.
    that.kind_ = symbol_kind::S_YYEMPTY;
  }

#if YY_CPLUSPLUS < 201103L
  tcc_sy_parser::stack_symbol_type&
  tcc_sy_parser::stack_symbol_type::operator= (const stack_symbol_type& that)
  {
    state = that.state;
    switch (that.kind ())
    {
      case symbol_kind::S_Block: // Block
      case symbol_kind::S_BlockItems: // BlockItems
        value.copy< ast::Block * > (that.value);
        break;

      case symbol_kind::S_BreakStmt: // BreakStmt
        value.copy< ast::BreakStmt * > (that.value);
        break;

      case symbol_kind::S_Cond: // Cond
        value.copy< ast::Cond * > (that.value);
        break;

      case symbol_kind::S_ContinueStmt: // ContinueStmt
        value.copy< ast::ContinueStmt * > (that.value);
        break;

      case symbol_kind::S_VarDef: // VarDef
      case symbol_kind::S_DefSingleElem: // DefSingleElem
      case symbol_kind::S_DefArray: // DefArray
      case symbol_kind::S_ArrayBody: // ArrayBody
      case symbol_kind::S_ConstDef: // ConstDef
      case symbol_kind::S_ConstDefSingleElem: // ConstDefSingleElem
      case symbol_kind::S_ConstDefArray: // ConstDefArray
        value.copy< ast::Decl * > (that.value);
        break;

      case symbol_kind::S_BType: // BType
        value.copy< ast::Decl::VarType > (that.value);
        break;

      case symbol_kind::S_Exp: // Exp
      case symbol_kind::S_LOrExp: // LOrExp
      case symbol_kind::S_LAndExp: // LAndExp
      case symbol_kind::S_EqExp: // EqExp
      case symbol_kind::S_RelExp: // RelExp
      case symbol_kind::S_AddExp: // AddExp
      case symbol_kind::S_MulExp: // MulExp
      case symbol_kind::S_UnaryExp: // UnaryExp
      case symbol_kind::S_PrimaryExp: // PrimaryExp
      case symbol_kind::S_ArrayItem: // ArrayItem
      case symbol_kind::S_Number: // Number
        value.copy< ast::Exp * > (that.value);
        break;

      case symbol_kind::S_AddOp: // AddOp
      case symbol_kind::S_MulOp: // MulOp
      case symbol_kind::S_UnaryOp: // UnaryOp
      case symbol_kind::S_RelOp: // RelOp
        value.copy< ast::Exp::Op > (that.value);
        break;

      case symbol_kind::S_FuncCall: // FuncCall
        value.copy< ast::FuncCall * > (that.value);
        break;

      case symbol_kind::S_FuncFParam: // FuncFParam
      case symbol_kind::S_FuncFSingleParam: // FuncFSingleParam
      case symbol_kind::S_FuncFParamArray: // FuncFParamArray
        value.copy< ast::FuncFParam * > (that.value);
        break;

      case symbol_kind::S_FuncDef: // FuncDef
        value.copy< ast::Function * > (that.value);
        break;

      case symbol_kind::S_IfStmt: // IfStmt
        value.copy< ast::IfStmt * > (that.value);
        break;

      case symbol_kind::S_InitVal: // InitVal
      case symbol_kind::S_InitValArray: // InitValArray
      case symbol_kind::S_InitValArrayInner: // InitValArrayInner
        value.copy< ast::InitVal * > (that.value);
        break;

      case symbol_kind::S_LVal: // LVal
        value.copy< ast::LVal * > (that.value);
        break;

      case symbol_kind::S_ReturnStmt: // ReturnStmt
        value.copy< ast::ReturnStmt * > (that.value);
        break;

      case symbol_kind::S_Stmt: // Stmt
        value.copy< ast::Stmt * > (that.value);
        break;

      case symbol_kind::S_WhileStmt: // WhileStmt
        value.copy< ast::WhileStmt * > (that.value);
        break;

      case symbol_kind::S_INTCONST: // "intconst"
        value.copy< int > (that.value);
        break;

      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_IDENT: // IDENT
        value.copy< string > (that.value);
        break;

      case symbol_kind::S_Decl: // Decl
      case symbol_kind::S_ConstDecl: // ConstDecl
      case symbol_kind::S_VarDecl: // VarDecl
        value.copy< vector<ast::Decl *> > (that.value);
        break;

      case symbol_kind::S_FuncRParams: // FuncRParams
        value.copy< vector<ast::Exp *> > (that.value);
        break;

      case symbol_kind::S_FuncFParams: // FuncFParams
        value.copy< vector<ast::FuncFParam *> > (that.value);
        break;

      case symbol_kind::S_BlockItem: // BlockItem
        value.copy< vector<ast::Node *> > (that.value);
        break;

      default:
        break;
    }

    location = that.location;
    return *this;
  }

  tcc_sy_parser::stack_symbol_type&
  tcc_sy_parser::stack_symbol_type::operator= (stack_symbol_type& that)
  {
    state = that.state;
    switch (that.kind ())
    {
      case symbol_kind::S_Block: // Block
      case symbol_kind::S_BlockItems: // BlockItems
        value.move< ast::Block * > (that.value);
        break;

      case symbol_kind::S_BreakStmt: // BreakStmt
        value.move< ast::BreakStmt * > (that.value);
        break;

      case symbol_kind::S_Cond: // Cond
        value.move< ast::Cond * > (that.value);
        break;

      case symbol_kind::S_ContinueStmt: // ContinueStmt
        value.move< ast::ContinueStmt * > (that.value);
        break;

      case symbol_kind::S_VarDef: // VarDef
      case symbol_kind::S_DefSingleElem: // DefSingleElem
      case symbol_kind::S_DefArray: // DefArray
      case symbol_kind::S_ArrayBody: // ArrayBody
      case symbol_kind::S_ConstDef: // ConstDef
      case symbol_kind::S_ConstDefSingleElem: // ConstDefSingleElem
      case symbol_kind::S_ConstDefArray: // ConstDefArray
        value.move< ast::Decl * > (that.value);
        break;

      case symbol_kind::S_BType: // BType
        value.move< ast::Decl::VarType > (that.value);
        break;

      case symbol_kind::S_Exp: // Exp
      case symbol_kind::S_LOrExp: // LOrExp
      case symbol_kind::S_LAndExp: // LAndExp
      case symbol_kind::S_EqExp: // EqExp
      case symbol_kind::S_RelExp: // RelExp
      case symbol_kind::S_AddExp: // AddExp
      case symbol_kind::S_MulExp: // MulExp
      case symbol_kind::S_UnaryExp: // UnaryExp
      case symbol_kind::S_PrimaryExp: // PrimaryExp
      case symbol_kind::S_ArrayItem: // ArrayItem
      case symbol_kind::S_Number: // Number
        value.move< ast::Exp * > (that.value);
        break;

      case symbol_kind::S_AddOp: // AddOp
      case symbol_kind::S_MulOp: // MulOp
      case symbol_kind::S_UnaryOp: // UnaryOp
      case symbol_kind::S_RelOp: // RelOp
        value.move< ast::Exp::Op > (that.value);
        break;

      case symbol_kind::S_FuncCall: // FuncCall
        value.move< ast::FuncCall * > (that.value);
        break;

      case symbol_kind::S_FuncFParam: // FuncFParam
      case symbol_kind::S_FuncFSingleParam: // FuncFSingleParam
      case symbol_kind::S_FuncFParamArray: // FuncFParamArray
        value.move< ast::FuncFParam * > (that.value);
        break;

      case symbol_kind::S_FuncDef: // FuncDef
        value.move< ast::Function * > (that.value);
        break;

      case symbol_kind::S_IfStmt: // IfStmt
        value.move< ast::IfStmt * > (that.value);
        break;

      case symbol_kind::S_InitVal: // InitVal
      case symbol_kind::S_InitValArray: // InitValArray
      case symbol_kind::S_InitValArrayInner: // InitValArrayInner
        value.move< ast::InitVal * > (that.value);
        break;

      case symbol_kind::S_LVal: // LVal
        value.move< ast::LVal * > (that.value);
        break;

      case symbol_kind::S_ReturnStmt: // ReturnStmt
        value.move< ast::ReturnStmt * > (that.value);
        break;

      case symbol_kind::S_Stmt: // Stmt
        value.move< ast::Stmt * > (that.value);
        break;

      case symbol_kind::S_WhileStmt: // WhileStmt
        value.move< ast::WhileStmt * > (that.value);
        break;

      case symbol_kind::S_INTCONST: // "intconst"
        value.move< int > (that.value);
        break;

      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_IDENT: // IDENT
        value.move< string > (that.value);
        break;

      case symbol_kind::S_Decl: // Decl
      case symbol_kind::S_ConstDecl: // ConstDecl
      case symbol_kind::S_VarDecl: // VarDecl
        value.move< vector<ast::Decl *> > (that.value);
        break;

      case symbol_kind::S_FuncRParams: // FuncRParams
        value.move< vector<ast::Exp *> > (that.value);
        break;

      case symbol_kind::S_FuncFParams: // FuncFParams
        value.move< vector<ast::FuncFParam *> > (that.value);
        break;

      case symbol_kind::S_BlockItem: // BlockItem
        value.move< vector<ast::Node *> > (that.value);
        break;

      default:
        break;
    }

    location = that.location;
    // that is emptied.
    that.state = empty_state;
    return *this;
  }
#endif

  template <typename Base>
  void
  tcc_sy_parser::yy_destroy_ (const char* yymsg, basic_symbol<Base>& yysym) const
  {
    if (yymsg)
      YY_SYMBOL_PRINT (yymsg, yysym);
  }

#if YYDEBUG
  template <typename Base>
  void
  tcc_sy_parser::yy_print_ (std::ostream& yyo, const basic_symbol<Base>& yysym) const
  {
    std::ostream& yyoutput = yyo;
    YYUSE (yyoutput);
    if (yysym.empty ())
      yyo << "empty symbol";
    else
      {
        symbol_kind_type yykind = yysym.kind ();
        yyo << (yykind < YYNTOKENS ? "token" : "nterm")
            << ' ' << yysym.name () << " ("
            << yysym.location << ": ";
        YYUSE (yykind);
        yyo << ')';
      }
  }
#endif

  void
  tcc_sy_parser::yypush_ (const char* m, YY_MOVE_REF (stack_symbol_type) sym)
  {
    if (m)
      YY_SYMBOL_PRINT (m, sym);
    yystack_.push (YY_MOVE (sym));
  }

  void
  tcc_sy_parser::yypush_ (const char* m, state_type s, YY_MOVE_REF (symbol_type) sym)
  {
#if 201103L <= YY_CPLUSPLUS
    yypush_ (m, stack_symbol_type (s, std::move (sym)));
#else
    stack_symbol_type ss (s, sym);
    yypush_ (m, ss);
#endif
  }

  void
  tcc_sy_parser::yypop_ (int n)
  {
    yystack_.pop (n);
  }

#if YYDEBUG
  std::ostream&
  tcc_sy_parser::debug_stream () const
  {
    return *yycdebug_;
  }

  void
  tcc_sy_parser::set_debug_stream (std::ostream& o)
  {
    yycdebug_ = &o;
  }


  tcc_sy_parser::debug_level_type
  tcc_sy_parser::debug_level () const
  {
    return yydebug_;
  }

  void
  tcc_sy_parser::set_debug_level (debug_level_type l)
  {
    yydebug_ = l;
  }
#endif // YYDEBUG

  tcc_sy_parser::state_type
  tcc_sy_parser::yy_lr_goto_state_ (state_type yystate, int yysym)
  {
    int yyr = yypgoto_[yysym - YYNTOKENS] + yystate;
    if (0 <= yyr && yyr <= yylast_ && yycheck_[yyr] == yystate)
      return yytable_[yyr];
    else
      return yydefgoto_[yysym - YYNTOKENS];
  }

  bool
  tcc_sy_parser::yy_pact_value_is_default_ (int yyvalue)
  {
    return yyvalue == yypact_ninf_;
  }

  bool
  tcc_sy_parser::yy_table_value_is_error_ (int yyvalue)
  {
    return yyvalue == yytable_ninf_;
  }

  int
  tcc_sy_parser::operator() ()
  {
    return parse ();
  }

  int
  tcc_sy_parser::parse ()
  {
    int yyn;
    /// Length of the RHS of the rule being reduced.
    int yylen = 0;

    // Error handling.
    int yynerrs_ = 0;
    int yyerrstatus_ = 0;

    /// The lookahead symbol.
    symbol_type yyla;

    /// The locations where the error started and ended.
    stack_symbol_type yyerror_range[3];

    /// The return value of parse ().
    int yyresult;

#if YY_EXCEPTIONS
    try
#endif // YY_EXCEPTIONS
      {
    YYCDEBUG << "Starting parse\n";


    // User initialization code.
#line 22 "src/parser/parser.yy"
{
  // Initialize the initial location.
  yyla.location.begin.filename = yyla.location.end.filename = &driver.file;
}

#line 884 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"


    /* Initialize the stack.  The initial state will be set in
       yynewstate, since the latter expects the semantical and the
       location values to have been already stored, initialize these
       stacks with a primary value.  */
    yystack_.clear ();
    yypush_ (YY_NULLPTR, 0, YY_MOVE (yyla));

  /*-----------------------------------------------.
  | yynewstate -- push a new symbol on the stack.  |
  `-----------------------------------------------*/
  yynewstate:
    YYCDEBUG << "Entering state " << int (yystack_[0].state) << '\n';
    YY_STACK_PRINT ();

    // Accept?
    if (yystack_[0].state == yyfinal_)
      YYACCEPT;

    goto yybackup;


  /*-----------.
  | yybackup.  |
  `-----------*/
  yybackup:
    // Try to take a decision without lookahead.
    yyn = yypact_[+yystack_[0].state];
    if (yy_pact_value_is_default_ (yyn))
      goto yydefault;

    // Read a lookahead token.
    if (yyla.empty ())
      {
        YYCDEBUG << "Reading a token\n";
#if YY_EXCEPTIONS
        try
#endif // YY_EXCEPTIONS
          {
            symbol_type yylookahead (yylex (driver));
            yyla.move (yylookahead);
          }
#if YY_EXCEPTIONS
        catch (const syntax_error& yyexc)
          {
            YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
            error (yyexc);
            goto yyerrlab1;
          }
#endif // YY_EXCEPTIONS
      }
    YY_SYMBOL_PRINT ("Next token is", yyla);

    if (yyla.kind () == symbol_kind::S_YYerror)
    {
      // The scanner already issued an error message, process directly
      // to error recovery.  But do not keep the error token as
      // lookahead, it is too special and may lead us to an endless
      // loop in error recovery. */
      yyla.kind_ = symbol_kind::S_YYUNDEF;
      goto yyerrlab1;
    }

    /* If the proper action on seeing token YYLA.TYPE is to reduce or
       to detect an error, take that action.  */
    yyn += yyla.kind ();
    if (yyn < 0 || yylast_ < yyn || yycheck_[yyn] != yyla.kind ())
      {
        goto yydefault;
      }

    // Reduce or error.
    yyn = yytable_[yyn];
    if (yyn <= 0)
      {
        if (yy_table_value_is_error_ (yyn))
          goto yyerrlab;
        yyn = -yyn;
        goto yyreduce;
      }

    // Count tokens shifted since error; after three, turn off error status.
    if (yyerrstatus_)
      --yyerrstatus_;

    // Shift the lookahead token.
    yypush_ ("Shifting", state_type (yyn), YY_MOVE (yyla));
    goto yynewstate;


  /*-----------------------------------------------------------.
  | yydefault -- do the default action for the current state.  |
  `-----------------------------------------------------------*/
  yydefault:
    yyn = yydefact_[+yystack_[0].state];
    if (yyn == 0)
      goto yyerrlab;
    goto yyreduce;


  /*-----------------------------.
  | yyreduce -- do a reduction.  |
  `-----------------------------*/
  yyreduce:
    yylen = yyr2_[yyn];
    {
      stack_symbol_type yylhs;
      yylhs.state = yy_lr_goto_state_ (yystack_[yylen].state, yyr1_[yyn]);
      /* Variants are always initialized to an empty instance of the
         correct type. The default '$$ = $1' action is NOT applied
         when using variants.  */
      switch (yyr1_[yyn])
    {
      case symbol_kind::S_Block: // Block
      case symbol_kind::S_BlockItems: // BlockItems
        yylhs.value.emplace< ast::Block * > ();
        break;

      case symbol_kind::S_BreakStmt: // BreakStmt
        yylhs.value.emplace< ast::BreakStmt * > ();
        break;

      case symbol_kind::S_Cond: // Cond
        yylhs.value.emplace< ast::Cond * > ();
        break;

      case symbol_kind::S_ContinueStmt: // ContinueStmt
        yylhs.value.emplace< ast::ContinueStmt * > ();
        break;

      case symbol_kind::S_VarDef: // VarDef
      case symbol_kind::S_DefSingleElem: // DefSingleElem
      case symbol_kind::S_DefArray: // DefArray
      case symbol_kind::S_ArrayBody: // ArrayBody
      case symbol_kind::S_ConstDef: // ConstDef
      case symbol_kind::S_ConstDefSingleElem: // ConstDefSingleElem
      case symbol_kind::S_ConstDefArray: // ConstDefArray
        yylhs.value.emplace< ast::Decl * > ();
        break;

      case symbol_kind::S_BType: // BType
        yylhs.value.emplace< ast::Decl::VarType > ();
        break;

      case symbol_kind::S_Exp: // Exp
      case symbol_kind::S_LOrExp: // LOrExp
      case symbol_kind::S_LAndExp: // LAndExp
      case symbol_kind::S_EqExp: // EqExp
      case symbol_kind::S_RelExp: // RelExp
      case symbol_kind::S_AddExp: // AddExp
      case symbol_kind::S_MulExp: // MulExp
      case symbol_kind::S_UnaryExp: // UnaryExp
      case symbol_kind::S_PrimaryExp: // PrimaryExp
      case symbol_kind::S_ArrayItem: // ArrayItem
      case symbol_kind::S_Number: // Number
        yylhs.value.emplace< ast::Exp * > ();
        break;

      case symbol_kind::S_AddOp: // AddOp
      case symbol_kind::S_MulOp: // MulOp
      case symbol_kind::S_UnaryOp: // UnaryOp
      case symbol_kind::S_RelOp: // RelOp
        yylhs.value.emplace< ast::Exp::Op > ();
        break;

      case symbol_kind::S_FuncCall: // FuncCall
        yylhs.value.emplace< ast::FuncCall * > ();
        break;

      case symbol_kind::S_FuncFParam: // FuncFParam
      case symbol_kind::S_FuncFSingleParam: // FuncFSingleParam
      case symbol_kind::S_FuncFParamArray: // FuncFParamArray
        yylhs.value.emplace< ast::FuncFParam * > ();
        break;

      case symbol_kind::S_FuncDef: // FuncDef
        yylhs.value.emplace< ast::Function * > ();
        break;

      case symbol_kind::S_IfStmt: // IfStmt
        yylhs.value.emplace< ast::IfStmt * > ();
        break;

      case symbol_kind::S_InitVal: // InitVal
      case symbol_kind::S_InitValArray: // InitValArray
      case symbol_kind::S_InitValArrayInner: // InitValArrayInner
        yylhs.value.emplace< ast::InitVal * > ();
        break;

      case symbol_kind::S_LVal: // LVal
        yylhs.value.emplace< ast::LVal * > ();
        break;

      case symbol_kind::S_ReturnStmt: // ReturnStmt
        yylhs.value.emplace< ast::ReturnStmt * > ();
        break;

      case symbol_kind::S_Stmt: // Stmt
        yylhs.value.emplace< ast::Stmt * > ();
        break;

      case symbol_kind::S_WhileStmt: // WhileStmt
        yylhs.value.emplace< ast::WhileStmt * > ();
        break;

      case symbol_kind::S_INTCONST: // "intconst"
        yylhs.value.emplace< int > ();
        break;

      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_IDENT: // IDENT
        yylhs.value.emplace< string > ();
        break;

      case symbol_kind::S_Decl: // Decl
      case symbol_kind::S_ConstDecl: // ConstDecl
      case symbol_kind::S_VarDecl: // VarDecl
        yylhs.value.emplace< vector<ast::Decl *> > ();
        break;

      case symbol_kind::S_FuncRParams: // FuncRParams
        yylhs.value.emplace< vector<ast::Exp *> > ();
        break;

      case symbol_kind::S_FuncFParams: // FuncFParams
        yylhs.value.emplace< vector<ast::FuncFParam *> > ();
        break;

      case symbol_kind::S_BlockItem: // BlockItem
        yylhs.value.emplace< vector<ast::Node *> > ();
        break;

      default:
        break;
    }


      // Default location.
      {
        stack_type::slice range (yystack_, yylen);
        YYLLOC_DEFAULT (yylhs.location, range, yylen);
        yyerror_range[1].location = yylhs.location;
      }

      // Perform the reduction.
      YY_REDUCE_PRINT (yyn);
#if YY_EXCEPTIONS
      try
#endif // YY_EXCEPTIONS
        {
          switch (yyn)
            {
  case 2: // CompUnit: CompUnit Decl
#line 104 "src/parser/parser.yy"
                        { driver.comp_unit->append_decls(yystack_[0].value.as < vector<ast::Decl *> > ()); }
#line 1141 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 3: // CompUnit: CompUnit FuncDef
#line 105 "src/parser/parser.yy"
                           { driver.comp_unit->append_function(yystack_[0].value.as < ast::Function * > ()); }
#line 1147 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 4: // CompUnit: Decl
#line 106 "src/parser/parser.yy"
               { driver.comp_unit->append_decls(yystack_[0].value.as < vector<ast::Decl *> > ()); }
#line 1153 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 5: // CompUnit: FuncDef
#line 107 "src/parser/parser.yy"
                  { driver.comp_unit->append_function(yystack_[0].value.as < ast::Function * > ()); }
#line 1159 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 6: // Decl: ConstDecl ";"
#line 110 "src/parser/parser.yy"
                    { std::swap(yylhs.value.as < vector<ast::Decl *> > (), yystack_[1].value.as < vector<ast::Decl *> > ()); }
#line 1165 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 7: // Decl: VarDecl ";"
#line 111 "src/parser/parser.yy"
                  { std::swap(yylhs.value.as < vector<ast::Decl *> > (), yystack_[1].value.as < vector<ast::Decl *> > ()); }
#line 1171 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 8: // BType: "int"
#line 114 "src/parser/parser.yy"
             { yylhs.value.as < ast::Decl::VarType > () = ast::Decl::VarType::INT; }
#line 1177 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 9: // ConstDecl: "const" BType ConstDef
#line 116 "src/parser/parser.yy"
                                  { yystack_[0].value.as < ast::Decl * > ()->is_const = true; yylhs.value.as < vector<ast::Decl *> > ().emplace_back(yystack_[0].value.as < ast::Decl * > ()); }
#line 1183 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 10: // ConstDecl: ConstDecl "," ConstDef
#line 117 "src/parser/parser.yy"
                                  { std::swap(yylhs.value.as < vector<ast::Decl *> > (), yystack_[2].value.as < vector<ast::Decl *> > ()); yystack_[0].value.as < ast::Decl * > ()->is_const = true; yylhs.value.as < vector<ast::Decl *> > ().emplace_back(yystack_[0].value.as < ast::Decl * > ()); }
#line 1189 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 11: // VarDecl: BType VarDef
#line 120 "src/parser/parser.yy"
                      { yylhs.value.as < vector<ast::Decl *> > ().emplace_back(yystack_[0].value.as < ast::Decl * > ()); }
#line 1195 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 12: // VarDecl: VarDecl "," VarDef
#line 121 "src/parser/parser.yy"
                            { std::swap(yylhs.value.as < vector<ast::Decl *> > (), yystack_[2].value.as < vector<ast::Decl *> > ()); yylhs.value.as < vector<ast::Decl *> > ().emplace_back(yystack_[0].value.as < ast::Decl * > ()); }
#line 1201 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 13: // VarDef: DefSingleElem
#line 124 "src/parser/parser.yy"
                      { yylhs.value.as < ast::Decl * > () = yystack_[0].value.as < ast::Decl * > (); }
#line 1207 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 14: // VarDef: DefArray
#line 125 "src/parser/parser.yy"
                 { yylhs.value.as < ast::Decl * > () = yystack_[0].value.as < ast::Decl * > (); }
#line 1213 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 15: // DefSingleElem: IDENT "=" InitVal
#line 128 "src/parser/parser.yy"
                                 { yylhs.value.as < ast::Decl * > () = new ast::Decl(yystack_[2].value.as < string > (), yystack_[0].value.as < ast::InitVal * > ()); }
#line 1219 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 16: // DefSingleElem: IDENT
#line 129 "src/parser/parser.yy"
                     { yylhs.value.as < ast::Decl * > () = new ast::Decl(yystack_[0].value.as < string > ()); }
#line 1225 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 17: // DefArray: ArrayBody "=" InitValArray
#line 132 "src/parser/parser.yy"
                                     { yylhs.value.as < ast::Decl * > () = yystack_[2].value.as < ast::Decl * > (); yylhs.value.as < ast::Decl * > ()->initval = yystack_[0].value.as < ast::InitVal * > (); }
#line 1231 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 18: // DefArray: ArrayBody
#line 133 "src/parser/parser.yy"
                    { yylhs.value.as < ast::Decl * > () = yystack_[0].value.as < ast::Decl * > (); }
#line 1237 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 19: // ArrayBody: ArrayBody "[" Exp "]"
#line 136 "src/parser/parser.yy"
                                 { yylhs.value.as < ast::Decl * > () = yystack_[3].value.as < ast::Decl * > (); yylhs.value.as < ast::Decl * > ()->add_dim(yystack_[1].value.as < ast::Exp * > ()); }
#line 1243 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 20: // ArrayBody: IDENT "[" Exp "]"
#line 137 "src/parser/parser.yy"
                             { yylhs.value.as < ast::Decl * > () = new ast::Decl(yystack_[3].value.as < string > ()); yylhs.value.as < ast::Decl * > ()->add_dim(yystack_[1].value.as < ast::Exp * > ()); }
#line 1249 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 21: // ConstDef: ConstDefSingleElem
#line 140 "src/parser/parser.yy"
                             { yylhs.value.as < ast::Decl * > () = yystack_[0].value.as < ast::Decl * > (); }
#line 1255 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 22: // ConstDef: ConstDefArray
#line 141 "src/parser/parser.yy"
                        { yylhs.value.as < ast::Decl * > () = yystack_[0].value.as < ast::Decl * > (); }
#line 1261 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 23: // ConstDefSingleElem: IDENT "=" InitVal
#line 144 "src/parser/parser.yy"
                                      { yylhs.value.as < ast::Decl * > () = new ast::Decl(yystack_[2].value.as < string > (), yystack_[0].value.as < ast::InitVal * > ()); }
#line 1267 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 24: // ConstDefArray: ArrayBody "=" InitValArray
#line 146 "src/parser/parser.yy"
                                          { yylhs.value.as < ast::Decl * > () = yystack_[2].value.as < ast::Decl * > (); yylhs.value.as < ast::Decl * > ()->initval = yystack_[0].value.as < ast::InitVal * > (); }
#line 1273 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 25: // InitVal: AddExp
#line 149 "src/parser/parser.yy"
                { yylhs.value.as < ast::InitVal * > () = new ast::InitVal(yystack_[0].value.as < ast::Exp * > ()); }
#line 1279 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 26: // InitValArray: "{" InitValArrayInner "}"
#line 151 "src/parser/parser.yy"
                                        { yylhs.value.as < ast::InitVal * > () = yystack_[1].value.as < ast::InitVal * > (); }
#line 1285 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 27: // InitValArray: "{" "}"
#line 152 "src/parser/parser.yy"
                      { yylhs.value.as < ast::InitVal * > () = new ast::InitVal(); }
#line 1291 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 28: // InitValArrayInner: InitValArrayInner "," InitValArray
#line 155 "src/parser/parser.yy"
                                                      { yylhs.value.as < ast::InitVal * > () = yystack_[2].value.as < ast::InitVal * > (); yylhs.value.as < ast::InitVal * > ()->append_entry(yystack_[0].value.as < ast::InitVal * > ()); }
#line 1297 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 29: // InitValArrayInner: InitValArrayInner "," InitVal
#line 156 "src/parser/parser.yy"
                                                 { yylhs.value.as < ast::InitVal * > () = yystack_[2].value.as < ast::InitVal * > (); yylhs.value.as < ast::InitVal * > ()->append_entry(yystack_[0].value.as < ast::InitVal * > ()); }
#line 1303 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 30: // InitValArrayInner: InitValArray
#line 157 "src/parser/parser.yy"
                                { yylhs.value.as < ast::InitVal * > () = new ast::InitVal(yystack_[0].value.as < ast::InitVal * > ()); }
#line 1309 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 31: // InitValArrayInner: InitVal
#line 158 "src/parser/parser.yy"
                           { yylhs.value.as < ast::InitVal * > () = new ast::InitVal(yystack_[0].value.as < ast::InitVal * > ()); }
#line 1315 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 32: // Exp: AddExp
#line 161 "src/parser/parser.yy"
            { yylhs.value.as < ast::Exp * > () = yystack_[0].value.as < ast::Exp * > (); }
#line 1321 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 33: // LOrExp: LOrExp "||" LAndExp
#line 163 "src/parser/parser.yy"
                            { yylhs.value.as < ast::Exp * > () = new ast::Exp(ast::Exp::Op::LOGIC_OR, yystack_[2].value.as < ast::Exp * > (), yystack_[0].value.as < ast::Exp * > ()); }
#line 1327 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 34: // LOrExp: LAndExp
#line 164 "src/parser/parser.yy"
                { yylhs.value.as < ast::Exp * > () = yystack_[0].value.as < ast::Exp * > (); }
#line 1333 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 35: // LAndExp: EqExp
#line 167 "src/parser/parser.yy"
               { yylhs.value.as < ast::Exp * > () = yystack_[0].value.as < ast::Exp * > (); }
#line 1339 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 36: // LAndExp: LAndExp "&&" EqExp
#line 168 "src/parser/parser.yy"
                            { yylhs.value.as < ast::Exp * > () = new ast::Exp(ast::Exp::Op::LOGIC_AND, yystack_[2].value.as < ast::Exp * > (), yystack_[0].value.as < ast::Exp * > ()); }
#line 1345 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 37: // EqExp: RelExp
#line 171 "src/parser/parser.yy"
              { yylhs.value.as < ast::Exp * > () = yystack_[0].value.as < ast::Exp * > (); }
#line 1351 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 38: // EqExp: RelExp "==" RelExp
#line 172 "src/parser/parser.yy"
                          { yylhs.value.as < ast::Exp * > () = new ast::Exp(ast::Exp::Op::EQ, yystack_[2].value.as < ast::Exp * > (), yystack_[0].value.as < ast::Exp * > ()); }
#line 1357 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 39: // EqExp: RelExp "!=" RelExp
#line 173 "src/parser/parser.yy"
                          { yylhs.value.as < ast::Exp * > () = new ast::Exp(ast::Exp::Op::INEQ, yystack_[2].value.as < ast::Exp * > (), yystack_[0].value.as < ast::Exp * > ()); }
#line 1363 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 40: // RelExp: AddExp
#line 176 "src/parser/parser.yy"
               { yylhs.value.as < ast::Exp * > () = yystack_[0].value.as < ast::Exp * > (); }
#line 1369 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 41: // RelExp: RelExp RelOp AddExp
#line 177 "src/parser/parser.yy"
                            { yylhs.value.as < ast::Exp * > () = new ast::Exp(yystack_[1].value.as < ast::Exp::Op > (), yystack_[2].value.as < ast::Exp * > (), yystack_[0].value.as < ast::Exp * > ()); }
#line 1375 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 42: // AddExp: MulExp
#line 180 "src/parser/parser.yy"
               { yylhs.value.as < ast::Exp * > () = yystack_[0].value.as < ast::Exp * > (); }
#line 1381 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 43: // AddExp: AddExp AddOp MulExp
#line 181 "src/parser/parser.yy"
                            { yylhs.value.as < ast::Exp * > () = new ast::Exp(yystack_[1].value.as < ast::Exp::Op > (), yystack_[2].value.as < ast::Exp * > (), yystack_[0].value.as < ast::Exp * > ()); }
#line 1387 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 44: // MulExp: UnaryExp
#line 184 "src/parser/parser.yy"
                 { yylhs.value.as < ast::Exp * > () = yystack_[0].value.as < ast::Exp * > (); }
#line 1393 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 45: // MulExp: MulExp MulOp UnaryExp
#line 185 "src/parser/parser.yy"
                              { yylhs.value.as < ast::Exp * > () = new ast::Exp(yystack_[1].value.as < ast::Exp::Op > (), yystack_[2].value.as < ast::Exp * > (), yystack_[0].value.as < ast::Exp * > ()); }
#line 1399 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 46: // UnaryExp: PrimaryExp
#line 188 "src/parser/parser.yy"
                     { yylhs.value.as < ast::Exp * > () = yystack_[0].value.as < ast::Exp * > (); }
#line 1405 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 47: // UnaryExp: FuncCall
#line 189 "src/parser/parser.yy"
                   { yylhs.value.as < ast::Exp * > () = new ast::Exp(yystack_[0].value.as < ast::FuncCall * > ()); }
#line 1411 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 48: // UnaryExp: UnaryOp UnaryExp
#line 190 "src/parser/parser.yy"
                           { yylhs.value.as < ast::Exp * > () = new ast::Exp(yystack_[1].value.as < ast::Exp::Op > (), yystack_[0].value.as < ast::Exp * > ()); }
#line 1417 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 49: // FuncCall: IDENT "(" FuncRParams ")"
#line 193 "src/parser/parser.yy"
                                    { yylhs.value.as < ast::FuncCall * > () = new ast::FuncCall(yystack_[3].value.as < string > (), yylhs.location.end.line); std::swap(yylhs.value.as < ast::FuncCall * > ()->params, yystack_[1].value.as < vector<ast::Exp *> > ()); }
#line 1423 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 50: // FuncCall: IDENT "(" ")"
#line 194 "src/parser/parser.yy"
                        { yylhs.value.as < ast::FuncCall * > () = new ast::FuncCall(yystack_[2].value.as < string > (), yylhs.location.end.line); }
#line 1429 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 51: // PrimaryExp: "(" Exp ")"
#line 197 "src/parser/parser.yy"
                        { yylhs.value.as < ast::Exp * > () = yystack_[1].value.as < ast::Exp * > (); }
#line 1435 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 52: // PrimaryExp: LVal
#line 198 "src/parser/parser.yy"
                 { yylhs.value.as < ast::Exp * > () = new ast::Exp(yystack_[0].value.as < ast::LVal * > ()); }
#line 1441 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 53: // PrimaryExp: Number
#line 199 "src/parser/parser.yy"
                   { yylhs.value.as < ast::Exp * > () = yystack_[0].value.as < ast::Exp * > (); }
#line 1447 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 54: // ArrayItem: "[" Exp "]"
#line 202 "src/parser/parser.yy"
                       { yylhs.value.as < ast::Exp * > () = yystack_[1].value.as < ast::Exp * > (); }
#line 1453 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 55: // LVal: LVal ArrayItem
#line 204 "src/parser/parser.yy"
                     { yylhs.value.as < ast::LVal * > () = yystack_[1].value.as < ast::LVal * > (); yylhs.value.as < ast::LVal * > ()->add_dim(yystack_[0].value.as < ast::Exp * > ()); }
#line 1459 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 56: // LVal: IDENT
#line 205 "src/parser/parser.yy"
            { yylhs.value.as < ast::LVal * > () = new ast::LVal(yystack_[0].value.as < string > ()); }
#line 1465 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 57: // FuncDef: "void" IDENT "(" FuncFParams ")" Block
#line 208 "src/parser/parser.yy"
                                                { yylhs.value.as < ast::Function * > () = new ast::Function(ast::Function::Type::VOID, yystack_[4].value.as < string > (), yystack_[2].value.as < vector<ast::FuncFParam *> > (), yystack_[0].value.as < ast::Block * > ()); }
#line 1471 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 58: // FuncDef: "void" IDENT "(" ")" Block
#line 209 "src/parser/parser.yy"
                                    { yylhs.value.as < ast::Function * > () = new ast::Function(ast::Function::Type::VOID, yystack_[3].value.as < string > (), yystack_[0].value.as < ast::Block * > ()); }
#line 1477 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 59: // FuncDef: BType IDENT "(" FuncFParams ")" Block
#line 210 "src/parser/parser.yy"
                                               { yylhs.value.as < ast::Function * > () = new ast::Function(ast::Function::Type::INT, yystack_[4].value.as < string > (), yystack_[2].value.as < vector<ast::FuncFParam *> > (), yystack_[0].value.as < ast::Block * > ()); }
#line 1483 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 60: // FuncDef: BType IDENT "(" ")" Block
#line 211 "src/parser/parser.yy"
                                   { yylhs.value.as < ast::Function * > () = new ast::Function(ast::Function::Type::INT, yystack_[3].value.as < string > (), yystack_[0].value.as < ast::Block * > ()); }
#line 1489 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 61: // FuncFParams: FuncFParams "," FuncFParam
#line 214 "src/parser/parser.yy"
                                        { std::swap(yylhs.value.as < vector<ast::FuncFParam *> > (), yystack_[2].value.as < vector<ast::FuncFParam *> > ()); yylhs.value.as < vector<ast::FuncFParam *> > ().emplace_back(yystack_[0].value.as < ast::FuncFParam * > ()); }
#line 1495 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 62: // FuncFParams: FuncFParam
#line 215 "src/parser/parser.yy"
                        { yylhs.value.as < vector<ast::FuncFParam *> > ().emplace_back(yystack_[0].value.as < ast::FuncFParam * > ()); }
#line 1501 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 63: // FuncFParam: FuncFSingleParam
#line 218 "src/parser/parser.yy"
                             { yylhs.value.as < ast::FuncFParam * > () = yystack_[0].value.as < ast::FuncFParam * > (); }
#line 1507 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 64: // FuncFParam: FuncFParamArray
#line 219 "src/parser/parser.yy"
                             { yylhs.value.as < ast::FuncFParam * > () = yystack_[0].value.as < ast::FuncFParam * > (); }
#line 1513 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 65: // FuncRParams: FuncRParams "," AddExp
#line 222 "src/parser/parser.yy"
                                    { std::swap(yylhs.value.as < vector<ast::Exp *> > (), yystack_[2].value.as < vector<ast::Exp *> > ()); yylhs.value.as < vector<ast::Exp *> > ().emplace_back(yystack_[0].value.as < ast::Exp * > ()); }
#line 1519 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 66: // FuncRParams: AddExp
#line 223 "src/parser/parser.yy"
                    { yylhs.value.as < vector<ast::Exp *> > ().emplace_back(yystack_[0].value.as < ast::Exp * > ()); }
#line 1525 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 67: // FuncFSingleParam: BType IDENT
#line 226 "src/parser/parser.yy"
                              { yylhs.value.as < ast::FuncFParam * > () = new ast::FuncFParam(yystack_[1].value.as < ast::Decl::VarType > (), yystack_[0].value.as < string > ()); }
#line 1531 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 68: // FuncFParamArray: FuncFSingleParam "[" "]"
#line 228 "src/parser/parser.yy"
                                          { yylhs.value.as < ast::FuncFParam * > () = yystack_[2].value.as < ast::FuncFParam * > (); yylhs.value.as < ast::FuncFParam * > ()->signature->add_dim(0); }
#line 1537 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 69: // FuncFParamArray: FuncFParamArray "[" Exp "]"
#line 229 "src/parser/parser.yy"
                                             { yylhs.value.as < ast::FuncFParam * > () = yystack_[3].value.as < ast::FuncFParam * > (); yylhs.value.as < ast::FuncFParam * > ()->signature->add_dim(yystack_[1].value.as < ast::Exp * > ()); }
#line 1543 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 70: // Block: "{" "}"
#line 232 "src/parser/parser.yy"
               { yylhs.value.as < ast::Block * > () = new ast::Block(); }
#line 1549 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 71: // Block: "{" BlockItems "}"
#line 233 "src/parser/parser.yy"
                          { yylhs.value.as < ast::Block * > () = yystack_[1].value.as < ast::Block * > (); }
#line 1555 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 72: // BlockItems: BlockItem
#line 236 "src/parser/parser.yy"
                      { yylhs.value.as < ast::Block * > () = new ast::Block(); yylhs.value.as < ast::Block * > ()->append_nodes(yystack_[0].value.as < vector<ast::Node *> > ()); }
#line 1561 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 73: // BlockItems: BlockItems BlockItem
#line 237 "src/parser/parser.yy"
                                 { yylhs.value.as < ast::Block * > () = yystack_[1].value.as < ast::Block * > (); yylhs.value.as < ast::Block * > ()->append_nodes(yystack_[0].value.as < vector<ast::Node *> > ()); }
#line 1567 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 74: // BlockItem: Decl
#line 240 "src/parser/parser.yy"
                { yylhs.value.as < vector<ast::Node *> > ().insert(std::end(yylhs.value.as < vector<ast::Node *> > ()), std::begin(yystack_[0].value.as < vector<ast::Decl *> > ()), std::end(yystack_[0].value.as < vector<ast::Decl *> > ())); }
#line 1573 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 75: // BlockItem: Stmt
#line 241 "src/parser/parser.yy"
                { yylhs.value.as < vector<ast::Node *> > ().emplace_back(yystack_[0].value.as < ast::Stmt * > ()); }
#line 1579 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 76: // Stmt: LVal "=" Exp ";"
#line 244 "src/parser/parser.yy"
                       { yylhs.value.as < ast::Stmt * > () = new ast::AssignmentStmt(yystack_[3].value.as < ast::LVal * > (), yystack_[1].value.as < ast::Exp * > ()); }
#line 1585 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 77: // Stmt: Exp ";"
#line 245 "src/parser/parser.yy"
              { yylhs.value.as < ast::Stmt * > () = new ast::EvalStmt(yystack_[1].value.as < ast::Exp * > ()); }
#line 1591 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 78: // Stmt: ";"
#line 246 "src/parser/parser.yy"
          { yylhs.value.as < ast::Stmt * > () = new ast::Block(); }
#line 1597 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 79: // Stmt: Block
#line 247 "src/parser/parser.yy"
            { yylhs.value.as < ast::Stmt * > () = yystack_[0].value.as < ast::Block * > (); }
#line 1603 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 80: // Stmt: IfStmt
#line 248 "src/parser/parser.yy"
             { yylhs.value.as < ast::Stmt * > () = yystack_[0].value.as < ast::IfStmt * > (); }
#line 1609 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 81: // Stmt: WhileStmt
#line 249 "src/parser/parser.yy"
                { yylhs.value.as < ast::Stmt * > () = yystack_[0].value.as < ast::WhileStmt * > (); }
#line 1615 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 82: // Stmt: BreakStmt
#line 250 "src/parser/parser.yy"
                { yylhs.value.as < ast::Stmt * > () = yystack_[0].value.as < ast::BreakStmt * > (); }
#line 1621 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 83: // Stmt: ReturnStmt
#line 251 "src/parser/parser.yy"
                 { yylhs.value.as < ast::Stmt * > () = yystack_[0].value.as < ast::ReturnStmt * > (); }
#line 1627 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 84: // Stmt: ContinueStmt
#line 252 "src/parser/parser.yy"
                   { yylhs.value.as < ast::Stmt * > () = yystack_[0].value.as < ast::ContinueStmt * > (); }
#line 1633 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 85: // IfStmt: "if" "(" Cond ")" Stmt "else" Stmt
#line 255 "src/parser/parser.yy"
                                           { yylhs.value.as < ast::IfStmt * > () = new ast::IfStmt(yystack_[4].value.as < ast::Cond * > (), yystack_[2].value.as < ast::Stmt * > (), yystack_[0].value.as < ast::Stmt * > ()); }
#line 1639 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 86: // IfStmt: "if" "(" Cond ")" Stmt
#line 256 "src/parser/parser.yy"
                               { yylhs.value.as < ast::IfStmt * > () = new ast::IfStmt(yystack_[2].value.as < ast::Cond * > (), yystack_[0].value.as < ast::Stmt * > ()); }
#line 1645 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 87: // ReturnStmt: "return" Exp ";"
#line 259 "src/parser/parser.yy"
                             { yylhs.value.as < ast::ReturnStmt * > () = new ast::ReturnStmt(yystack_[1].value.as < ast::Exp * > ()); }
#line 1651 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 88: // ReturnStmt: "return" ";"
#line 260 "src/parser/parser.yy"
                         { yylhs.value.as < ast::ReturnStmt * > () = new ast::ReturnStmt; }
#line 1657 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 89: // WhileStmt: "while" "(" Cond ")" Stmt
#line 263 "src/parser/parser.yy"
                                     { yylhs.value.as < ast::WhileStmt * > () = new ast::WhileStmt(yystack_[2].value.as < ast::Cond * > (), yystack_[0].value.as < ast::Stmt * > ()); }
#line 1663 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 90: // BreakStmt: "break" ";"
#line 265 "src/parser/parser.yy"
                       { yylhs.value.as < ast::BreakStmt * > () = new ast::BreakStmt; }
#line 1669 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 91: // ContinueStmt: "continue" ";"
#line 267 "src/parser/parser.yy"
                             { yylhs.value.as < ast::ContinueStmt * > () = new ast::ContinueStmt; }
#line 1675 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 92: // Cond: LOrExp
#line 269 "src/parser/parser.yy"
             { yylhs.value.as < ast::Cond * > () = new ast::Cond(yystack_[0].value.as < ast::Exp * > ()); }
#line 1681 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 93: // Number: "intconst"
#line 271 "src/parser/parser.yy"
                 { yylhs.value.as < ast::Exp * > () = new ast::Exp(yystack_[0].value.as < int > ()); }
#line 1687 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 94: // AddOp: "+"
#line 273 "src/parser/parser.yy"
           { yylhs.value.as < ast::Exp::Op > () = ast::Exp::Op::PLUS; }
#line 1693 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 95: // AddOp: "-"
#line 274 "src/parser/parser.yy"
           { yylhs.value.as < ast::Exp::Op > () = ast::Exp::Op::MINUS; }
#line 1699 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 96: // MulOp: "*"
#line 277 "src/parser/parser.yy"
           { yylhs.value.as < ast::Exp::Op > () = ast::Exp::Op::MUL; }
#line 1705 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 97: // MulOp: "/"
#line 278 "src/parser/parser.yy"
           { yylhs.value.as < ast::Exp::Op > () = ast::Exp::Op::DIV; }
#line 1711 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 98: // MulOp: "%"
#line 279 "src/parser/parser.yy"
           { yylhs.value.as < ast::Exp::Op > () = ast::Exp::Op::MOD; }
#line 1717 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 99: // UnaryOp: "+"
#line 282 "src/parser/parser.yy"
             { yylhs.value.as < ast::Exp::Op > () = ast::Exp::Op::UNARY_PLUS; }
#line 1723 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 100: // UnaryOp: "-"
#line 283 "src/parser/parser.yy"
             { yylhs.value.as < ast::Exp::Op > () = ast::Exp::Op::UNARY_MINUS; }
#line 1729 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 101: // UnaryOp: "!"
#line 284 "src/parser/parser.yy"
             { yylhs.value.as < ast::Exp::Op > () = ast::Exp::Op::LOGIC_NOT; }
#line 1735 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 102: // RelOp: ">"
#line 287 "src/parser/parser.yy"
           { yylhs.value.as < ast::Exp::Op > () = ast::Exp::Op::GREATER_THAN; }
#line 1741 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 103: // RelOp: ">="
#line 288 "src/parser/parser.yy"
            { yylhs.value.as < ast::Exp::Op > () = ast::Exp::Op::GREATER_EQ; }
#line 1747 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 104: // RelOp: "<"
#line 289 "src/parser/parser.yy"
           { yylhs.value.as < ast::Exp::Op > () = ast::Exp::Op::LESS_THAN; }
#line 1753 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 105: // RelOp: "<="
#line 290 "src/parser/parser.yy"
            { yylhs.value.as < ast::Exp::Op > () = ast::Exp::Op::LESS_EQ; }
#line 1759 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;

  case 106: // IDENT: "identifier"
#line 293 "src/parser/parser.yy"
       { yylhs.value.as < string > () = yystack_[0].value.as < string > (); }
#line 1765 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"
    break;


#line 1769 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"

            default:
              break;
            }
        }
#if YY_EXCEPTIONS
      catch (const syntax_error& yyexc)
        {
          YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
          error (yyexc);
          YYERROR;
        }
#endif // YY_EXCEPTIONS
      YY_SYMBOL_PRINT ("-> $$ =", yylhs);
      yypop_ (yylen);
      yylen = 0;

      // Shift the result of the reduction.
      yypush_ (YY_NULLPTR, YY_MOVE (yylhs));
    }
    goto yynewstate;


  /*--------------------------------------.
  | yyerrlab -- here on detecting error.  |
  `--------------------------------------*/
  yyerrlab:
    // If not already recovering from an error, report this error.
    if (!yyerrstatus_)
      {
        ++yynerrs_;
        context yyctx (*this, yyla);
        std::string msg = yysyntax_error_ (yyctx);
        error (yyla.location, YY_MOVE (msg));
      }


    yyerror_range[1].location = yyla.location;
    if (yyerrstatus_ == 3)
      {
        /* If just tried and failed to reuse lookahead token after an
           error, discard it.  */

        // Return failure if at end of input.
        if (yyla.kind () == symbol_kind::S_YYEOF)
          YYABORT;
        else if (!yyla.empty ())
          {
            yy_destroy_ ("Error: discarding", yyla);
            yyla.clear ();
          }
      }

    // Else will try to reuse lookahead token after shifting the error token.
    goto yyerrlab1;


  /*---------------------------------------------------.
  | yyerrorlab -- error raised explicitly by YYERROR.  |
  `---------------------------------------------------*/
  yyerrorlab:
    /* Pacify compilers when the user code never invokes YYERROR and
       the label yyerrorlab therefore never appears in user code.  */
    if (false)
      YYERROR;

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYERROR.  */
    yypop_ (yylen);
    yylen = 0;
    YY_STACK_PRINT ();
    goto yyerrlab1;


  /*-------------------------------------------------------------.
  | yyerrlab1 -- common code for both syntax error and YYERROR.  |
  `-------------------------------------------------------------*/
  yyerrlab1:
    yyerrstatus_ = 3;   // Each real token shifted decrements this.
    // Pop stack until we find a state that shifts the error token.
    for (;;)
      {
        yyn = yypact_[+yystack_[0].state];
        if (!yy_pact_value_is_default_ (yyn))
          {
            yyn += symbol_kind::S_YYerror;
            if (0 <= yyn && yyn <= yylast_
                && yycheck_[yyn] == symbol_kind::S_YYerror)
              {
                yyn = yytable_[yyn];
                if (0 < yyn)
                  break;
              }
          }

        // Pop the current state because it cannot handle the error token.
        if (yystack_.size () == 1)
          YYABORT;

        yyerror_range[1].location = yystack_[0].location;
        yy_destroy_ ("Error: popping", yystack_[0]);
        yypop_ ();
        YY_STACK_PRINT ();
      }
    {
      stack_symbol_type error_token;

      yyerror_range[2].location = yyla.location;
      YYLLOC_DEFAULT (error_token.location, yyerror_range, 2);

      // Shift the error token.
      error_token.state = state_type (yyn);
      yypush_ ("Shifting", YY_MOVE (error_token));
    }
    goto yynewstate;


  /*-------------------------------------.
  | yyacceptlab -- YYACCEPT comes here.  |
  `-------------------------------------*/
  yyacceptlab:
    yyresult = 0;
    goto yyreturn;


  /*-----------------------------------.
  | yyabortlab -- YYABORT comes here.  |
  `-----------------------------------*/
  yyabortlab:
    yyresult = 1;
    goto yyreturn;


  /*-----------------------------------------------------.
  | yyreturn -- parsing is finished, return the result.  |
  `-----------------------------------------------------*/
  yyreturn:
    if (!yyla.empty ())
      yy_destroy_ ("Cleanup: discarding lookahead", yyla);

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYABORT or YYACCEPT.  */
    yypop_ (yylen);
    YY_STACK_PRINT ();
    while (1 < yystack_.size ())
      {
        yy_destroy_ ("Cleanup: popping", yystack_[0]);
        yypop_ ();
      }

    return yyresult;
  }
#if YY_EXCEPTIONS
    catch (...)
      {
        YYCDEBUG << "Exception caught: cleaning lookahead and stack\n";
        // Do not try to display the values of the reclaimed symbols,
        // as their printers might throw an exception.
        if (!yyla.empty ())
          yy_destroy_ (YY_NULLPTR, yyla);

        while (1 < yystack_.size ())
          {
            yy_destroy_ (YY_NULLPTR, yystack_[0]);
            yypop_ ();
          }
        throw;
      }
#endif // YY_EXCEPTIONS
  }

  void
  tcc_sy_parser::error (const syntax_error& yyexc)
  {
    error (yyexc.location, yyexc.what ());
  }

  /* Return YYSTR after stripping away unnecessary quotes and
     backslashes, so that it's suitable for yyerror.  The heuristic is
     that double-quoting is unnecessary unless the string contains an
     apostrophe, a comma, or backslash (other than backslash-backslash).
     YYSTR is taken from yytname.  */
  std::string
  tcc_sy_parser::yytnamerr_ (const char *yystr)
  {
    if (*yystr == '"')
      {
        std::string yyr;
        char const *yyp = yystr;

        for (;;)
          switch (*++yyp)
            {
            case '\'':
            case ',':
              goto do_not_strip_quotes;

            case '\\':
              if (*++yyp != '\\')
                goto do_not_strip_quotes;
              else
                goto append;

            append:
            default:
              yyr += *yyp;
              break;

            case '"':
              return yyr;
            }
      do_not_strip_quotes: ;
      }

    return yystr;
  }

  std::string
  tcc_sy_parser::symbol_name (symbol_kind_type yysymbol)
  {
    return yytnamerr_ (yytname_[yysymbol]);
  }



  // tcc_sy_parser::context.
  tcc_sy_parser::context::context (const tcc_sy_parser& yyparser, const symbol_type& yyla)
    : yyparser_ (yyparser)
    , yyla_ (yyla)
  {}

  int
  tcc_sy_parser::context::expected_tokens (symbol_kind_type yyarg[], int yyargn) const
  {
    // Actual number of expected tokens
    int yycount = 0;

    int yyn = yypact_[+yyparser_.yystack_[0].state];
    if (!yy_pact_value_is_default_ (yyn))
      {
        /* Start YYX at -YYN if negative to avoid negative indexes in
           YYCHECK.  In other words, skip the first -YYN actions for
           this state because they are default actions.  */
        int yyxbegin = yyn < 0 ? -yyn : 0;
        // Stay within bounds of both yycheck and yytname.
        int yychecklim = yylast_ - yyn + 1;
        int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
        for (int yyx = yyxbegin; yyx < yyxend; ++yyx)
          if (yycheck_[yyx + yyn] == yyx && yyx != symbol_kind::S_YYerror
              && !yy_table_value_is_error_ (yytable_[yyx + yyn]))
            {
              if (!yyarg)
                ++yycount;
              else if (yycount == yyargn)
                return 0;
              else
                yyarg[yycount++] = YY_CAST (symbol_kind_type, yyx);
            }
      }

    if (yyarg && yycount == 0 && 0 < yyargn)
      yyarg[0] = symbol_kind::S_YYEMPTY;
    return yycount;
  }



  int
  tcc_sy_parser::yy_syntax_error_arguments_ (const context& yyctx,
                                                 symbol_kind_type yyarg[], int yyargn) const
  {
    /* There are many possibilities here to consider:
       - If this state is a consistent state with a default action, then
         the only way this function was invoked is if the default action
         is an error action.  In that case, don't check for expected
         tokens because there are none.
       - The only way there can be no lookahead present (in yyla) is
         if this state is a consistent state with a default action.
         Thus, detecting the absence of a lookahead is sufficient to
         determine that there is no unexpected or expected token to
         report.  In that case, just report a simple "syntax error".
       - Don't assume there isn't a lookahead just because this state is
         a consistent state with a default action.  There might have
         been a previous inconsistent state, consistent state with a
         non-default action, or user semantic action that manipulated
         yyla.  (However, yyla is currently not documented for users.)
       - Of course, the expected token list depends on states to have
         correct lookahead information, and it depends on the parser not
         to perform extra reductions after fetching a lookahead from the
         scanner and before detecting a syntax error.  Thus, state merging
         (from LALR or IELR) and default reductions corrupt the expected
         token list.  However, the list is correct for canonical LR with
         one exception: it will still contain any token that will not be
         accepted due to an error action in a later state.
    */

    if (!yyctx.lookahead ().empty ())
      {
        if (yyarg)
          yyarg[0] = yyctx.token ();
        int yyn = yyctx.expected_tokens (yyarg ? yyarg + 1 : yyarg, yyargn - 1);
        return yyn + 1;
      }
    return 0;
  }

  // Generate an error message.
  std::string
  tcc_sy_parser::yysyntax_error_ (const context& yyctx) const
  {
    // Its maximum.
    enum { YYARGS_MAX = 5 };
    // Arguments of yyformat.
    symbol_kind_type yyarg[YYARGS_MAX];
    int yycount = yy_syntax_error_arguments_ (yyctx, yyarg, YYARGS_MAX);

    char const* yyformat = YY_NULLPTR;
    switch (yycount)
      {
#define YYCASE_(N, S)                         \
        case N:                               \
          yyformat = S;                       \
        break
      default: // Avoid compiler warnings.
        YYCASE_ (0, YY_("syntax error"));
        YYCASE_ (1, YY_("syntax error, unexpected %s"));
        YYCASE_ (2, YY_("syntax error, unexpected %s, expecting %s"));
        YYCASE_ (3, YY_("syntax error, unexpected %s, expecting %s or %s"));
        YYCASE_ (4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
        YYCASE_ (5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
      }

    std::string yyres;
    // Argument number.
    std::ptrdiff_t yyi = 0;
    for (char const* yyp = yyformat; *yyp; ++yyp)
      if (yyp[0] == '%' && yyp[1] == 's' && yyi < yycount)
        {
          yyres += symbol_name (yyarg[yyi++]);
          ++yyp;
        }
      else
        yyres += *yyp;
    return yyres;
  }


  const signed char tcc_sy_parser::yypact_ninf_ = -118;

  const signed char tcc_sy_parser::yytable_ninf_ = -1;

  const short
  tcc_sy_parser::yypact_[] =
  {
      85,    28,  -118,    33,   180,  -118,    33,   172,   175,  -118,
      33,  -118,    51,  -118,  -118,  -118,  -118,  -118,  -118,    41,
     145,    33,  -118,    33,  -118,    93,  -118,  -118,  -118,   130,
      11,   170,    55,   170,   105,   170,  -118,  -118,   142,    55,
     170,    58,    33,    45,  -118,    77,    84,  -118,   170,  -118,
    -118,  -118,   108,    -7,   151,  -118,  -118,  -118,    94,  -118,
     170,   123,    24,  -118,   154,    58,    82,  -118,    -7,  -118,
    -118,    62,  -118,  -118,    28,    58,   181,   170,   125,  -118,
    -118,  -118,   170,  -118,  -118,  -118,   170,   170,  -118,  -118,
       8,  -118,  -118,  -118,    29,  -118,  -118,    58,  -118,  -118,
     129,   148,   186,   187,    52,  -118,    33,   189,   152,  -118,
     127,  -118,  -118,  -118,  -118,  -118,  -118,  -118,  -118,  -118,
    -118,   194,  -118,   151,  -118,   196,  -118,    -7,   107,   101,
    -118,  -118,   170,   170,  -118,  -118,  -118,   199,  -118,   170,
    -118,  -118,  -118,  -118,   170,  -118,  -118,  -118,   174,   176,
    -118,   171,    -7,   198,   200,  -118,   203,    -7,   170,   170,
    -118,  -118,  -118,  -118,   170,   170,   170,   149,   149,  -118,
     176,  -118,   160,   160,    -7,   211,  -118,   149,  -118
  };

  const signed char
  tcc_sy_parser::yydefact_[] =
  {
       0,     0,     8,     0,     0,     4,     0,     0,     0,     5,
       0,   106,     0,     1,     2,     3,    11,    13,    14,    18,
      16,     0,     6,     0,     7,     0,     9,    21,    22,     0,
       0,     0,     0,     0,     0,     0,    10,    12,    16,     0,
       0,     0,     0,     0,    62,    63,    64,    93,     0,    99,
     100,   101,     0,    32,    42,    44,    47,    46,    52,    53,
       0,    56,     0,    17,     0,     0,     0,    15,    25,    24,
      23,     0,    58,    67,     0,     0,     0,     0,     0,    19,
      94,    95,     0,    96,    97,    98,     0,     0,    55,    48,
       0,    27,    31,    30,     0,    20,    60,     0,    78,    70,
       0,     0,     0,     0,     0,    74,     0,     0,    52,    79,
       0,    72,    75,    80,    83,    81,    82,    84,    61,    57,
      68,     0,    51,    43,    45,     0,    50,    66,     0,     0,
      26,    59,     0,     0,    90,    91,    88,     0,    77,     0,
      71,    73,    69,    54,     0,    49,    29,    28,    92,    34,
      35,    37,    40,     0,     0,    87,     0,    65,     0,     0,
     102,   103,   104,   105,     0,     0,     0,     0,     0,    76,
      33,    36,    38,    39,    41,    86,    89,     0,    85
  };

  const short
  tcc_sy_parser::yypgoto_[] =
  {
    -118,  -118,   118,     2,  -118,  -118,   193,  -118,  -118,    10,
     197,  -118,  -118,   -39,   -27,  -118,   -24,  -118,    59,    60,
     -37,   -35,   138,   -44,  -118,  -118,  -118,   -67,   217,   188,
     150,  -118,  -118,  -118,   -31,  -118,   113,  -117,  -118,  -118,
    -118,  -118,  -118,    92,  -118,  -118,  -118,  -118,  -118,     5
  };

  const short
  tcc_sy_parser::yydefgoto_[] =
  {
      -1,     4,   105,    42,     7,     8,    16,    17,    18,    19,
      26,    27,    28,    67,    63,    94,   107,   148,   149,   150,
     151,    53,    54,    55,    56,    57,    88,    58,     9,    43,
      44,   128,    45,    46,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   153,    59,    82,    86,    60,   166,    61
  };

  const unsigned char
  tcc_sy_parser::yytable_[] =
  {
      68,    70,     6,    10,   108,    68,     6,    52,    12,    64,
      72,    20,    69,    47,    11,    29,    89,    80,    81,     2,
      25,    48,   126,    92,    78,    41,    29,    68,    38,    47,
      11,    25,    49,    50,    96,    93,     2,    48,   129,    11,
      62,    91,   124,   108,   119,    51,   130,    73,    49,    50,
     175,   176,    31,   121,    74,   127,    32,    47,    11,    75,
     178,    51,   136,   125,    30,    48,   131,    47,    11,     1,
       2,    62,    98,   106,    71,    48,    49,    50,    71,    99,
     137,   100,   101,   102,   103,   104,    49,    50,    76,    51,
     146,    74,     1,     2,    68,    77,    97,   152,   152,    51,
     108,   108,   147,     3,    31,    87,    47,    11,    39,   157,
     108,    38,   106,     2,    48,   156,   144,    62,     5,    65,
      79,   145,    14,   152,   152,    49,    50,   172,   173,   152,
     152,   174,    47,    11,     1,     2,    90,    98,    51,   122,
      48,    33,   132,    71,   140,    40,   100,   101,   102,   103,
     104,    49,    50,    33,    47,    11,    33,    35,    34,    98,
      35,   133,    48,    87,    51,    71,    95,   139,   100,   101,
     102,   103,   104,    49,    50,    47,    11,    83,    84,    85,
      13,    21,    22,    48,    23,    24,    51,     1,     2,   160,
     161,   162,   163,   120,    49,    50,   134,   135,     3,   138,
     160,   161,   162,   163,   164,   165,   142,    51,   143,   155,
     158,   159,   167,   169,   168,   177,    37,   170,    36,   171,
     123,    15,    66,   141,   118,   154
  };

  const unsigned char
  tcc_sy_parser::yycheck_[] =
  {
      35,    40,     0,     1,    71,    40,     4,    31,     3,    33,
      41,     6,    39,     5,     6,    10,    60,    24,    25,     8,
      10,    13,    14,    62,    48,    14,    21,    62,    23,     5,
       6,    21,    24,    25,    65,    62,     8,    13,     9,     6,
      16,    17,    86,   110,    75,    37,    17,    42,    24,    25,
     167,   168,    11,    77,     9,    90,    15,     5,     6,    14,
     177,    37,    10,    87,    13,    13,    97,     5,     6,     7,
       8,    16,    10,    71,    16,    13,    24,    25,    16,    17,
     104,    19,    20,    21,    22,    23,    24,    25,    11,    37,
     129,     9,     7,     8,   129,    11,    14,   132,   133,    37,
     167,   168,   129,    18,    11,    11,     5,     6,    15,   144,
     177,   106,   110,     8,    13,   139,     9,    16,     0,    14,
      12,    14,     4,   158,   159,    24,    25,   164,   165,   164,
     165,   166,     5,     6,     7,     8,    13,    10,    37,    14,
      13,    11,    13,    16,    17,    15,    19,    20,    21,    22,
      23,    24,    25,    11,     5,     6,    11,    15,    13,    10,
      15,    13,    13,    11,    37,    16,    12,    15,    19,    20,
      21,    22,    23,    24,    25,     5,     6,    26,    27,    28,
       0,     9,    10,    13,     9,    10,    37,     7,     8,    29,
      30,    31,    32,    12,    24,    25,    10,    10,    18,    10,
      29,    30,    31,    32,    33,    34,    12,    37,    12,    10,
      36,    35,    14,    10,    14,     4,    23,   158,    21,   159,
      82,     4,    34,   110,    74,   133
  };

  const signed char
  tcc_sy_parser::yystos_[] =
  {
       0,     7,     8,    18,    40,    41,    42,    43,    44,    67,
      42,     6,    88,     0,    41,    67,    45,    46,    47,    48,
      88,     9,    10,     9,    10,    48,    49,    50,    51,    88,
      13,    11,    15,    11,    13,    15,    49,    45,    88,    15,
      15,    14,    42,    68,    69,    71,    72,     5,    13,    24,
      25,    37,    55,    60,    61,    62,    63,    64,    66,    83,
      86,    88,    16,    53,    55,    14,    68,    52,    60,    53,
      52,    16,    73,    88,     9,    14,    11,    11,    55,    12,
      24,    25,    84,    26,    27,    28,    85,    11,    65,    62,
      13,    17,    52,    53,    54,    12,    73,    14,    10,    17,
      19,    20,    21,    22,    23,    41,    42,    55,    66,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    69,    73,
      12,    55,    14,    61,    62,    55,    14,    60,    70,     9,
      17,    73,    13,    13,    10,    10,    10,    55,    10,    15,
      17,    75,    12,    12,     9,    14,    52,    53,    56,    57,
      58,    59,    60,    82,    82,    10,    55,    60,    36,    35,
      29,    30,    31,    32,    33,    34,    87,    14,    14,    10,
      57,    58,    59,    59,    60,    76,    76,     4,    76
  };

  const signed char
  tcc_sy_parser::yyr1_[] =
  {
       0,    39,    40,    40,    40,    40,    41,    41,    42,    43,
      43,    44,    44,    45,    45,    46,    46,    47,    47,    48,
      48,    49,    49,    50,    51,    52,    53,    53,    54,    54,
      54,    54,    55,    56,    56,    57,    57,    58,    58,    58,
      59,    59,    60,    60,    61,    61,    62,    62,    62,    63,
      63,    64,    64,    64,    65,    66,    66,    67,    67,    67,
      67,    68,    68,    69,    69,    70,    70,    71,    72,    72,
      73,    73,    74,    74,    75,    75,    76,    76,    76,    76,
      76,    76,    76,    76,    76,    77,    77,    78,    78,    79,
      80,    81,    82,    83,    84,    84,    85,    85,    85,    86,
      86,    86,    87,    87,    87,    87,    88
  };

  const signed char
  tcc_sy_parser::yyr2_[] =
  {
       0,     2,     2,     2,     1,     1,     2,     2,     1,     3,
       3,     2,     3,     1,     1,     3,     1,     3,     1,     4,
       4,     1,     1,     3,     3,     1,     3,     2,     3,     3,
       1,     1,     1,     3,     1,     1,     3,     1,     3,     3,
       1,     3,     1,     3,     1,     3,     1,     1,     2,     4,
       3,     3,     1,     1,     3,     2,     1,     6,     5,     6,
       5,     3,     1,     1,     1,     3,     1,     2,     3,     4,
       2,     3,     1,     2,     1,     1,     4,     2,     1,     1,
       1,     1,     1,     1,     1,     7,     5,     3,     2,     5,
       2,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1
  };


#if YYDEBUG || 1
  // YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
  // First, the terminals, then, starting at \a YYNTOKENS, nonterminals.
  const char*
  const tcc_sy_parser::yytname_[] =
  {
  "\"end of file\"", "error", "\"invalid token\"", "\"then\"", "\"else\"",
  "\"intconst\"", "\"identifier\"", "\"const\"", "\"int\"", "\",\"",
  "\";\"", "\"[\"", "\"]\"", "\"(\"", "\")\"", "\"=\"", "\"{\"", "\"}\"",
  "\"void\"", "\"if\"", "\"while\"", "\"break\"", "\"continue\"",
  "\"return\"", "\"+\"", "\"-\"", "\"*\"", "\"/\"", "\"%\"", "\">\"",
  "\">=\"", "\"<\"", "\"<=\"", "\"==\"", "\"!=\"", "\"&&\"", "\"||\"",
  "\"!\"", "\"//\"", "$accept", "CompUnit", "Decl", "BType", "ConstDecl",
  "VarDecl", "VarDef", "DefSingleElem", "DefArray", "ArrayBody",
  "ConstDef", "ConstDefSingleElem", "ConstDefArray", "InitVal",
  "InitValArray", "InitValArrayInner", "Exp", "LOrExp", "LAndExp", "EqExp",
  "RelExp", "AddExp", "MulExp", "UnaryExp", "FuncCall", "PrimaryExp",
  "ArrayItem", "LVal", "FuncDef", "FuncFParams", "FuncFParam",
  "FuncRParams", "FuncFSingleParam", "FuncFParamArray", "Block",
  "BlockItems", "BlockItem", "Stmt", "IfStmt", "ReturnStmt", "WhileStmt",
  "BreakStmt", "ContinueStmt", "Cond", "Number", "AddOp", "MulOp",
  "UnaryOp", "RelOp", "IDENT", YY_NULLPTR
  };
#endif


#if YYDEBUG
  const short
  tcc_sy_parser::yyrline_[] =
  {
       0,   104,   104,   105,   106,   107,   110,   111,   114,   116,
     117,   120,   121,   124,   125,   128,   129,   132,   133,   136,
     137,   140,   141,   144,   146,   149,   151,   152,   155,   156,
     157,   158,   161,   163,   164,   167,   168,   171,   172,   173,
     176,   177,   180,   181,   184,   185,   188,   189,   190,   193,
     194,   197,   198,   199,   202,   204,   205,   208,   209,   210,
     211,   214,   215,   218,   219,   222,   223,   226,   228,   229,
     232,   233,   236,   237,   240,   241,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   255,   256,   259,   260,   263,
     265,   267,   269,   271,   273,   274,   277,   278,   279,   282,
     283,   284,   287,   288,   289,   290,   293
  };

  void
  tcc_sy_parser::yy_stack_print_ () const
  {
    *yycdebug_ << "Stack now";
    for (stack_type::const_iterator
           i = yystack_.begin (),
           i_end = yystack_.end ();
         i != i_end; ++i)
      *yycdebug_ << ' ' << int (i->state);
    *yycdebug_ << '\n';
  }

  void
  tcc_sy_parser::yy_reduce_print_ (int yyrule) const
  {
    int yylno = yyrline_[yyrule];
    int yynrhs = yyr2_[yyrule];
    // Print the symbols being reduced, and their result.
    *yycdebug_ << "Reducing stack by rule " << yyrule - 1
               << " (line " << yylno << "):\n";
    // The symbols being reduced.
    for (int yyi = 0; yyi < yynrhs; yyi++)
      YY_SYMBOL_PRINT ("   $" << yyi + 1 << " =",
                       yystack_[(yynrhs) - (yyi + 1)]);
  }
#endif // YYDEBUG


} // yy
#line 2372 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.cpp"

#line 298 "src/parser/parser.yy"

void
yy::tcc_sy_parser::error (const location_type& l,
                          const std::string& m)
{
  driver.error (l, m);
}
