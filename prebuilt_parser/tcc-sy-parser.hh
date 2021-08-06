// A Bison parser, made by GNU Bison 3.7.4.

// Skeleton interface for Bison LALR(1) parsers in C++

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


/**
 ** \file /home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.hh
 ** Define the yy::parser class.
 */

// C++ LALR(1) parser skeleton written by Akim Demaille.

// DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
// especially those whose name start with YY_ or yy_.  They are
// private implementation details that can be changed or removed.

#ifndef YY_YY_HOME_GCH981213_SRC_CSCCC_TINBACCC_CMAKE_BUILD_DEBUG_TCC_SY_PARSER_HH_INCLUDED
# define YY_YY_HOME_GCH981213_SRC_CSCCC_TINBACCC_CMAKE_BUILD_DEBUG_TCC_SY_PARSER_HH_INCLUDED
// "%code requires" blocks.
#line 9 "src/parser/parser.yy"

# include <string>
# include <vector>
# include <algorithm>
# include <ast/ast.h>
using std::string;
using std::vector;
class tcc_sy_driver;

#line 59 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.hh"

# include <cassert>
# include <cstdlib> // std::abort
# include <iostream>
# include <stdexcept>
# include <string>
# include <vector>

#if defined __cplusplus
# define YY_CPLUSPLUS __cplusplus
#else
# define YY_CPLUSPLUS 199711L
#endif

// Support move semantics when possible.
#if 201103L <= YY_CPLUSPLUS
# define YY_MOVE           std::move
# define YY_MOVE_OR_COPY   move
# define YY_MOVE_REF(Type) Type&&
# define YY_RVREF(Type)    Type&&
# define YY_COPY(Type)     Type
#else
# define YY_MOVE
# define YY_MOVE_OR_COPY   copy
# define YY_MOVE_REF(Type) Type&
# define YY_RVREF(Type)    const Type&
# define YY_COPY(Type)     const Type&
#endif

// Support noexcept when possible.
#if 201103L <= YY_CPLUSPLUS
# define YY_NOEXCEPT noexcept
# define YY_NOTHROW
#else
# define YY_NOEXCEPT
# define YY_NOTHROW throw ()
#endif

// Support constexpr when possible.
#if 201703 <= YY_CPLUSPLUS
# define YY_CONSTEXPR constexpr
#else
# define YY_CONSTEXPR
#endif
# include "location.hh"
#include <typeinfo>
#ifndef YY_ASSERT
# include <cassert>
# define YY_ASSERT assert
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && ! defined __ICC && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                            \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif

namespace yy {
#line 193 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.hh"




  /// A Bison parser.
  class tcc_sy_parser
  {
  public:
#ifndef YYSTYPE
  /// A buffer to store and retrieve objects.
  ///
  /// Sort of a variant, but does not keep track of the nature
  /// of the stored data, since that knowledge is available
  /// via the current parser state.
  class semantic_type
  {
  public:
    /// Type of *this.
    typedef semantic_type self_type;

    /// Empty construction.
    semantic_type () YY_NOEXCEPT
      : yybuffer_ ()
      , yytypeid_ (YY_NULLPTR)
    {}

    /// Construct and fill.
    template <typename T>
    semantic_type (YY_RVREF (T) t)
      : yytypeid_ (&typeid (T))
    {
      YY_ASSERT (sizeof (T) <= size);
      new (yyas_<T> ()) T (YY_MOVE (t));
    }

#if 201103L <= YY_CPLUSPLUS
    /// Non copyable.
    semantic_type (const self_type&) = delete;
    /// Non copyable.
    self_type& operator= (const self_type&) = delete;
#endif

    /// Destruction, allowed only if empty.
    ~semantic_type () YY_NOEXCEPT
    {
      YY_ASSERT (!yytypeid_);
    }

# if 201103L <= YY_CPLUSPLUS
    /// Instantiate a \a T in here from \a t.
    template <typename T, typename... U>
    T&
    emplace (U&&... u)
    {
      YY_ASSERT (!yytypeid_);
      YY_ASSERT (sizeof (T) <= size);
      yytypeid_ = & typeid (T);
      return *new (yyas_<T> ()) T (std::forward <U>(u)...);
    }
# else
    /// Instantiate an empty \a T in here.
    template <typename T>
    T&
    emplace ()
    {
      YY_ASSERT (!yytypeid_);
      YY_ASSERT (sizeof (T) <= size);
      yytypeid_ = & typeid (T);
      return *new (yyas_<T> ()) T ();
    }

    /// Instantiate a \a T in here from \a t.
    template <typename T>
    T&
    emplace (const T& t)
    {
      YY_ASSERT (!yytypeid_);
      YY_ASSERT (sizeof (T) <= size);
      yytypeid_ = & typeid (T);
      return *new (yyas_<T> ()) T (t);
    }
# endif

    /// Instantiate an empty \a T in here.
    /// Obsolete, use emplace.
    template <typename T>
    T&
    build ()
    {
      return emplace<T> ();
    }

    /// Instantiate a \a T in here from \a t.
    /// Obsolete, use emplace.
    template <typename T>
    T&
    build (const T& t)
    {
      return emplace<T> (t);
    }

    /// Accessor to a built \a T.
    template <typename T>
    T&
    as () YY_NOEXCEPT
    {
      YY_ASSERT (yytypeid_);
      YY_ASSERT (*yytypeid_ == typeid (T));
      YY_ASSERT (sizeof (T) <= size);
      return *yyas_<T> ();
    }

    /// Const accessor to a built \a T (for %printer).
    template <typename T>
    const T&
    as () const YY_NOEXCEPT
    {
      YY_ASSERT (yytypeid_);
      YY_ASSERT (*yytypeid_ == typeid (T));
      YY_ASSERT (sizeof (T) <= size);
      return *yyas_<T> ();
    }

    /// Swap the content with \a that, of same type.
    ///
    /// Both variants must be built beforehand, because swapping the actual
    /// data requires reading it (with as()), and this is not possible on
    /// unconstructed variants: it would require some dynamic testing, which
    /// should not be the variant's responsibility.
    /// Swapping between built and (possibly) non-built is done with
    /// self_type::move ().
    template <typename T>
    void
    swap (self_type& that) YY_NOEXCEPT
    {
      YY_ASSERT (yytypeid_);
      YY_ASSERT (*yytypeid_ == *that.yytypeid_);
      std::swap (as<T> (), that.as<T> ());
    }

    /// Move the content of \a that to this.
    ///
    /// Destroys \a that.
    template <typename T>
    void
    move (self_type& that)
    {
# if 201103L <= YY_CPLUSPLUS
      emplace<T> (std::move (that.as<T> ()));
# else
      emplace<T> ();
      swap<T> (that);
# endif
      that.destroy<T> ();
    }

# if 201103L <= YY_CPLUSPLUS
    /// Move the content of \a that to this.
    template <typename T>
    void
    move (self_type&& that)
    {
      emplace<T> (std::move (that.as<T> ()));
      that.destroy<T> ();
    }
#endif

    /// Copy the content of \a that to this.
    template <typename T>
    void
    copy (const self_type& that)
    {
      emplace<T> (that.as<T> ());
    }

    /// Destroy the stored \a T.
    template <typename T>
    void
    destroy ()
    {
      as<T> ().~T ();
      yytypeid_ = YY_NULLPTR;
    }

  private:
#if YY_CPLUSPLUS < 201103L
    /// Non copyable.
    semantic_type (const self_type&);
    /// Non copyable.
    self_type& operator= (const self_type&);
#endif

    /// Accessor to raw memory as \a T.
    template <typename T>
    T*
    yyas_ () YY_NOEXCEPT
    {
      void *yyp = yybuffer_.yyraw;
      return static_cast<T*> (yyp);
     }

    /// Const accessor to raw memory as \a T.
    template <typename T>
    const T*
    yyas_ () const YY_NOEXCEPT
    {
      const void *yyp = yybuffer_.yyraw;
      return static_cast<const T*> (yyp);
     }

    /// An auxiliary type to compute the largest semantic type.
    union union_type
    {
      // Block
      // BlockItems
      char dummy1[sizeof (ast::Block *)];

      // BreakStmt
      char dummy2[sizeof (ast::BreakStmt *)];

      // Cond
      char dummy3[sizeof (ast::Cond *)];

      // ContinueStmt
      char dummy4[sizeof (ast::ContinueStmt *)];

      // VarDef
      // DefSingleElem
      // DefArray
      // ArrayBody
      // ConstDef
      // ConstDefSingleElem
      // ConstDefArray
      char dummy5[sizeof (ast::Decl *)];

      // BType
      char dummy6[sizeof (ast::Decl::VarType)];

      // Exp
      // LOrExp
      // LAndExp
      // EqExp
      // RelExp
      // AddExp
      // MulExp
      // UnaryExp
      // PrimaryExp
      // ArrayItem
      // Number
      char dummy7[sizeof (ast::Exp *)];

      // AddOp
      // MulOp
      // UnaryOp
      // RelOp
      char dummy8[sizeof (ast::Exp::Op)];

      // FuncCall
      char dummy9[sizeof (ast::FuncCall *)];

      // FuncFParam
      // FuncFSingleParam
      // FuncFParamArray
      char dummy10[sizeof (ast::FuncFParam *)];

      // FuncDef
      char dummy11[sizeof (ast::Function *)];

      // IfStmt
      char dummy12[sizeof (ast::IfStmt *)];

      // InitVal
      // InitValArray
      // InitValArrayInner
      char dummy13[sizeof (ast::InitVal *)];

      // LVal
      char dummy14[sizeof (ast::LVal *)];

      // ReturnStmt
      char dummy15[sizeof (ast::ReturnStmt *)];

      // Stmt
      char dummy16[sizeof (ast::Stmt *)];

      // WhileStmt
      char dummy17[sizeof (ast::WhileStmt *)];

      // "intconst"
      char dummy18[sizeof (int)];

      // "identifier"
      // IDENT
      char dummy19[sizeof (string)];

      // Decl
      // ConstDecl
      // VarDecl
      char dummy20[sizeof (vector<ast::Decl *>)];

      // FuncRParams
      char dummy21[sizeof (vector<ast::Exp *>)];

      // FuncFParams
      char dummy22[sizeof (vector<ast::FuncFParam *>)];

      // BlockItem
      char dummy23[sizeof (vector<ast::Node *>)];
    };

    /// The size of the largest semantic type.
    enum { size = sizeof (union_type) };

    /// A buffer to store semantic values.
    union
    {
      /// Strongest alignment constraints.
      long double yyalign_me;
      /// A buffer large enough to store any of the semantic values.
      char yyraw[size];
    } yybuffer_;

    /// Whether the content is built: if defined, the name of the stored type.
    const std::type_info *yytypeid_;
  };

#else
    typedef YYSTYPE semantic_type;
#endif
    /// Symbol locations.
    typedef location location_type;

    /// Syntax errors thrown from user actions.
    struct syntax_error : std::runtime_error
    {
      syntax_error (const location_type& l, const std::string& m)
        : std::runtime_error (m)
        , location (l)
      {}

      syntax_error (const syntax_error& s)
        : std::runtime_error (s.what ())
        , location (s.location)
      {}

      ~syntax_error () YY_NOEXCEPT YY_NOTHROW;

      location_type location;
    };

    /// Token kinds.
    struct token
    {
      enum token_kind_type
      {
        TOK_YYEMPTY = -2,
    TOK_END = 0,                   // "end of file"
    TOK_YYerror = 256,             // error
    TOK_YYUNDEF = 257,             // "invalid token"
    TOK_ELSE = 259,                // "else"
    TOK_INTCONST = 260,            // "intconst"
    TOK_IDENTIFIER = 261,          // "identifier"
    TOK_CONST = 262,               // "const"
    TOK_INT = 263,                 // "int"
    TOK_COMMA = 264,               // ","
    TOK_SEMI = 265,                // ";"
    TOK_LSQUARE = 266,             // "["
    TOK_RSQUARE = 267,             // "]"
    TOK_LBRACE = 268,              // "("
    TOK_RBRACE = 269,              // ")"
    TOK_ASSIGN = 270,              // "="
    TOK_LBBRACE = 271,             // "{"
    TOK_RBBRACE = 272,             // "}"
    TOK_VOID = 273,                // "void"
    TOK_IF = 274,                  // "if"
    TOK_WHILE = 275,               // "while"
    TOK_BREAK = 276,               // "break"
    TOK_CONTINUE = 277,            // "continue"
    TOK_RETURN = 278,              // "return"
    TOK_PLUS = 279,                // "+"
    TOK_MINUS = 280,               // "-"
    TOK_MUL = 281,                 // "*"
    TOK_DIVIDE = 282,              // "/"
    TOK_MOD = 283,                 // "%"
    TOK_GREATER = 284,             // ">"
    TOK_GREATEREQ = 285,           // ">="
    TOK_SMALLER = 286,             // "<"
    TOK_SMALLEREQ = 287,           // "<="
    TOK_EQUAL = 288,               // "=="
    TOK_NOTEQUAL = 289,            // "!="
    TOK_AND = 290,                 // "&&"
    TOK_OR = 291,                  // "||"
    TOK_NOT = 292,                 // "!"
    TOK_SINGLECOMMENT = 293        // "//"
      };
      /// Backward compatibility alias (Bison 3.6).
      typedef token_kind_type yytokentype;
    };

    /// Token kind, as returned by yylex.
    typedef token::yytokentype token_kind_type;

    /// Backward compatibility alias (Bison 3.6).
    typedef token_kind_type token_type;

    /// Symbol kinds.
    struct symbol_kind
    {
      enum symbol_kind_type
      {
        YYNTOKENS = 39, ///< Number of tokens.
        S_YYEMPTY = -2,
        S_YYEOF = 0,                             // "end of file"
        S_YYerror = 1,                           // error
        S_YYUNDEF = 2,                           // "invalid token"
        S_3_then_ = 3,                           // "then"
        S_ELSE = 4,                              // "else"
        S_INTCONST = 5,                          // "intconst"
        S_IDENTIFIER = 6,                        // "identifier"
        S_CONST = 7,                             // "const"
        S_INT = 8,                               // "int"
        S_COMMA = 9,                             // ","
        S_SEMI = 10,                             // ";"
        S_LSQUARE = 11,                          // "["
        S_RSQUARE = 12,                          // "]"
        S_LBRACE = 13,                           // "("
        S_RBRACE = 14,                           // ")"
        S_ASSIGN = 15,                           // "="
        S_LBBRACE = 16,                          // "{"
        S_RBBRACE = 17,                          // "}"
        S_VOID = 18,                             // "void"
        S_IF = 19,                               // "if"
        S_WHILE = 20,                            // "while"
        S_BREAK = 21,                            // "break"
        S_CONTINUE = 22,                         // "continue"
        S_RETURN = 23,                           // "return"
        S_PLUS = 24,                             // "+"
        S_MINUS = 25,                            // "-"
        S_MUL = 26,                              // "*"
        S_DIVIDE = 27,                           // "/"
        S_MOD = 28,                              // "%"
        S_GREATER = 29,                          // ">"
        S_GREATEREQ = 30,                        // ">="
        S_SMALLER = 31,                          // "<"
        S_SMALLEREQ = 32,                        // "<="
        S_EQUAL = 33,                            // "=="
        S_NOTEQUAL = 34,                         // "!="
        S_AND = 35,                              // "&&"
        S_OR = 36,                               // "||"
        S_NOT = 37,                              // "!"
        S_SINGLECOMMENT = 38,                    // "//"
        S_YYACCEPT = 39,                         // $accept
        S_CompUnit = 40,                         // CompUnit
        S_Decl = 41,                             // Decl
        S_BType = 42,                            // BType
        S_ConstDecl = 43,                        // ConstDecl
        S_VarDecl = 44,                          // VarDecl
        S_VarDef = 45,                           // VarDef
        S_DefSingleElem = 46,                    // DefSingleElem
        S_DefArray = 47,                         // DefArray
        S_ArrayBody = 48,                        // ArrayBody
        S_ConstDef = 49,                         // ConstDef
        S_ConstDefSingleElem = 50,               // ConstDefSingleElem
        S_ConstDefArray = 51,                    // ConstDefArray
        S_InitVal = 52,                          // InitVal
        S_InitValArray = 53,                     // InitValArray
        S_InitValArrayInner = 54,                // InitValArrayInner
        S_Exp = 55,                              // Exp
        S_LOrExp = 56,                           // LOrExp
        S_LAndExp = 57,                          // LAndExp
        S_EqExp = 58,                            // EqExp
        S_RelExp = 59,                           // RelExp
        S_AddExp = 60,                           // AddExp
        S_MulExp = 61,                           // MulExp
        S_UnaryExp = 62,                         // UnaryExp
        S_FuncCall = 63,                         // FuncCall
        S_PrimaryExp = 64,                       // PrimaryExp
        S_ArrayItem = 65,                        // ArrayItem
        S_LVal = 66,                             // LVal
        S_FuncDef = 67,                          // FuncDef
        S_FuncFParams = 68,                      // FuncFParams
        S_FuncFParam = 69,                       // FuncFParam
        S_FuncRParams = 70,                      // FuncRParams
        S_FuncFSingleParam = 71,                 // FuncFSingleParam
        S_FuncFParamArray = 72,                  // FuncFParamArray
        S_Block = 73,                            // Block
        S_BlockItems = 74,                       // BlockItems
        S_BlockItem = 75,                        // BlockItem
        S_Stmt = 76,                             // Stmt
        S_IfStmt = 77,                           // IfStmt
        S_ReturnStmt = 78,                       // ReturnStmt
        S_WhileStmt = 79,                        // WhileStmt
        S_BreakStmt = 80,                        // BreakStmt
        S_ContinueStmt = 81,                     // ContinueStmt
        S_Cond = 82,                             // Cond
        S_Number = 83,                           // Number
        S_AddOp = 84,                            // AddOp
        S_MulOp = 85,                            // MulOp
        S_UnaryOp = 86,                          // UnaryOp
        S_RelOp = 87,                            // RelOp
        S_IDENT = 88                             // IDENT
      };
    };

    /// (Internal) symbol kind.
    typedef symbol_kind::symbol_kind_type symbol_kind_type;

    /// The number of tokens.
    static const symbol_kind_type YYNTOKENS = symbol_kind::YYNTOKENS;

    /// A complete symbol.
    ///
    /// Expects its Base type to provide access to the symbol kind
    /// via kind ().
    ///
    /// Provide access to semantic value and location.
    template <typename Base>
    struct basic_symbol : Base
    {
      /// Alias to Base.
      typedef Base super_type;

      /// Default constructor.
      basic_symbol ()
        : value ()
        , location ()
      {}

#if 201103L <= YY_CPLUSPLUS
      /// Move constructor.
      basic_symbol (basic_symbol&& that)
        : Base (std::move (that))
        , value ()
        , location (std::move (that.location))
      {
        switch (this->kind ())
    {
      case symbol_kind::S_Block: // Block
      case symbol_kind::S_BlockItems: // BlockItems
        value.move< ast::Block * > (std::move (that.value));
        break;

      case symbol_kind::S_BreakStmt: // BreakStmt
        value.move< ast::BreakStmt * > (std::move (that.value));
        break;

      case symbol_kind::S_Cond: // Cond
        value.move< ast::Cond * > (std::move (that.value));
        break;

      case symbol_kind::S_ContinueStmt: // ContinueStmt
        value.move< ast::ContinueStmt * > (std::move (that.value));
        break;

      case symbol_kind::S_VarDef: // VarDef
      case symbol_kind::S_DefSingleElem: // DefSingleElem
      case symbol_kind::S_DefArray: // DefArray
      case symbol_kind::S_ArrayBody: // ArrayBody
      case symbol_kind::S_ConstDef: // ConstDef
      case symbol_kind::S_ConstDefSingleElem: // ConstDefSingleElem
      case symbol_kind::S_ConstDefArray: // ConstDefArray
        value.move< ast::Decl * > (std::move (that.value));
        break;

      case symbol_kind::S_BType: // BType
        value.move< ast::Decl::VarType > (std::move (that.value));
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
        value.move< ast::Exp * > (std::move (that.value));
        break;

      case symbol_kind::S_AddOp: // AddOp
      case symbol_kind::S_MulOp: // MulOp
      case symbol_kind::S_UnaryOp: // UnaryOp
      case symbol_kind::S_RelOp: // RelOp
        value.move< ast::Exp::Op > (std::move (that.value));
        break;

      case symbol_kind::S_FuncCall: // FuncCall
        value.move< ast::FuncCall * > (std::move (that.value));
        break;

      case symbol_kind::S_FuncFParam: // FuncFParam
      case symbol_kind::S_FuncFSingleParam: // FuncFSingleParam
      case symbol_kind::S_FuncFParamArray: // FuncFParamArray
        value.move< ast::FuncFParam * > (std::move (that.value));
        break;

      case symbol_kind::S_FuncDef: // FuncDef
        value.move< ast::Function * > (std::move (that.value));
        break;

      case symbol_kind::S_IfStmt: // IfStmt
        value.move< ast::IfStmt * > (std::move (that.value));
        break;

      case symbol_kind::S_InitVal: // InitVal
      case symbol_kind::S_InitValArray: // InitValArray
      case symbol_kind::S_InitValArrayInner: // InitValArrayInner
        value.move< ast::InitVal * > (std::move (that.value));
        break;

      case symbol_kind::S_LVal: // LVal
        value.move< ast::LVal * > (std::move (that.value));
        break;

      case symbol_kind::S_ReturnStmt: // ReturnStmt
        value.move< ast::ReturnStmt * > (std::move (that.value));
        break;

      case symbol_kind::S_Stmt: // Stmt
        value.move< ast::Stmt * > (std::move (that.value));
        break;

      case symbol_kind::S_WhileStmt: // WhileStmt
        value.move< ast::WhileStmt * > (std::move (that.value));
        break;

      case symbol_kind::S_INTCONST: // "intconst"
        value.move< int > (std::move (that.value));
        break;

      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_IDENT: // IDENT
        value.move< string > (std::move (that.value));
        break;

      case symbol_kind::S_Decl: // Decl
      case symbol_kind::S_ConstDecl: // ConstDecl
      case symbol_kind::S_VarDecl: // VarDecl
        value.move< vector<ast::Decl *> > (std::move (that.value));
        break;

      case symbol_kind::S_FuncRParams: // FuncRParams
        value.move< vector<ast::Exp *> > (std::move (that.value));
        break;

      case symbol_kind::S_FuncFParams: // FuncFParams
        value.move< vector<ast::FuncFParam *> > (std::move (that.value));
        break;

      case symbol_kind::S_BlockItem: // BlockItem
        value.move< vector<ast::Node *> > (std::move (that.value));
        break;

      default:
        break;
    }

      }
#endif

      /// Copy constructor.
      basic_symbol (const basic_symbol& that);

      /// Constructors for typed symbols.
#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, location_type&& l)
        : Base (t)
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const location_type& l)
        : Base (t)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, ast::Block *&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const ast::Block *& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, ast::BreakStmt *&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const ast::BreakStmt *& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, ast::Cond *&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const ast::Cond *& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, ast::ContinueStmt *&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const ast::ContinueStmt *& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, ast::Decl *&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const ast::Decl *& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, ast::Decl::VarType&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const ast::Decl::VarType& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, ast::Exp *&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const ast::Exp *& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, ast::Exp::Op&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const ast::Exp::Op& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, ast::FuncCall *&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const ast::FuncCall *& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, ast::FuncFParam *&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const ast::FuncFParam *& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, ast::Function *&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const ast::Function *& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, ast::IfStmt *&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const ast::IfStmt *& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, ast::InitVal *&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const ast::InitVal *& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, ast::LVal *&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const ast::LVal *& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, ast::ReturnStmt *&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const ast::ReturnStmt *& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, ast::Stmt *&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const ast::Stmt *& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, ast::WhileStmt *&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const ast::WhileStmt *& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, int&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const int& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, string&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const string& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, vector<ast::Decl *>&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const vector<ast::Decl *>& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, vector<ast::Exp *>&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const vector<ast::Exp *>& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, vector<ast::FuncFParam *>&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const vector<ast::FuncFParam *>& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

#if 201103L <= YY_CPLUSPLUS
      basic_symbol (typename Base::kind_type t, vector<ast::Node *>&& v, location_type&& l)
        : Base (t)
        , value (std::move (v))
        , location (std::move (l))
      {}
#else
      basic_symbol (typename Base::kind_type t, const vector<ast::Node *>& v, const location_type& l)
        : Base (t)
        , value (v)
        , location (l)
      {}
#endif

      /// Destroy the symbol.
      ~basic_symbol ()
      {
        clear ();
      }

      /// Destroy contents, and record that is empty.
      void clear ()
      {
        // User destructor.
        symbol_kind_type yykind = this->kind ();
        basic_symbol<Base>& yysym = *this;
        (void) yysym;
        switch (yykind)
        {
       default:
          break;
        }

        // Value type destructor.
switch (yykind)
    {
      case symbol_kind::S_Block: // Block
      case symbol_kind::S_BlockItems: // BlockItems
        value.template destroy< ast::Block * > ();
        break;

      case symbol_kind::S_BreakStmt: // BreakStmt
        value.template destroy< ast::BreakStmt * > ();
        break;

      case symbol_kind::S_Cond: // Cond
        value.template destroy< ast::Cond * > ();
        break;

      case symbol_kind::S_ContinueStmt: // ContinueStmt
        value.template destroy< ast::ContinueStmt * > ();
        break;

      case symbol_kind::S_VarDef: // VarDef
      case symbol_kind::S_DefSingleElem: // DefSingleElem
      case symbol_kind::S_DefArray: // DefArray
      case symbol_kind::S_ArrayBody: // ArrayBody
      case symbol_kind::S_ConstDef: // ConstDef
      case symbol_kind::S_ConstDefSingleElem: // ConstDefSingleElem
      case symbol_kind::S_ConstDefArray: // ConstDefArray
        value.template destroy< ast::Decl * > ();
        break;

      case symbol_kind::S_BType: // BType
        value.template destroy< ast::Decl::VarType > ();
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
        value.template destroy< ast::Exp * > ();
        break;

      case symbol_kind::S_AddOp: // AddOp
      case symbol_kind::S_MulOp: // MulOp
      case symbol_kind::S_UnaryOp: // UnaryOp
      case symbol_kind::S_RelOp: // RelOp
        value.template destroy< ast::Exp::Op > ();
        break;

      case symbol_kind::S_FuncCall: // FuncCall
        value.template destroy< ast::FuncCall * > ();
        break;

      case symbol_kind::S_FuncFParam: // FuncFParam
      case symbol_kind::S_FuncFSingleParam: // FuncFSingleParam
      case symbol_kind::S_FuncFParamArray: // FuncFParamArray
        value.template destroy< ast::FuncFParam * > ();
        break;

      case symbol_kind::S_FuncDef: // FuncDef
        value.template destroy< ast::Function * > ();
        break;

      case symbol_kind::S_IfStmt: // IfStmt
        value.template destroy< ast::IfStmt * > ();
        break;

      case symbol_kind::S_InitVal: // InitVal
      case symbol_kind::S_InitValArray: // InitValArray
      case symbol_kind::S_InitValArrayInner: // InitValArrayInner
        value.template destroy< ast::InitVal * > ();
        break;

      case symbol_kind::S_LVal: // LVal
        value.template destroy< ast::LVal * > ();
        break;

      case symbol_kind::S_ReturnStmt: // ReturnStmt
        value.template destroy< ast::ReturnStmt * > ();
        break;

      case symbol_kind::S_Stmt: // Stmt
        value.template destroy< ast::Stmt * > ();
        break;

      case symbol_kind::S_WhileStmt: // WhileStmt
        value.template destroy< ast::WhileStmt * > ();
        break;

      case symbol_kind::S_INTCONST: // "intconst"
        value.template destroy< int > ();
        break;

      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_IDENT: // IDENT
        value.template destroy< string > ();
        break;

      case symbol_kind::S_Decl: // Decl
      case symbol_kind::S_ConstDecl: // ConstDecl
      case symbol_kind::S_VarDecl: // VarDecl
        value.template destroy< vector<ast::Decl *> > ();
        break;

      case symbol_kind::S_FuncRParams: // FuncRParams
        value.template destroy< vector<ast::Exp *> > ();
        break;

      case symbol_kind::S_FuncFParams: // FuncFParams
        value.template destroy< vector<ast::FuncFParam *> > ();
        break;

      case symbol_kind::S_BlockItem: // BlockItem
        value.template destroy< vector<ast::Node *> > ();
        break;

      default:
        break;
    }

        Base::clear ();
      }

      /// The user-facing name of this symbol.
      std::string name () const YY_NOEXCEPT
      {
        return tcc_sy_parser::symbol_name (this->kind ());
      }

      /// Backward compatibility (Bison 3.6).
      symbol_kind_type type_get () const YY_NOEXCEPT;

      /// Whether empty.
      bool empty () const YY_NOEXCEPT;

      /// Destructive move, \a s is emptied into this.
      void move (basic_symbol& s);

      /// The semantic value.
      semantic_type value;

      /// The location.
      location_type location;

    private:
#if YY_CPLUSPLUS < 201103L
      /// Assignment operator.
      basic_symbol& operator= (const basic_symbol& that);
#endif
    };

    /// Type access provider for token (enum) based symbols.
    struct by_kind
    {
      /// Default constructor.
      by_kind ();

#if 201103L <= YY_CPLUSPLUS
      /// Move constructor.
      by_kind (by_kind&& that);
#endif

      /// Copy constructor.
      by_kind (const by_kind& that);

      /// The symbol kind as needed by the constructor.
      typedef token_kind_type kind_type;

      /// Constructor from (external) token numbers.
      by_kind (kind_type t);

      /// Record that this symbol is empty.
      void clear ();

      /// Steal the symbol kind from \a that.
      void move (by_kind& that);

      /// The (internal) type number (corresponding to \a type).
      /// \a empty when empty.
      symbol_kind_type kind () const YY_NOEXCEPT;

      /// Backward compatibility (Bison 3.6).
      symbol_kind_type type_get () const YY_NOEXCEPT;

      /// The symbol kind.
      /// \a S_YYEMPTY when empty.
      symbol_kind_type kind_;
    };

    /// Backward compatibility for a private implementation detail (Bison 3.6).
    typedef by_kind by_type;

    /// "External" symbols: returned by the scanner.
    struct symbol_type : basic_symbol<by_kind>
    {
      /// Superclass.
      typedef basic_symbol<by_kind> super_type;

      /// Empty symbol.
      symbol_type () {}

      /// Constructor for valueless symbols, and symbols from each type.
#if 201103L <= YY_CPLUSPLUS
      symbol_type (int tok, location_type l)
        : super_type(token_type (tok), std::move (l))
#else
      symbol_type (int tok, const location_type& l)
        : super_type(token_type (tok), l)
#endif
      {
        YY_ASSERT (tok == token::TOK_END
                   || (token::TOK_YYerror <= tok && tok <= token::TOK_ELSE)
                   || (token::TOK_CONST <= tok && tok <= token::TOK_SINGLECOMMENT));
      }
#if 201103L <= YY_CPLUSPLUS
      symbol_type (int tok, int v, location_type l)
        : super_type(token_type (tok), std::move (v), std::move (l))
#else
      symbol_type (int tok, const int& v, const location_type& l)
        : super_type(token_type (tok), v, l)
#endif
      {
        YY_ASSERT (tok == token::TOK_INTCONST);
      }
#if 201103L <= YY_CPLUSPLUS
      symbol_type (int tok, string v, location_type l)
        : super_type(token_type (tok), std::move (v), std::move (l))
#else
      symbol_type (int tok, const string& v, const location_type& l)
        : super_type(token_type (tok), v, l)
#endif
      {
        YY_ASSERT (tok == token::TOK_IDENTIFIER);
      }
    };

    /// Build a parser object.
    tcc_sy_parser (tcc_sy_driver & driver_yyarg);
    virtual ~tcc_sy_parser ();

#if 201103L <= YY_CPLUSPLUS
    /// Non copyable.
    tcc_sy_parser (const tcc_sy_parser&) = delete;
    /// Non copyable.
    tcc_sy_parser& operator= (const tcc_sy_parser&) = delete;
#endif

    /// Parse.  An alias for parse ().
    /// \returns  0 iff parsing succeeded.
    int operator() ();

    /// Parse.
    /// \returns  0 iff parsing succeeded.
    virtual int parse ();

#if YYDEBUG
    /// The current debugging stream.
    std::ostream& debug_stream () const YY_ATTRIBUTE_PURE;
    /// Set the current debugging stream.
    void set_debug_stream (std::ostream &);

    /// Type for debugging levels.
    typedef int debug_level_type;
    /// The current debugging level.
    debug_level_type debug_level () const YY_ATTRIBUTE_PURE;
    /// Set the current debugging level.
    void set_debug_level (debug_level_type l);
#endif

    /// Report a syntax error.
    /// \param loc    where the syntax error is found.
    /// \param msg    a description of the syntax error.
    virtual void error (const location_type& loc, const std::string& msg);

    /// Report a syntax error.
    void error (const syntax_error& err);

    /// The user-facing name of the symbol whose (internal) number is
    /// YYSYMBOL.  No bounds checking.
    static std::string symbol_name (symbol_kind_type yysymbol);

    // Implementation of make_symbol for each symbol type.
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_END (location_type l)
      {
        return symbol_type (token::TOK_END, std::move (l));
      }
#else
      static
      symbol_type
      make_END (const location_type& l)
      {
        return symbol_type (token::TOK_END, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_YYerror (location_type l)
      {
        return symbol_type (token::TOK_YYerror, std::move (l));
      }
#else
      static
      symbol_type
      make_YYerror (const location_type& l)
      {
        return symbol_type (token::TOK_YYerror, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_YYUNDEF (location_type l)
      {
        return symbol_type (token::TOK_YYUNDEF, std::move (l));
      }
#else
      static
      symbol_type
      make_YYUNDEF (const location_type& l)
      {
        return symbol_type (token::TOK_YYUNDEF, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_ELSE (location_type l)
      {
        return symbol_type (token::TOK_ELSE, std::move (l));
      }
#else
      static
      symbol_type
      make_ELSE (const location_type& l)
      {
        return symbol_type (token::TOK_ELSE, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_INTCONST (int v, location_type l)
      {
        return symbol_type (token::TOK_INTCONST, std::move (v), std::move (l));
      }
#else
      static
      symbol_type
      make_INTCONST (const int& v, const location_type& l)
      {
        return symbol_type (token::TOK_INTCONST, v, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_IDENTIFIER (string v, location_type l)
      {
        return symbol_type (token::TOK_IDENTIFIER, std::move (v), std::move (l));
      }
#else
      static
      symbol_type
      make_IDENTIFIER (const string& v, const location_type& l)
      {
        return symbol_type (token::TOK_IDENTIFIER, v, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_CONST (location_type l)
      {
        return symbol_type (token::TOK_CONST, std::move (l));
      }
#else
      static
      symbol_type
      make_CONST (const location_type& l)
      {
        return symbol_type (token::TOK_CONST, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_INT (location_type l)
      {
        return symbol_type (token::TOK_INT, std::move (l));
      }
#else
      static
      symbol_type
      make_INT (const location_type& l)
      {
        return symbol_type (token::TOK_INT, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_COMMA (location_type l)
      {
        return symbol_type (token::TOK_COMMA, std::move (l));
      }
#else
      static
      symbol_type
      make_COMMA (const location_type& l)
      {
        return symbol_type (token::TOK_COMMA, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_SEMI (location_type l)
      {
        return symbol_type (token::TOK_SEMI, std::move (l));
      }
#else
      static
      symbol_type
      make_SEMI (const location_type& l)
      {
        return symbol_type (token::TOK_SEMI, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_LSQUARE (location_type l)
      {
        return symbol_type (token::TOK_LSQUARE, std::move (l));
      }
#else
      static
      symbol_type
      make_LSQUARE (const location_type& l)
      {
        return symbol_type (token::TOK_LSQUARE, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_RSQUARE (location_type l)
      {
        return symbol_type (token::TOK_RSQUARE, std::move (l));
      }
#else
      static
      symbol_type
      make_RSQUARE (const location_type& l)
      {
        return symbol_type (token::TOK_RSQUARE, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_LBRACE (location_type l)
      {
        return symbol_type (token::TOK_LBRACE, std::move (l));
      }
#else
      static
      symbol_type
      make_LBRACE (const location_type& l)
      {
        return symbol_type (token::TOK_LBRACE, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_RBRACE (location_type l)
      {
        return symbol_type (token::TOK_RBRACE, std::move (l));
      }
#else
      static
      symbol_type
      make_RBRACE (const location_type& l)
      {
        return symbol_type (token::TOK_RBRACE, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_ASSIGN (location_type l)
      {
        return symbol_type (token::TOK_ASSIGN, std::move (l));
      }
#else
      static
      symbol_type
      make_ASSIGN (const location_type& l)
      {
        return symbol_type (token::TOK_ASSIGN, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_LBBRACE (location_type l)
      {
        return symbol_type (token::TOK_LBBRACE, std::move (l));
      }
#else
      static
      symbol_type
      make_LBBRACE (const location_type& l)
      {
        return symbol_type (token::TOK_LBBRACE, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_RBBRACE (location_type l)
      {
        return symbol_type (token::TOK_RBBRACE, std::move (l));
      }
#else
      static
      symbol_type
      make_RBBRACE (const location_type& l)
      {
        return symbol_type (token::TOK_RBBRACE, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_VOID (location_type l)
      {
        return symbol_type (token::TOK_VOID, std::move (l));
      }
#else
      static
      symbol_type
      make_VOID (const location_type& l)
      {
        return symbol_type (token::TOK_VOID, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_IF (location_type l)
      {
        return symbol_type (token::TOK_IF, std::move (l));
      }
#else
      static
      symbol_type
      make_IF (const location_type& l)
      {
        return symbol_type (token::TOK_IF, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_WHILE (location_type l)
      {
        return symbol_type (token::TOK_WHILE, std::move (l));
      }
#else
      static
      symbol_type
      make_WHILE (const location_type& l)
      {
        return symbol_type (token::TOK_WHILE, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_BREAK (location_type l)
      {
        return symbol_type (token::TOK_BREAK, std::move (l));
      }
#else
      static
      symbol_type
      make_BREAK (const location_type& l)
      {
        return symbol_type (token::TOK_BREAK, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_CONTINUE (location_type l)
      {
        return symbol_type (token::TOK_CONTINUE, std::move (l));
      }
#else
      static
      symbol_type
      make_CONTINUE (const location_type& l)
      {
        return symbol_type (token::TOK_CONTINUE, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_RETURN (location_type l)
      {
        return symbol_type (token::TOK_RETURN, std::move (l));
      }
#else
      static
      symbol_type
      make_RETURN (const location_type& l)
      {
        return symbol_type (token::TOK_RETURN, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_PLUS (location_type l)
      {
        return symbol_type (token::TOK_PLUS, std::move (l));
      }
#else
      static
      symbol_type
      make_PLUS (const location_type& l)
      {
        return symbol_type (token::TOK_PLUS, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_MINUS (location_type l)
      {
        return symbol_type (token::TOK_MINUS, std::move (l));
      }
#else
      static
      symbol_type
      make_MINUS (const location_type& l)
      {
        return symbol_type (token::TOK_MINUS, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_MUL (location_type l)
      {
        return symbol_type (token::TOK_MUL, std::move (l));
      }
#else
      static
      symbol_type
      make_MUL (const location_type& l)
      {
        return symbol_type (token::TOK_MUL, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_DIVIDE (location_type l)
      {
        return symbol_type (token::TOK_DIVIDE, std::move (l));
      }
#else
      static
      symbol_type
      make_DIVIDE (const location_type& l)
      {
        return symbol_type (token::TOK_DIVIDE, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_MOD (location_type l)
      {
        return symbol_type (token::TOK_MOD, std::move (l));
      }
#else
      static
      symbol_type
      make_MOD (const location_type& l)
      {
        return symbol_type (token::TOK_MOD, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_GREATER (location_type l)
      {
        return symbol_type (token::TOK_GREATER, std::move (l));
      }
#else
      static
      symbol_type
      make_GREATER (const location_type& l)
      {
        return symbol_type (token::TOK_GREATER, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_GREATEREQ (location_type l)
      {
        return symbol_type (token::TOK_GREATEREQ, std::move (l));
      }
#else
      static
      symbol_type
      make_GREATEREQ (const location_type& l)
      {
        return symbol_type (token::TOK_GREATEREQ, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_SMALLER (location_type l)
      {
        return symbol_type (token::TOK_SMALLER, std::move (l));
      }
#else
      static
      symbol_type
      make_SMALLER (const location_type& l)
      {
        return symbol_type (token::TOK_SMALLER, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_SMALLEREQ (location_type l)
      {
        return symbol_type (token::TOK_SMALLEREQ, std::move (l));
      }
#else
      static
      symbol_type
      make_SMALLEREQ (const location_type& l)
      {
        return symbol_type (token::TOK_SMALLEREQ, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_EQUAL (location_type l)
      {
        return symbol_type (token::TOK_EQUAL, std::move (l));
      }
#else
      static
      symbol_type
      make_EQUAL (const location_type& l)
      {
        return symbol_type (token::TOK_EQUAL, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_NOTEQUAL (location_type l)
      {
        return symbol_type (token::TOK_NOTEQUAL, std::move (l));
      }
#else
      static
      symbol_type
      make_NOTEQUAL (const location_type& l)
      {
        return symbol_type (token::TOK_NOTEQUAL, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_AND (location_type l)
      {
        return symbol_type (token::TOK_AND, std::move (l));
      }
#else
      static
      symbol_type
      make_AND (const location_type& l)
      {
        return symbol_type (token::TOK_AND, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_OR (location_type l)
      {
        return symbol_type (token::TOK_OR, std::move (l));
      }
#else
      static
      symbol_type
      make_OR (const location_type& l)
      {
        return symbol_type (token::TOK_OR, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_NOT (location_type l)
      {
        return symbol_type (token::TOK_NOT, std::move (l));
      }
#else
      static
      symbol_type
      make_NOT (const location_type& l)
      {
        return symbol_type (token::TOK_NOT, l);
      }
#endif
#if 201103L <= YY_CPLUSPLUS
      static
      symbol_type
      make_SINGLECOMMENT (location_type l)
      {
        return symbol_type (token::TOK_SINGLECOMMENT, std::move (l));
      }
#else
      static
      symbol_type
      make_SINGLECOMMENT (const location_type& l)
      {
        return symbol_type (token::TOK_SINGLECOMMENT, l);
      }
#endif


    class context
    {
    public:
      context (const tcc_sy_parser& yyparser, const symbol_type& yyla);
      const symbol_type& lookahead () const { return yyla_; }
      symbol_kind_type token () const { return yyla_.kind (); }
      const location_type& location () const { return yyla_.location; }

      /// Put in YYARG at most YYARGN of the expected tokens, and return the
      /// number of tokens stored in YYARG.  If YYARG is null, return the
      /// number of expected tokens (guaranteed to be less than YYNTOKENS).
      int expected_tokens (symbol_kind_type yyarg[], int yyargn) const;

    private:
      const tcc_sy_parser& yyparser_;
      const symbol_type& yyla_;
    };

  private:
#if YY_CPLUSPLUS < 201103L
    /// Non copyable.
    tcc_sy_parser (const tcc_sy_parser&);
    /// Non copyable.
    tcc_sy_parser& operator= (const tcc_sy_parser&);
#endif


    /// Stored state numbers (used for stacks).
    typedef unsigned char state_type;

    /// The arguments of the error message.
    int yy_syntax_error_arguments_ (const context& yyctx,
                                    symbol_kind_type yyarg[], int yyargn) const;

    /// Generate an error message.
    /// \param yyctx     the context in which the error occurred.
    virtual std::string yysyntax_error_ (const context& yyctx) const;
    /// Compute post-reduction state.
    /// \param yystate   the current state
    /// \param yysym     the nonterminal to push on the stack
    static state_type yy_lr_goto_state_ (state_type yystate, int yysym);

    /// Whether the given \c yypact_ value indicates a defaulted state.
    /// \param yyvalue   the value to check
    static bool yy_pact_value_is_default_ (int yyvalue);

    /// Whether the given \c yytable_ value indicates a syntax error.
    /// \param yyvalue   the value to check
    static bool yy_table_value_is_error_ (int yyvalue);

    static const signed char yypact_ninf_;
    static const signed char yytable_ninf_;

    /// Convert a scanner token kind \a t to a symbol kind.
    /// In theory \a t should be a token_kind_type, but character literals
    /// are valid, yet not members of the token_type enum.
    static symbol_kind_type yytranslate_ (int t);

    /// Convert the symbol name \a n to a form suitable for a diagnostic.
    static std::string yytnamerr_ (const char *yystr);

    /// For a symbol, its name in clear.
    static const char* const yytname_[];


    // Tables.
    // YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
    // STATE-NUM.
    static const short yypact_[];

    // YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
    // Performed when YYTABLE does not specify something else to do.  Zero
    // means the default is an error.
    static const signed char yydefact_[];

    // YYPGOTO[NTERM-NUM].
    static const short yypgoto_[];

    // YYDEFGOTO[NTERM-NUM].
    static const short yydefgoto_[];

    // YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
    // positive, shift that token.  If negative, reduce the rule whose
    // number is the opposite.  If YYTABLE_NINF, syntax error.
    static const unsigned char yytable_[];

    static const unsigned char yycheck_[];

    // YYSTOS[STATE-NUM] -- The (internal number of the) accessing
    // symbol of state STATE-NUM.
    static const signed char yystos_[];

    // YYR1[YYN] -- Symbol number of symbol that rule YYN derives.
    static const signed char yyr1_[];

    // YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.
    static const signed char yyr2_[];


#if YYDEBUG
    // YYRLINE[YYN] -- Source line where rule number YYN was defined.
    static const short yyrline_[];
    /// Report on the debug stream that the rule \a r is going to be reduced.
    virtual void yy_reduce_print_ (int r) const;
    /// Print the state stack on the debug stream.
    virtual void yy_stack_print_ () const;

    /// Debugging level.
    int yydebug_;
    /// Debug stream.
    std::ostream* yycdebug_;

    /// \brief Display a symbol kind, value and location.
    /// \param yyo    The output stream.
    /// \param yysym  The symbol.
    template <typename Base>
    void yy_print_ (std::ostream& yyo, const basic_symbol<Base>& yysym) const;
#endif

    /// \brief Reclaim the memory associated to a symbol.
    /// \param yymsg     Why this token is reclaimed.
    ///                  If null, print nothing.
    /// \param yysym     The symbol.
    template <typename Base>
    void yy_destroy_ (const char* yymsg, basic_symbol<Base>& yysym) const;

  private:
    /// Type access provider for state based symbols.
    struct by_state
    {
      /// Default constructor.
      by_state () YY_NOEXCEPT;

      /// The symbol kind as needed by the constructor.
      typedef state_type kind_type;

      /// Constructor.
      by_state (kind_type s) YY_NOEXCEPT;

      /// Copy constructor.
      by_state (const by_state& that) YY_NOEXCEPT;

      /// Record that this symbol is empty.
      void clear () YY_NOEXCEPT;

      /// Steal the symbol kind from \a that.
      void move (by_state& that);

      /// The symbol kind (corresponding to \a state).
      /// \a symbol_kind::S_YYEMPTY when empty.
      symbol_kind_type kind () const YY_NOEXCEPT;

      /// The state number used to denote an empty symbol.
      /// We use the initial state, as it does not have a value.
      enum { empty_state = 0 };

      /// The state.
      /// \a empty when empty.
      state_type state;
    };

    /// "Internal" symbol: element of the stack.
    struct stack_symbol_type : basic_symbol<by_state>
    {
      /// Superclass.
      typedef basic_symbol<by_state> super_type;
      /// Construct an empty symbol.
      stack_symbol_type ();
      /// Move or copy construction.
      stack_symbol_type (YY_RVREF (stack_symbol_type) that);
      /// Steal the contents from \a sym to build this.
      stack_symbol_type (state_type s, YY_MOVE_REF (symbol_type) sym);
#if YY_CPLUSPLUS < 201103L
      /// Assignment, needed by push_back by some old implementations.
      /// Moves the contents of that.
      stack_symbol_type& operator= (stack_symbol_type& that);

      /// Assignment, needed by push_back by other implementations.
      /// Needed by some other old implementations.
      stack_symbol_type& operator= (const stack_symbol_type& that);
#endif
    };

    /// A stack with random access from its top.
    template <typename T, typename S = std::vector<T> >
    class stack
    {
    public:
      // Hide our reversed order.
      typedef typename S::iterator iterator;
      typedef typename S::const_iterator const_iterator;
      typedef typename S::size_type size_type;
      typedef typename std::ptrdiff_t index_type;

      stack (size_type n = 200)
        : seq_ (n)
      {}

#if 201103L <= YY_CPLUSPLUS
      /// Non copyable.
      stack (const stack&) = delete;
      /// Non copyable.
      stack& operator= (const stack&) = delete;
#endif

      /// Random access.
      ///
      /// Index 0 returns the topmost element.
      const T&
      operator[] (index_type i) const
      {
        return seq_[size_type (size () - 1 - i)];
      }

      /// Random access.
      ///
      /// Index 0 returns the topmost element.
      T&
      operator[] (index_type i)
      {
        return seq_[size_type (size () - 1 - i)];
      }

      /// Steal the contents of \a t.
      ///
      /// Close to move-semantics.
      void
      push (YY_MOVE_REF (T) t)
      {
        seq_.push_back (T ());
        operator[] (0).move (t);
      }

      /// Pop elements from the stack.
      void
      pop (std::ptrdiff_t n = 1) YY_NOEXCEPT
      {
        for (; 0 < n; --n)
          seq_.pop_back ();
      }

      /// Pop all elements from the stack.
      void
      clear () YY_NOEXCEPT
      {
        seq_.clear ();
      }

      /// Number of elements on the stack.
      index_type
      size () const YY_NOEXCEPT
      {
        return index_type (seq_.size ());
      }

      /// Iterator on top of the stack (going downwards).
      const_iterator
      begin () const YY_NOEXCEPT
      {
        return seq_.begin ();
      }

      /// Bottom of the stack.
      const_iterator
      end () const YY_NOEXCEPT
      {
        return seq_.end ();
      }

      /// Present a slice of the top of a stack.
      class slice
      {
      public:
        slice (const stack& stack, index_type range)
          : stack_ (stack)
          , range_ (range)
        {}

        const T&
        operator[] (index_type i) const
        {
          return stack_[range_ - i];
        }

      private:
        const stack& stack_;
        index_type range_;
      };

    private:
#if YY_CPLUSPLUS < 201103L
      /// Non copyable.
      stack (const stack&);
      /// Non copyable.
      stack& operator= (const stack&);
#endif
      /// The wrapped container.
      S seq_;
    };


    /// Stack type.
    typedef stack<stack_symbol_type> stack_type;

    /// The stack.
    stack_type yystack_;

    /// Push a new state on the stack.
    /// \param m    a debug message to display
    ///             if null, no trace is output.
    /// \param sym  the symbol
    /// \warning the contents of \a s.value is stolen.
    void yypush_ (const char* m, YY_MOVE_REF (stack_symbol_type) sym);

    /// Push a new look ahead token on the state on the stack.
    /// \param m    a debug message to display
    ///             if null, no trace is output.
    /// \param s    the state
    /// \param sym  the symbol (for its value and location).
    /// \warning the contents of \a sym.value is stolen.
    void yypush_ (const char* m, state_type s, YY_MOVE_REF (symbol_type) sym);

    /// Pop \a n symbols from the stack.
    void yypop_ (int n = 1);

    /// Constants.
    enum
    {
      yylast_ = 225,     ///< Last index in yytable_.
      yynnts_ = 50,  ///< Number of nonterminal symbols.
      yyfinal_ = 13 ///< Termination state number.
    };


    // User arguments.
    tcc_sy_driver & driver;

  };

  inline
  tcc_sy_parser::symbol_kind_type
  tcc_sy_parser::yytranslate_ (int t)
  {
    // YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to
    // TOKEN-NUM as returned by yylex.
    static
    const signed char
    translate_table[] =
    {
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38
    };
    // Last valid token kind.
    const int code_max = 293;

    if (t <= 0)
      return symbol_kind::S_YYEOF;
    else if (t <= code_max)
      return YY_CAST (symbol_kind_type, translate_table[t]);
    else
      return symbol_kind::S_YYUNDEF;
  }

  // basic_symbol.
  template <typename Base>
  tcc_sy_parser::basic_symbol<Base>::basic_symbol (const basic_symbol& that)
    : Base (that)
    , value ()
    , location (that.location)
  {
    switch (this->kind ())
    {
      case symbol_kind::S_Block: // Block
      case symbol_kind::S_BlockItems: // BlockItems
        value.copy< ast::Block * > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_BreakStmt: // BreakStmt
        value.copy< ast::BreakStmt * > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_Cond: // Cond
        value.copy< ast::Cond * > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_ContinueStmt: // ContinueStmt
        value.copy< ast::ContinueStmt * > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_VarDef: // VarDef
      case symbol_kind::S_DefSingleElem: // DefSingleElem
      case symbol_kind::S_DefArray: // DefArray
      case symbol_kind::S_ArrayBody: // ArrayBody
      case symbol_kind::S_ConstDef: // ConstDef
      case symbol_kind::S_ConstDefSingleElem: // ConstDefSingleElem
      case symbol_kind::S_ConstDefArray: // ConstDefArray
        value.copy< ast::Decl * > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_BType: // BType
        value.copy< ast::Decl::VarType > (YY_MOVE (that.value));
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
        value.copy< ast::Exp * > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_AddOp: // AddOp
      case symbol_kind::S_MulOp: // MulOp
      case symbol_kind::S_UnaryOp: // UnaryOp
      case symbol_kind::S_RelOp: // RelOp
        value.copy< ast::Exp::Op > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_FuncCall: // FuncCall
        value.copy< ast::FuncCall * > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_FuncFParam: // FuncFParam
      case symbol_kind::S_FuncFSingleParam: // FuncFSingleParam
      case symbol_kind::S_FuncFParamArray: // FuncFParamArray
        value.copy< ast::FuncFParam * > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_FuncDef: // FuncDef
        value.copy< ast::Function * > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_IfStmt: // IfStmt
        value.copy< ast::IfStmt * > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_InitVal: // InitVal
      case symbol_kind::S_InitValArray: // InitValArray
      case symbol_kind::S_InitValArrayInner: // InitValArrayInner
        value.copy< ast::InitVal * > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_LVal: // LVal
        value.copy< ast::LVal * > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_ReturnStmt: // ReturnStmt
        value.copy< ast::ReturnStmt * > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_Stmt: // Stmt
        value.copy< ast::Stmt * > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_WhileStmt: // WhileStmt
        value.copy< ast::WhileStmt * > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_INTCONST: // "intconst"
        value.copy< int > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_IDENT: // IDENT
        value.copy< string > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_Decl: // Decl
      case symbol_kind::S_ConstDecl: // ConstDecl
      case symbol_kind::S_VarDecl: // VarDecl
        value.copy< vector<ast::Decl *> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_FuncRParams: // FuncRParams
        value.copy< vector<ast::Exp *> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_FuncFParams: // FuncFParams
        value.copy< vector<ast::FuncFParam *> > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_BlockItem: // BlockItem
        value.copy< vector<ast::Node *> > (YY_MOVE (that.value));
        break;

      default:
        break;
    }

  }



  template <typename Base>
  tcc_sy_parser::symbol_kind_type
  tcc_sy_parser::basic_symbol<Base>::type_get () const YY_NOEXCEPT
  {
    return this->kind ();
  }

  template <typename Base>
  bool
  tcc_sy_parser::basic_symbol<Base>::empty () const YY_NOEXCEPT
  {
    return this->kind () == symbol_kind::S_YYEMPTY;
  }

  template <typename Base>
  void
  tcc_sy_parser::basic_symbol<Base>::move (basic_symbol& s)
  {
    super_type::move (s);
    switch (this->kind ())
    {
      case symbol_kind::S_Block: // Block
      case symbol_kind::S_BlockItems: // BlockItems
        value.move< ast::Block * > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_BreakStmt: // BreakStmt
        value.move< ast::BreakStmt * > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_Cond: // Cond
        value.move< ast::Cond * > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_ContinueStmt: // ContinueStmt
        value.move< ast::ContinueStmt * > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_VarDef: // VarDef
      case symbol_kind::S_DefSingleElem: // DefSingleElem
      case symbol_kind::S_DefArray: // DefArray
      case symbol_kind::S_ArrayBody: // ArrayBody
      case symbol_kind::S_ConstDef: // ConstDef
      case symbol_kind::S_ConstDefSingleElem: // ConstDefSingleElem
      case symbol_kind::S_ConstDefArray: // ConstDefArray
        value.move< ast::Decl * > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_BType: // BType
        value.move< ast::Decl::VarType > (YY_MOVE (s.value));
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
        value.move< ast::Exp * > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_AddOp: // AddOp
      case symbol_kind::S_MulOp: // MulOp
      case symbol_kind::S_UnaryOp: // UnaryOp
      case symbol_kind::S_RelOp: // RelOp
        value.move< ast::Exp::Op > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_FuncCall: // FuncCall
        value.move< ast::FuncCall * > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_FuncFParam: // FuncFParam
      case symbol_kind::S_FuncFSingleParam: // FuncFSingleParam
      case symbol_kind::S_FuncFParamArray: // FuncFParamArray
        value.move< ast::FuncFParam * > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_FuncDef: // FuncDef
        value.move< ast::Function * > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_IfStmt: // IfStmt
        value.move< ast::IfStmt * > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_InitVal: // InitVal
      case symbol_kind::S_InitValArray: // InitValArray
      case symbol_kind::S_InitValArrayInner: // InitValArrayInner
        value.move< ast::InitVal * > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_LVal: // LVal
        value.move< ast::LVal * > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_ReturnStmt: // ReturnStmt
        value.move< ast::ReturnStmt * > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_Stmt: // Stmt
        value.move< ast::Stmt * > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_WhileStmt: // WhileStmt
        value.move< ast::WhileStmt * > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_INTCONST: // "intconst"
        value.move< int > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_IDENT: // IDENT
        value.move< string > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_Decl: // Decl
      case symbol_kind::S_ConstDecl: // ConstDecl
      case symbol_kind::S_VarDecl: // VarDecl
        value.move< vector<ast::Decl *> > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_FuncRParams: // FuncRParams
        value.move< vector<ast::Exp *> > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_FuncFParams: // FuncFParams
        value.move< vector<ast::FuncFParam *> > (YY_MOVE (s.value));
        break;

      case symbol_kind::S_BlockItem: // BlockItem
        value.move< vector<ast::Node *> > (YY_MOVE (s.value));
        break;

      default:
        break;
    }

    location = YY_MOVE (s.location);
  }

  // by_kind.
  inline
  tcc_sy_parser::by_kind::by_kind ()
    : kind_ (symbol_kind::S_YYEMPTY)
  {}

#if 201103L <= YY_CPLUSPLUS
  inline
  tcc_sy_parser::by_kind::by_kind (by_kind&& that)
    : kind_ (that.kind_)
  {
    that.clear ();
  }
#endif

  inline
  tcc_sy_parser::by_kind::by_kind (const by_kind& that)
    : kind_ (that.kind_)
  {}

  inline
  tcc_sy_parser::by_kind::by_kind (token_kind_type t)
    : kind_ (yytranslate_ (t))
  {}

  inline
  void
  tcc_sy_parser::by_kind::clear ()
  {
    kind_ = symbol_kind::S_YYEMPTY;
  }

  inline
  void
  tcc_sy_parser::by_kind::move (by_kind& that)
  {
    kind_ = that.kind_;
    that.clear ();
  }

  inline
  tcc_sy_parser::symbol_kind_type
  tcc_sy_parser::by_kind::kind () const YY_NOEXCEPT
  {
    return kind_;
  }

  inline
  tcc_sy_parser::symbol_kind_type
  tcc_sy_parser::by_kind::type_get () const YY_NOEXCEPT
  {
    return this->kind ();
  }

} // yy
#line 2803 "/home/gch981213/src/csccc/tinbaccc/cmake-build-debug/tcc-sy-parser.hh"




#endif // !YY_YY_HOME_GCH981213_SRC_CSCCC_TINBACCC_CMAKE_BUILD_DEBUG_TCC_SY_PARSER_HH_INCLUDED
