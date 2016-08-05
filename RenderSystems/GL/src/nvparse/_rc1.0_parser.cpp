/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.0.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1


/* Substitute the variable and function names.  */
#define yyparse         rc10_parse
#define yylex           rc10_lex
#define yyerror         rc10_error
#define yydebug         rc10_debug
#define yynerrs         rc10_nerrs

#define yylval          rc10_lval
#define yychar          rc10_char

/* Copy the first part of user declarations.  */
#line 3 "rc1.0_grammar.y" /* yacc.c:339  */

void yyerror(const char* s);
int yylex ( void );

#ifdef _WIN32
# include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#include "rc1.0_combiners.h"
#include "nvparse_errors.h"
#include "nvparse_externs.h"



#line 92 "_rc1.0_parser.c" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "_rc1.0_parser.h".  */
#ifndef YY_RC10_RC1_0_PARSER_H_INCLUDED
# define YY_RC10_RC1_0_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int rc10_debug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    regVariable = 258,
    constVariable = 259,
    color_sum = 260,
    final_product = 261,
    expandString = 262,
    halfBiasString = 263,
    unsignedString = 264,
    unsignedInvertString = 265,
    muxString = 266,
    sumString = 267,
    rgb_portion = 268,
    alpha_portion = 269,
    openParen = 270,
    closeParen = 271,
    openBracket = 272,
    closeBracket = 273,
    semicolon = 274,
    comma = 275,
    dot = 276,
    times = 277,
    minus = 278,
    equals = 279,
    plus = 280,
    bias_by_negative_one_half_scale_by_two = 281,
    bias_by_negative_one_half = 282,
    scale_by_one_half = 283,
    scale_by_two = 284,
    scale_by_four = 285,
    clamp_color_sum = 286,
    lerp = 287,
    fragment_rgb = 288,
    fragment_alpha = 289,
    floatValue = 290
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE YYSTYPE;
union YYSTYPE
{
#line 20 "rc1.0_grammar.y" /* yacc.c:355  */

  int ival;
  float fval;
  RegisterEnum registerEnum;
  BiasScaleEnum biasScaleEnum;
  MappedRegisterStruct mappedRegisterStruct;
  ConstColorStruct constColorStruct;
  GeneralPortionStruct generalPortionStruct;
  GeneralFunctionStruct generalFunctionStruct;
  OpStruct opStruct;
  GeneralCombinerStruct generalCombinerStruct;
  GeneralCombinersStruct generalCombinersStruct;
  FinalProductStruct finalProductStruct;
  FinalRgbFunctionStruct finalRgbFunctionStruct;
  FinalAlphaFunctionStruct finalAlphaFunctionStruct;
  FinalCombinerStruct finalCombinerStruct;
  CombinersStruct combinersStruct;

#line 187 "_rc1.0_parser.c" /* yacc.c:355  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE rc10_lval;

int rc10_parse (void);

#endif /* !YY_RC10_RC1_0_PARSER_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 202 "_rc1.0_parser.c" /* yacc.c:358  */

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
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


#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  27
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   258

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  36
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  22
/* YYNRULES -- Number of rules.  */
#define YYNRULES  88
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  220

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   290

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
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
      35
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,    73,    73,    80,    86,    92,    98,   106,   114,   124,
     133,   138,   146,   152,   158,   164,   170,   176,   184,   190,
     200,   204,   210,   216,   222,   228,   234,   240,   246,   252,
     260,   266,   272,   278,   284,   290,   296,   302,   310,   318,
     324,   336,   344,   352,   356,   360,   364,   368,   374,   380,
     386,   392,   398,   404,   410,   416,   422,   430,   436,   442,
     448,   454,   461,   467,   473,   479,   485,   492,   500,   508,
     516,   524,   533,   541,   549,   557,   565,   575,   581,   589,
     595,   601,   611,   623,   633,   647,   661,   669,   673
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "regVariable", "constVariable",
  "color_sum", "final_product", "expandString", "halfBiasString",
  "unsignedString", "unsignedInvertString", "muxString", "sumString",
  "rgb_portion", "alpha_portion", "openParen", "closeParen", "openBracket",
  "closeBracket", "semicolon", "comma", "dot", "times", "minus", "equals",
  "plus", "bias_by_negative_one_half_scale_by_two",
  "bias_by_negative_one_half", "scale_by_one_half", "scale_by_two",
  "scale_by_four", "clamp_color_sum", "lerp", "fragment_rgb",
  "fragment_alpha", "floatValue", "$accept", "WholeEnchilada", "Combiners",
  "ConstColor", "GeneralCombiners", "GeneralCombiner", "GeneralPortion",
  "PortionDesignator", "GeneralMappedRegister", "GeneralFunction", "Dot",
  "Mul", "Mux", "Sum", "BiasScale", "FinalMappedRegister", "FinalCombiner",
  "ClampColorSum", "FinalProduct", "FinalRgbFunction",
  "FinalAlphaFunction", "Register", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290
};
# endif

#define YYPACT_NINF -63

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-63)))

#define YYTABLE_NINF -1

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      48,   -13,    -8,    53,    16,    11,    14,    73,   -63,    48,
      94,   -63,   -63,    43,   147,    44,    47,    70,   144,   -63,
     -63,    53,    -5,    74,    78,    83,   144,   -63,    94,    94,
     -63,   -63,   -63,    17,    44,    47,    17,    44,    47,   -63,
     -63,    64,   -63,   -63,   -63,   -63,    97,   111,   107,   -63,
      58,   103,   -63,   117,    80,   132,   142,    21,   139,    94,
     -63,   -63,    44,    47,   -63,   -63,    44,    47,   -63,   -63,
     146,   167,   171,   144,   151,   -63,   149,   -63,   116,    80,
      80,   138,   -63,   144,   -63,   144,    92,   -63,   -63,   -63,
     -63,   -63,   -63,   133,   170,   173,   174,   175,   176,   177,
     160,   -63,   169,   -63,   -63,   179,   180,   181,   182,   183,
     184,   -63,   -63,   -63,    80,   129,   185,    28,   186,   187,
     188,   -63,   -63,   -63,   -63,   -63,   -63,   -63,   -63,   191,
     193,   194,   195,   196,   -63,   -63,   -63,   189,   199,   200,
     201,   202,   152,   163,   -63,   144,   -63,   144,   144,   -63,
     164,   203,   204,   205,   206,   207,    98,    80,    80,    80,
      80,   212,   213,   -63,   -63,   129,   129,   198,   210,   211,
     214,   -63,   -63,   -63,   -63,   -63,   215,   217,   219,   220,
     221,   222,    80,    80,   223,   224,   144,   -63,   144,   165,
     225,   228,   -63,   -63,   -63,   -63,   229,   230,   -63,   -63,
     231,   232,   233,   234,   235,   -63,   -63,    50,   144,   236,
     -63,   -63,   -63,   144,   240,   -63,   238,   239,   -63,   -63
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     0,     0,     0,     0,     0,     0,     2,     0,
       0,    11,     8,     0,     0,    67,    72,     0,     0,    20,
      21,     0,     0,     0,     0,     0,     0,     1,     0,     0,
       6,    10,     5,     0,    68,    73,     0,    69,    74,    62,
      57,     0,    88,    87,    51,    54,     0,     0,     0,    48,
       0,     0,    15,     0,     0,     0,     0,     0,     0,     0,
       7,     3,    70,    75,    63,    58,    71,    76,    64,    59,
       0,     0,     0,     0,     0,    16,     0,    12,     0,    33,
      37,     0,    77,     0,    84,     0,     0,    86,     4,    65,
      60,    66,    61,     0,     0,     0,     0,     0,     0,     0,
       0,    17,     0,    13,    19,     0,     0,     0,     0,     0,
       0,    30,    31,    32,    34,     0,     0,     0,     0,     0,
       0,    52,    55,    49,    53,    56,    50,    78,    14,     0,
       0,     0,     0,     0,    18,    35,    36,     0,     0,     0,
       0,     0,     0,     0,    22,     0,    82,     0,     0,    85,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    23,    40,     0,     0,     0,     0,     0,
       0,    43,    44,    45,    46,    47,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    83,     0,     0,
       0,     0,    24,    26,    28,    29,     0,     0,    38,    39,
       0,     0,     0,     0,     0,    25,    27,     0,     0,     0,
      41,    42,    81,     0,     0,     9,     0,     0,    79,    80
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
     -63,   -63,   -63,     9,    -2,     0,   -18,   -63,   -62,   -63,
      39,    61,   -63,   -63,   -63,   -25,    -4,   190,   208,     1,
       8,   -52
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     7,     8,     9,    10,    11,    22,    23,   143,    78,
      79,    80,   135,   136,   110,    48,    12,    13,    14,    15,
      16,    49
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
      57,    58,    81,    51,    53,    30,    32,    29,    19,    20,
      31,    17,    21,    52,    34,    37,    18,    40,    28,    96,
      99,    35,    38,    39,    60,    61,    59,    81,    81,    31,
      50,    24,    74,    76,    62,    25,    65,    66,    26,    69,
      84,    63,    64,    85,    67,    68,    86,   146,   100,     2,
       5,     6,     1,   147,     2,    88,   102,     1,   116,    31,
     117,   119,   137,   144,    90,     3,    19,    20,    92,   212,
      89,    19,    20,    27,    91,   213,     5,     6,     6,     4,
       5,     5,     6,    42,    43,    41,    42,    43,    44,    45,
     163,    54,    46,    47,    55,    42,    43,    44,    45,    70,
       2,    46,    47,   184,   185,   178,   179,   180,   181,   176,
     177,     3,    71,   144,   144,    56,    19,    20,   111,   113,
     167,    75,   168,   169,   118,     4,    72,     5,     6,    73,
     196,   197,    42,    43,   104,    77,   138,   139,   140,   141,
     112,   114,   105,   106,   107,   108,   109,    42,    43,    44,
      45,    82,   142,    46,    47,    42,    43,    83,    87,   161,
     162,   200,   115,   201,    19,    20,    93,   103,   120,   101,
      42,    43,    94,    95,    42,    43,    97,    98,     4,   127,
       5,     6,   164,   214,   165,   166,   121,   128,   216,   122,
     123,   124,   125,   126,   129,   130,   131,   132,   133,   170,
     202,   148,   134,     0,    36,   145,   149,   151,   150,   152,
     153,   154,   155,   156,   157,   158,   159,   160,   186,     0,
       0,    33,   171,   172,   173,   174,   175,   182,   183,   187,
     190,   188,   191,     0,   189,   192,   193,   194,   195,     0,
       0,   203,   198,   199,   204,   205,   206,   207,     0,   209,
       0,     0,   208,   210,   211,   215,   217,   218,   219
};

static const yytype_int16 yycheck[] =
{
      25,    26,    54,    21,    22,     9,    10,     9,    13,    14,
      10,    24,     3,    18,    13,    14,    24,    16,     9,    71,
      72,    13,    14,    15,    28,    29,    28,    79,    80,    29,
      21,    15,    50,    51,    33,    24,    35,    36,    24,    38,
      19,    33,    34,    22,    36,    37,    25,    19,    73,     6,
      33,    34,     4,    25,     6,    59,    74,     4,    83,    59,
      85,    86,   114,   115,    63,    17,    13,    14,    67,    19,
      62,    13,    14,     0,    66,    25,    33,    34,    34,    31,
      33,    33,    34,     3,     4,    15,     3,     4,     5,     6,
     142,    17,     9,    10,    16,     3,     4,     5,     6,    35,
       6,     9,    10,   165,   166,   157,   158,   159,   160,    11,
      12,    17,    15,   165,   166,    32,    13,    14,    79,    80,
     145,    18,   147,   148,    32,    31,    15,    33,    34,    22,
     182,   183,     3,     4,    18,    18,     7,     8,     9,    10,
      79,    80,    26,    27,    28,    29,    30,     3,     4,     5,
       6,    19,    23,     9,    10,     3,     4,    15,    19,     7,
       8,   186,    24,   188,    13,    14,    20,    18,    35,    18,
       3,     4,     5,     6,     3,     4,     5,     6,    31,    19,
      33,    34,    19,   208,    21,    22,    16,    18,   213,    16,
      16,    16,    16,    16,    15,    15,    15,    15,    15,    35,
      35,    15,    18,    -1,    14,    20,    19,    16,    20,    16,
      16,    16,    16,    24,    15,    15,    15,    15,    20,    -1,
      -1,    13,    19,    19,    19,    19,    19,    15,    15,    19,
      15,    20,    15,    -1,    20,    16,    16,    16,    16,    -1,
      -1,    16,    19,    19,    16,    16,    16,    16,    -1,    16,
      -1,    -1,    20,    19,    19,    19,    16,    19,    19
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     4,     6,    17,    31,    33,    34,    37,    38,    39,
      40,    41,    52,    53,    54,    55,    56,    24,    24,    13,
      14,    39,    42,    43,    15,    24,    24,     0,    39,    40,
      52,    41,    52,    54,    55,    56,    53,    55,    56,    56,
      55,    15,     3,     4,     5,     6,     9,    10,    51,    57,
      39,    42,    18,    42,    17,    16,    32,    51,    51,    40,
      52,    52,    55,    56,    56,    55,    55,    56,    56,    55,
      35,    15,    15,    22,    42,    18,    42,    18,    45,    46,
      47,    57,    19,    15,    19,    22,    25,    19,    52,    56,
      55,    56,    55,    20,     5,     6,    57,     5,     6,    57,
      51,    18,    42,    18,    18,    26,    27,    28,    29,    30,
      50,    46,    47,    46,    47,    24,    51,    51,    32,    51,
      35,    16,    16,    16,    16,    16,    16,    19,    18,    15,
      15,    15,    15,    15,    18,    48,    49,    57,     7,     8,
       9,    10,    23,    44,    57,    20,    19,    25,    15,    19,
      20,    16,    16,    16,    16,    16,    24,    15,    15,    15,
      15,     7,     8,    57,    19,    21,    22,    51,    51,    51,
      35,    19,    19,    19,    19,    19,    11,    12,    57,    57,
      57,    57,    15,    15,    44,    44,    20,    19,    20,    20,
      15,    15,    16,    16,    16,    16,    57,    57,    19,    19,
      51,    51,    35,    16,    16,    16,    16,    16,    20,    16,
      19,    19,    19,    25,    51,    19,    51,    16,    19,    19
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    36,    37,    38,    38,    38,    38,    38,    38,    39,
      40,    40,    41,    41,    41,    41,    41,    41,    42,    42,
      43,    43,    44,    44,    44,    44,    44,    44,    44,    44,
      45,    45,    45,    45,    45,    45,    45,    45,    46,    47,
      47,    48,    49,    50,    50,    50,    50,    50,    51,    51,
      51,    51,    51,    51,    51,    51,    51,    52,    52,    52,
      52,    52,    52,    52,    52,    52,    52,    52,    52,    52,
      52,    52,    52,    52,    52,    52,    52,    53,    54,    55,
      55,    55,    55,    55,    55,    55,    56,    57,    57
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     3,     4,     2,     2,     3,     1,    12,
       2,     1,     4,     5,     6,     3,     4,     5,     5,     4,
       1,     1,     1,     2,     4,     5,     4,     5,     4,     4,
       2,     2,     2,     1,     2,     3,     3,     1,     6,     6,
       4,     6,     6,     4,     4,     4,     4,     4,     1,     4,
       4,     1,     4,     4,     1,     4,     4,     2,     3,     3,
       4,     4,     2,     3,     3,     4,     4,     1,     2,     2,
       3,     3,     1,     2,     2,     3,     3,     4,     6,    13,
      13,    11,     6,     8,     4,     6,     4,     1,     1
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                                              );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
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
            /* Fall through.  */
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yystacksize);

        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 74 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			(yyvsp[0].combinersStruct).Validate();
			(yyvsp[0].combinersStruct).Invoke();
		}
#line 1430 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 3:
#line 81 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			CombinersStruct combinersStruct;
			combinersStruct.Init((yyvsp[-1].generalCombinersStruct), (yyvsp[0].finalCombinerStruct), (yyvsp[-2].constColorStruct));
			(yyval.combinersStruct) = combinersStruct;
		}
#line 1440 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 4:
#line 87 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			CombinersStruct combinersStruct;
			combinersStruct.Init((yyvsp[-1].generalCombinersStruct), (yyvsp[0].finalCombinerStruct), (yyvsp[-3].constColorStruct), (yyvsp[-2].constColorStruct));
			(yyval.combinersStruct) = combinersStruct;
		}
#line 1450 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 5:
#line 93 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			CombinersStruct combinersStruct;
			combinersStruct.Init((yyvsp[-1].generalCombinersStruct), (yyvsp[0].finalCombinerStruct));
			(yyval.combinersStruct) = combinersStruct;
		}
#line 1460 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 6:
#line 99 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			GeneralCombinersStruct generalCombinersStruct;
			generalCombinersStruct.Init();
			CombinersStruct combinersStruct;
			combinersStruct.Init(generalCombinersStruct, (yyvsp[0].finalCombinerStruct), (yyvsp[-1].constColorStruct));
			(yyval.combinersStruct) = combinersStruct;
		}
#line 1472 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 7:
#line 107 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			GeneralCombinersStruct generalCombinersStruct;
			generalCombinersStruct.Init();
			CombinersStruct combinersStruct;
			combinersStruct.Init(generalCombinersStruct, (yyvsp[0].finalCombinerStruct), (yyvsp[-2].constColorStruct), (yyvsp[-1].constColorStruct));
			(yyval.combinersStruct) = combinersStruct;
		}
#line 1484 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 8:
#line 115 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			GeneralCombinersStruct generalCombinersStruct;
			generalCombinersStruct.Init();
			CombinersStruct combinersStruct;
			combinersStruct.Init(generalCombinersStruct, (yyvsp[0].finalCombinerStruct));
			(yyval.combinersStruct) = combinersStruct;
		}
#line 1496 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 9:
#line 125 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			ConstColorStruct constColorStruct;
			constColorStruct.Init((yyvsp[-11].registerEnum), (yyvsp[-8].fval), (yyvsp[-6].fval), (yyvsp[-4].fval), (yyvsp[-2].fval));
			(yyval.constColorStruct) = constColorStruct;
		}
#line 1506 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 10:
#line 134 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			(yyvsp[-1].generalCombinersStruct) += (yyvsp[0].generalCombinerStruct);
			(yyval.generalCombinersStruct) = (yyvsp[-1].generalCombinersStruct);
		}
#line 1515 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 11:
#line 139 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			GeneralCombinersStruct generalCombinersStruct;
			generalCombinersStruct.Init((yyvsp[0].generalCombinerStruct));
			(yyval.generalCombinersStruct) = generalCombinersStruct;
		}
#line 1525 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 12:
#line 147 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			GeneralCombinerStruct generalCombinerStruct;
			generalCombinerStruct.Init((yyvsp[-2].generalPortionStruct), (yyvsp[-1].generalPortionStruct));
			(yyval.generalCombinerStruct) = generalCombinerStruct;
		}
#line 1535 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 13:
#line 153 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			GeneralCombinerStruct generalCombinerStruct;
			generalCombinerStruct.Init((yyvsp[-2].generalPortionStruct), (yyvsp[-1].generalPortionStruct), (yyvsp[-3].constColorStruct));
			(yyval.generalCombinerStruct) = generalCombinerStruct;
		}
#line 1545 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 14:
#line 159 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			GeneralCombinerStruct generalCombinerStruct;
			generalCombinerStruct.Init((yyvsp[-2].generalPortionStruct), (yyvsp[-1].generalPortionStruct), (yyvsp[-4].constColorStruct), (yyvsp[-3].constColorStruct));
			(yyval.generalCombinerStruct) = generalCombinerStruct;
		}
#line 1555 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 15:
#line 165 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			GeneralCombinerStruct generalCombinerStruct;
			generalCombinerStruct.Init((yyvsp[-1].generalPortionStruct));
			(yyval.generalCombinerStruct) = generalCombinerStruct;
		}
#line 1565 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 16:
#line 171 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			GeneralCombinerStruct generalCombinerStruct;
			generalCombinerStruct.Init((yyvsp[-1].generalPortionStruct), (yyvsp[-2].constColorStruct));
			(yyval.generalCombinerStruct) = generalCombinerStruct;
		}
#line 1575 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 17:
#line 177 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			GeneralCombinerStruct generalCombinerStruct;
			generalCombinerStruct.Init((yyvsp[-1].generalPortionStruct), (yyvsp[-3].constColorStruct), (yyvsp[-2].constColorStruct));
			(yyval.generalCombinerStruct) = generalCombinerStruct;
		}
#line 1585 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 18:
#line 185 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			GeneralPortionStruct generalPortionStruct;
			generalPortionStruct.Init((yyvsp[-4].ival), (yyvsp[-2].generalFunctionStruct), (yyvsp[-1].biasScaleEnum));
			(yyval.generalPortionStruct) = generalPortionStruct;
		}
#line 1595 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 19:
#line 191 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			BiasScaleEnum noScale;
			noScale.word = RCP_SCALE_BY_ONE;
			GeneralPortionStruct generalPortionStruct;
			generalPortionStruct.Init((yyvsp[-3].ival), (yyvsp[-1].generalFunctionStruct), noScale);
			(yyval.generalPortionStruct) = generalPortionStruct;
		}
#line 1607 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 20:
#line 201 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			(yyval.ival) = (yyvsp[0].ival);
		}
#line 1615 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 21:
#line 205 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			(yyval.ival) = (yyvsp[0].ival);
		}
#line 1623 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 22:
#line 211 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			MappedRegisterStruct reg;
			reg.Init((yyvsp[0].registerEnum), GL_SIGNED_IDENTITY_NV);
			(yyval.mappedRegisterStruct) = reg;
		}
#line 1633 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 23:
#line 217 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			MappedRegisterStruct reg;
			reg.Init((yyvsp[0].registerEnum), GL_SIGNED_NEGATE_NV);
			(yyval.mappedRegisterStruct) = reg;
		}
#line 1643 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 24:
#line 223 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			MappedRegisterStruct reg;
			reg.Init((yyvsp[-1].registerEnum), GL_EXPAND_NORMAL_NV);
			(yyval.mappedRegisterStruct) = reg;
		}
#line 1653 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 25:
#line 229 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			MappedRegisterStruct reg;
			reg.Init((yyvsp[-1].registerEnum), GL_EXPAND_NEGATE_NV);
			(yyval.mappedRegisterStruct) = reg;
		}
#line 1663 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 26:
#line 235 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			MappedRegisterStruct reg;
			reg.Init((yyvsp[-1].registerEnum), GL_HALF_BIAS_NORMAL_NV);
			(yyval.mappedRegisterStruct) = reg;
		}
#line 1673 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 27:
#line 241 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			MappedRegisterStruct reg;
			reg.Init((yyvsp[-1].registerEnum), GL_HALF_BIAS_NEGATE_NV);
			(yyval.mappedRegisterStruct) = reg;
		}
#line 1683 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 28:
#line 247 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			MappedRegisterStruct reg;
			reg.Init((yyvsp[-1].registerEnum), GL_UNSIGNED_IDENTITY_NV);
			(yyval.mappedRegisterStruct) = reg;
		}
#line 1693 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 29:
#line 253 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			MappedRegisterStruct reg;
			reg.Init((yyvsp[-1].registerEnum), GL_UNSIGNED_INVERT_NV);
			(yyval.mappedRegisterStruct) = reg;
		}
#line 1703 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 30:
#line 261 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			GeneralFunctionStruct generalFunction;
			generalFunction.Init((yyvsp[-1].opStruct), (yyvsp[0].opStruct));
			(yyval.generalFunctionStruct) = generalFunction;
		}
#line 1713 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 31:
#line 267 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			GeneralFunctionStruct generalFunction;
			generalFunction.Init((yyvsp[-1].opStruct), (yyvsp[0].opStruct));
			(yyval.generalFunctionStruct) = generalFunction;
		}
#line 1723 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 32:
#line 273 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			GeneralFunctionStruct generalFunction;
			generalFunction.Init((yyvsp[-1].opStruct), (yyvsp[0].opStruct));
			(yyval.generalFunctionStruct) = generalFunction;
		}
#line 1733 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 33:
#line 279 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			GeneralFunctionStruct generalFunction;
			generalFunction.Init((yyvsp[0].opStruct));
			(yyval.generalFunctionStruct) = generalFunction;
		}
#line 1743 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 34:
#line 285 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			GeneralFunctionStruct generalFunction;
			generalFunction.Init((yyvsp[-1].opStruct), (yyvsp[0].opStruct));
			(yyval.generalFunctionStruct) = generalFunction;
		}
#line 1753 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 35:
#line 291 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			GeneralFunctionStruct generalFunction;
			generalFunction.Init((yyvsp[-2].opStruct), (yyvsp[-1].opStruct), (yyvsp[0].opStruct));
			(yyval.generalFunctionStruct) = generalFunction;
		}
#line 1763 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 36:
#line 297 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			GeneralFunctionStruct generalFunction;
			generalFunction.Init((yyvsp[-2].opStruct), (yyvsp[-1].opStruct), (yyvsp[0].opStruct));
			(yyval.generalFunctionStruct) = generalFunction;
		}
#line 1773 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 37:
#line 303 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			GeneralFunctionStruct generalFunction;
			generalFunction.Init((yyvsp[0].opStruct));
			(yyval.generalFunctionStruct) = generalFunction;
		}
#line 1783 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 38:
#line 311 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			OpStruct dotFunction;
			dotFunction.Init(RCP_DOT, (yyvsp[-5].registerEnum), (yyvsp[-3].mappedRegisterStruct), (yyvsp[-1].mappedRegisterStruct));
			(yyval.opStruct) = dotFunction;
		}
#line 1793 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 39:
#line 319 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			OpStruct mulFunction;
			mulFunction.Init(RCP_MUL, (yyvsp[-5].registerEnum), (yyvsp[-3].mappedRegisterStruct), (yyvsp[-1].mappedRegisterStruct));
			(yyval.opStruct) = mulFunction;
		}
#line 1803 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 40:
#line 325 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			RegisterEnum zero;
			zero.word = RCP_ZERO;
			MappedRegisterStruct one;
			one.Init(zero, GL_UNSIGNED_INVERT_NV);
			OpStruct mulFunction;
			mulFunction.Init(RCP_MUL, (yyvsp[-3].registerEnum), (yyvsp[-1].mappedRegisterStruct), one);
			(yyval.opStruct) = mulFunction;
		}
#line 1817 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 41:
#line 337 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			OpStruct muxFunction;
			muxFunction.Init(RCP_MUX, (yyvsp[-5].registerEnum));
			(yyval.opStruct) = muxFunction;
		}
#line 1827 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 42:
#line 345 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			OpStruct sumFunction;
			sumFunction.Init(RCP_SUM, (yyvsp[-5].registerEnum));
			(yyval.opStruct) = sumFunction;
		}
#line 1837 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 43:
#line 353 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			(yyval.biasScaleEnum) = (yyvsp[-3].biasScaleEnum);
		}
#line 1845 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 44:
#line 357 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			(yyval.biasScaleEnum) = (yyvsp[-3].biasScaleEnum);
		}
#line 1853 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 45:
#line 361 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			(yyval.biasScaleEnum) = (yyvsp[-3].biasScaleEnum);
		}
#line 1861 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 46:
#line 365 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			(yyval.biasScaleEnum) = (yyvsp[-3].biasScaleEnum);
		}
#line 1869 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 47:
#line 369 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			(yyval.biasScaleEnum) = (yyvsp[-3].biasScaleEnum);
		}
#line 1877 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 48:
#line 375 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			MappedRegisterStruct reg;
			reg.Init((yyvsp[0].registerEnum), GL_UNSIGNED_IDENTITY_NV);
			(yyval.mappedRegisterStruct) = reg;
		}
#line 1887 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 49:
#line 381 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			MappedRegisterStruct reg;
			reg.Init((yyvsp[-1].registerEnum), GL_UNSIGNED_IDENTITY_NV);
			(yyval.mappedRegisterStruct) = reg;
		}
#line 1897 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 50:
#line 387 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			MappedRegisterStruct reg;
			reg.Init((yyvsp[-1].registerEnum), GL_UNSIGNED_INVERT_NV);
			(yyval.mappedRegisterStruct) = reg;
		}
#line 1907 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 51:
#line 393 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			MappedRegisterStruct reg;
			reg.Init((yyvsp[0].registerEnum));
			(yyval.mappedRegisterStruct) = reg;
		}
#line 1917 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 52:
#line 399 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			MappedRegisterStruct reg;
			reg.Init((yyvsp[-1].registerEnum), GL_UNSIGNED_IDENTITY_NV);
			(yyval.mappedRegisterStruct) = reg;
		}
#line 1927 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 53:
#line 405 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			MappedRegisterStruct reg;
			reg.Init((yyvsp[-1].registerEnum), GL_UNSIGNED_INVERT_NV);
			(yyval.mappedRegisterStruct) = reg;
		}
#line 1937 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 54:
#line 411 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			MappedRegisterStruct reg;
			reg.Init((yyvsp[0].registerEnum));
			(yyval.mappedRegisterStruct) = reg;
		}
#line 1947 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 55:
#line 417 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			MappedRegisterStruct reg;
			reg.Init((yyvsp[-1].registerEnum), GL_UNSIGNED_IDENTITY_NV);
			(yyval.mappedRegisterStruct) = reg;
		}
#line 1957 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 56:
#line 423 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			MappedRegisterStruct reg;
			reg.Init((yyvsp[-1].registerEnum), GL_UNSIGNED_INVERT_NV);
			(yyval.mappedRegisterStruct) = reg;
		}
#line 1967 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 57:
#line 431 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			FinalCombinerStruct finalCombinerStruct;
			finalCombinerStruct.Init((yyvsp[0].finalRgbFunctionStruct), (yyvsp[-1].finalAlphaFunctionStruct), false);
			(yyval.finalCombinerStruct) = finalCombinerStruct;
		}
#line 1977 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 58:
#line 437 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			FinalCombinerStruct finalCombinerStruct;
			finalCombinerStruct.Init((yyvsp[0].finalRgbFunctionStruct), (yyvsp[-1].finalAlphaFunctionStruct), true);
			(yyval.finalCombinerStruct) = finalCombinerStruct;
		}
#line 1987 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 59:
#line 443 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			FinalCombinerStruct finalCombinerStruct;
			finalCombinerStruct.Init((yyvsp[0].finalRgbFunctionStruct), (yyvsp[-1].finalAlphaFunctionStruct), false, (yyvsp[-2].finalProductStruct));
			(yyval.finalCombinerStruct) = finalCombinerStruct;
		}
#line 1997 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 60:
#line 449 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			FinalCombinerStruct finalCombinerStruct;
			finalCombinerStruct.Init((yyvsp[0].finalRgbFunctionStruct), (yyvsp[-1].finalAlphaFunctionStruct), true, (yyvsp[-2].finalProductStruct));
			(yyval.finalCombinerStruct) = finalCombinerStruct;
		}
#line 2007 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 61:
#line 455 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			FinalCombinerStruct finalCombinerStruct;
			finalCombinerStruct.Init((yyvsp[0].finalRgbFunctionStruct), (yyvsp[-1].finalAlphaFunctionStruct), true, (yyvsp[-3].finalProductStruct));
			(yyval.finalCombinerStruct) = finalCombinerStruct;
		}
#line 2017 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 62:
#line 462 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			FinalCombinerStruct finalCombinerStruct;
			finalCombinerStruct.Init((yyvsp[-1].finalRgbFunctionStruct), (yyvsp[0].finalAlphaFunctionStruct), false);
			(yyval.finalCombinerStruct) = finalCombinerStruct;
		}
#line 2027 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 63:
#line 468 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			FinalCombinerStruct finalCombinerStruct;
			finalCombinerStruct.Init((yyvsp[-1].finalRgbFunctionStruct), (yyvsp[0].finalAlphaFunctionStruct), true);
			(yyval.finalCombinerStruct) = finalCombinerStruct;
		}
#line 2037 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 64:
#line 474 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			FinalCombinerStruct finalCombinerStruct;
			finalCombinerStruct.Init((yyvsp[-1].finalRgbFunctionStruct), (yyvsp[0].finalAlphaFunctionStruct), false, (yyvsp[-2].finalProductStruct));
			(yyval.finalCombinerStruct) = finalCombinerStruct;
		}
#line 2047 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 65:
#line 480 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			FinalCombinerStruct finalCombinerStruct;
			finalCombinerStruct.Init((yyvsp[-1].finalRgbFunctionStruct), (yyvsp[0].finalAlphaFunctionStruct), true, (yyvsp[-2].finalProductStruct));
			(yyval.finalCombinerStruct) = finalCombinerStruct;
		}
#line 2057 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 66:
#line 486 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			FinalCombinerStruct finalCombinerStruct;
			finalCombinerStruct.Init((yyvsp[-1].finalRgbFunctionStruct), (yyvsp[0].finalAlphaFunctionStruct), true, (yyvsp[-3].finalProductStruct));
			(yyval.finalCombinerStruct) = finalCombinerStruct;
		}
#line 2067 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 67:
#line 493 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			FinalCombinerStruct finalCombinerStruct;
			FinalAlphaFunctionStruct finalAlphaFunctionStruct;
			finalAlphaFunctionStruct.ZeroOut();
			finalCombinerStruct.Init((yyvsp[0].finalRgbFunctionStruct), finalAlphaFunctionStruct, false);
			(yyval.finalCombinerStruct) = finalCombinerStruct;
		}
#line 2079 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 68:
#line 501 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			FinalCombinerStruct finalCombinerStruct;
			FinalAlphaFunctionStruct finalAlphaFunctionStruct;
			finalAlphaFunctionStruct.ZeroOut();
			finalCombinerStruct.Init((yyvsp[0].finalRgbFunctionStruct), finalAlphaFunctionStruct, true);
			(yyval.finalCombinerStruct) = finalCombinerStruct;
		}
#line 2091 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 69:
#line 509 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			FinalCombinerStruct finalCombinerStruct;
			FinalAlphaFunctionStruct finalAlphaFunctionStruct;
			finalAlphaFunctionStruct.ZeroOut();
			finalCombinerStruct.Init((yyvsp[0].finalRgbFunctionStruct), finalAlphaFunctionStruct, false, (yyvsp[-1].finalProductStruct));
			(yyval.finalCombinerStruct) = finalCombinerStruct;
		}
#line 2103 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 70:
#line 517 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			FinalCombinerStruct finalCombinerStruct;
			FinalAlphaFunctionStruct finalAlphaFunctionStruct;
			finalAlphaFunctionStruct.ZeroOut();
			finalCombinerStruct.Init((yyvsp[0].finalRgbFunctionStruct), finalAlphaFunctionStruct, true, (yyvsp[-1].finalProductStruct));
			(yyval.finalCombinerStruct) = finalCombinerStruct;
		}
#line 2115 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 71:
#line 525 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			FinalCombinerStruct finalCombinerStruct;
			FinalAlphaFunctionStruct finalAlphaFunctionStruct;
			finalAlphaFunctionStruct.ZeroOut();
			finalCombinerStruct.Init((yyvsp[0].finalRgbFunctionStruct), finalAlphaFunctionStruct, true, (yyvsp[-2].finalProductStruct));
			(yyval.finalCombinerStruct) = finalCombinerStruct;
		}
#line 2127 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 72:
#line 534 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			FinalCombinerStruct finalCombinerStruct;
			FinalRgbFunctionStruct finalRgbFunctionStruct;
			finalRgbFunctionStruct.ZeroOut();
			finalCombinerStruct.Init(finalRgbFunctionStruct, (yyvsp[0].finalAlphaFunctionStruct), false);
			(yyval.finalCombinerStruct) = finalCombinerStruct;
		}
#line 2139 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 73:
#line 542 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			FinalCombinerStruct finalCombinerStruct;
			FinalRgbFunctionStruct finalRgbFunctionStruct;
			finalRgbFunctionStruct.ZeroOut();
			finalCombinerStruct.Init(finalRgbFunctionStruct, (yyvsp[0].finalAlphaFunctionStruct), true);
			(yyval.finalCombinerStruct) = finalCombinerStruct;
		}
#line 2151 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 74:
#line 550 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			FinalCombinerStruct finalCombinerStruct;
			FinalRgbFunctionStruct finalRgbFunctionStruct;
			finalRgbFunctionStruct.ZeroOut();
			finalCombinerStruct.Init(finalRgbFunctionStruct, (yyvsp[0].finalAlphaFunctionStruct), false, (yyvsp[-1].finalProductStruct));
			(yyval.finalCombinerStruct) = finalCombinerStruct;
		}
#line 2163 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 75:
#line 558 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			FinalCombinerStruct finalCombinerStruct;
			FinalRgbFunctionStruct finalRgbFunctionStruct;
			finalRgbFunctionStruct.ZeroOut();
			finalCombinerStruct.Init(finalRgbFunctionStruct, (yyvsp[0].finalAlphaFunctionStruct), true, (yyvsp[-1].finalProductStruct));
			(yyval.finalCombinerStruct) = finalCombinerStruct;
		}
#line 2175 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 76:
#line 566 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			FinalCombinerStruct finalCombinerStruct;
			FinalRgbFunctionStruct finalRgbFunctionStruct;
			finalRgbFunctionStruct.ZeroOut();
			finalCombinerStruct.Init(finalRgbFunctionStruct, (yyvsp[0].finalAlphaFunctionStruct), true, (yyvsp[-2].finalProductStruct));
			(yyval.finalCombinerStruct) = finalCombinerStruct;
		}
#line 2187 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 77:
#line 576 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			(yyval.ival) = (yyvsp[-3].ival);
		}
#line 2195 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 78:
#line 582 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			FinalProductStruct finalProductStruct;
			finalProductStruct.Init((yyvsp[-3].mappedRegisterStruct), (yyvsp[-1].mappedRegisterStruct));
			(yyval.finalProductStruct) = finalProductStruct;
		}
#line 2205 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 79:
#line 590 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			FinalRgbFunctionStruct finalRgbFunctionStruct;
			finalRgbFunctionStruct.Init((yyvsp[-8].mappedRegisterStruct), (yyvsp[-6].mappedRegisterStruct), (yyvsp[-4].mappedRegisterStruct), (yyvsp[-1].mappedRegisterStruct));
			(yyval.finalRgbFunctionStruct) = finalRgbFunctionStruct;
		}
#line 2215 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 80:
#line 596 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			FinalRgbFunctionStruct finalRgbFunctionStruct;
			finalRgbFunctionStruct.Init((yyvsp[-6].mappedRegisterStruct), (yyvsp[-4].mappedRegisterStruct), (yyvsp[-2].mappedRegisterStruct), (yyvsp[-10].mappedRegisterStruct));
			(yyval.finalRgbFunctionStruct) = finalRgbFunctionStruct;
		}
#line 2225 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 81:
#line 602 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			RegisterEnum zero;
			zero.word = RCP_ZERO;
			MappedRegisterStruct reg;
			reg.Init(zero, GL_UNSIGNED_IDENTITY_NV);
			FinalRgbFunctionStruct finalRgbFunctionStruct;
			finalRgbFunctionStruct.Init((yyvsp[-6].mappedRegisterStruct), (yyvsp[-4].mappedRegisterStruct), (yyvsp[-2].mappedRegisterStruct), reg);
			(yyval.finalRgbFunctionStruct) = finalRgbFunctionStruct;
		}
#line 2239 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 82:
#line 612 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			RegisterEnum zero;
			zero.word = RCP_ZERO;
			MappedRegisterStruct reg1;
			reg1.Init(zero, GL_UNSIGNED_IDENTITY_NV);
			MappedRegisterStruct reg2;
			reg2.Init(zero, GL_UNSIGNED_IDENTITY_NV);
			FinalRgbFunctionStruct finalRgbFunctionStruct;
			finalRgbFunctionStruct.Init((yyvsp[-3].mappedRegisterStruct), (yyvsp[-1].mappedRegisterStruct), reg1, reg2);
			(yyval.finalRgbFunctionStruct) = finalRgbFunctionStruct;
		}
#line 2255 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 83:
#line 624 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			RegisterEnum zero;
			zero.word = RCP_ZERO;
			MappedRegisterStruct reg1;
			reg1.Init(zero, GL_UNSIGNED_IDENTITY_NV);
			FinalRgbFunctionStruct finalRgbFunctionStruct;
			finalRgbFunctionStruct.Init((yyvsp[-5].mappedRegisterStruct), (yyvsp[-3].mappedRegisterStruct), reg1, (yyvsp[-1].mappedRegisterStruct));
			(yyval.finalRgbFunctionStruct) = finalRgbFunctionStruct;
		}
#line 2269 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 84:
#line 634 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			RegisterEnum zero;
			zero.word = RCP_ZERO;
			MappedRegisterStruct reg1;
			reg1.Init(zero, GL_UNSIGNED_IDENTITY_NV);
			MappedRegisterStruct reg2;
			reg2.Init(zero, GL_UNSIGNED_IDENTITY_NV);
			MappedRegisterStruct reg3;
			reg3.Init(zero, GL_UNSIGNED_IDENTITY_NV);
			FinalRgbFunctionStruct finalRgbFunctionStruct;
			finalRgbFunctionStruct.Init(reg1, reg2, reg3, (yyvsp[-1].mappedRegisterStruct));
			(yyval.finalRgbFunctionStruct) = finalRgbFunctionStruct;
		}
#line 2287 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 85:
#line 648 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			RegisterEnum zero;
			zero.word = RCP_ZERO;
			MappedRegisterStruct reg2;
			reg2.Init(zero, GL_UNSIGNED_INVERT_NV);
			MappedRegisterStruct reg3;
			reg3.Init(zero, GL_UNSIGNED_IDENTITY_NV);
			FinalRgbFunctionStruct finalRgbFunctionStruct;
			finalRgbFunctionStruct.Init((yyvsp[-3].mappedRegisterStruct), reg2, reg3, (yyvsp[-1].mappedRegisterStruct));
			(yyval.finalRgbFunctionStruct) = finalRgbFunctionStruct;
		}
#line 2303 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 86:
#line 662 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			FinalAlphaFunctionStruct finalAlphaFunctionStruct;
			finalAlphaFunctionStruct.Init((yyvsp[-1].mappedRegisterStruct));
			(yyval.finalAlphaFunctionStruct) = finalAlphaFunctionStruct;
		}
#line 2313 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 87:
#line 670 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			(yyval.registerEnum) = (yyvsp[0].registerEnum);
		}
#line 2321 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;

  case 88:
#line 674 "rc1.0_grammar.y" /* yacc.c:1646  */
    {
			(yyval.registerEnum) = (yyvsp[0].registerEnum);
		}
#line 2329 "_rc1.0_parser.c" /* yacc.c:1646  */
    break;


#line 2333 "_rc1.0_parser.c" /* yacc.c:1646  */
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
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

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 679 "rc1.0_grammar.y" /* yacc.c:1906  */

void yyerror(const char* s)
{
     errors.set("unrecognized token");
}
