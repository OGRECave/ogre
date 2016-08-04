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
#define yyparse         ts10_parse
#define yylex           ts10_lex
#define yyerror         ts10_error
#define yydebug         ts10_debug
#define yynerrs         ts10_nerrs

#define yylval          ts10_lval
#define yychar          ts10_char

/* Copy the first part of user declarations.  */
#line 3 "ts1.0_grammar.y" /* yacc.c:339  */

void yyerror(const char* s);
int yylex ( void );

#ifdef _WIN32
# include <windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include "ts1.0_inst.h"
#include "ts1.0_inst_list.h"
#include "nvparse_errors.h"
#include "nvparse_externs.h"


#line 90 "_ts1.0_parser.c" /* yacc.c:339  */

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
   by #include "_ts1.0_parser.h".  */
#ifndef YY_TS10_TS1_0_PARSER_H_INCLUDED
# define YY_TS10_TS1_0_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int ts10_debug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    floatValue = 258,
    gequal = 259,
    less = 260,
    texVariable = 261,
    expandString = 262,
    openParen = 263,
    closeParen = 264,
    semicolon = 265,
    comma = 266,
    nop = 267,
    texture_1d = 268,
    texture_2d = 269,
    texture_rectangle = 270,
    texture_3d = 271,
    texture_cube_map = 272,
    cull_fragment = 273,
    pass_through = 274,
    offset_2d_scale = 275,
    offset_2d = 276,
    offset_rectangle_scale = 277,
    offset_rectangle = 278,
    dependent_ar = 279,
    dependent_gb = 280,
    dot_product_2d_1of2 = 281,
    dot_product_2d_2of2 = 282,
    dot_product_rectangle_1of2 = 283,
    dot_product_rectangle_2of2 = 284,
    dot_product_depth_replace_1of2 = 285,
    dot_product_depth_replace_2of2 = 286,
    dot_product_3d_1of3 = 287,
    dot_product_3d_2of3 = 288,
    dot_product_3d_3of3 = 289,
    dot_product_cube_map_1of3 = 290,
    dot_product_cube_map_2of3 = 291,
    dot_product_cube_map_3of3 = 292,
    dot_product_reflect_cube_map_eye_from_qs_1of3 = 293,
    dot_product_reflect_cube_map_eye_from_qs_2of3 = 294,
    dot_product_reflect_cube_map_eye_from_qs_3of3 = 295,
    dot_product_reflect_cube_map_const_eye_1of3 = 296,
    dot_product_reflect_cube_map_const_eye_2of3 = 297,
    dot_product_reflect_cube_map_const_eye_3of3 = 298,
    dot_product_cube_map_and_reflect_cube_map_eye_from_qs_1of3 = 299,
    dot_product_cube_map_and_reflect_cube_map_eye_from_qs_2of3 = 300,
    dot_product_cube_map_and_reflect_cube_map_eye_from_qs_3of3 = 301,
    dot_product_cube_map_and_reflect_cube_map_const_eye_1of3 = 302,
    dot_product_cube_map_and_reflect_cube_map_const_eye_2of3 = 303,
    dot_product_cube_map_and_reflect_cube_map_const_eye_3of3 = 304
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE YYSTYPE;
union YYSTYPE
{
#line 18 "ts1.0_grammar.y" /* yacc.c:355  */

  float fval;
  InstPtr inst;
  InstListPtr instList;
  MappedVariablePtr variable;

#line 187 "_ts1.0_parser.c" /* yacc.c:355  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE ts10_lval;

int ts10_parse (void);

#endif /* !YY_TS10_TS1_0_PARSER_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 202 "_ts1.0_parser.c" /* yacc.c:358  */

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
#define YYFINAL  80
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   223

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  50
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  6
/* YYNRULES -- Number of rules.  */
#define YYNRULES  46
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  218

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   304

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
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,    73,    73,    81,    87,    97,   103,   111,   115,   119,
     123,   127,   131,   135,   139,   143,   147,   151,   155,   159,
     163,   167,   172,   177,   182,   187,   192,   197,   202,   207,
     212,   217,   222,   227,   232,   237,   242,   247,   252,   257,
     262,   267,   272,   277,   282,   289,   293
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "floatValue", "gequal", "less",
  "texVariable", "expandString", "openParen", "closeParen", "semicolon",
  "comma", "nop", "texture_1d", "texture_2d", "texture_rectangle",
  "texture_3d", "texture_cube_map", "cull_fragment", "pass_through",
  "offset_2d_scale", "offset_2d", "offset_rectangle_scale",
  "offset_rectangle", "dependent_ar", "dependent_gb",
  "dot_product_2d_1of2", "dot_product_2d_2of2",
  "dot_product_rectangle_1of2", "dot_product_rectangle_2of2",
  "dot_product_depth_replace_1of2", "dot_product_depth_replace_2of2",
  "dot_product_3d_1of3", "dot_product_3d_2of3", "dot_product_3d_3of3",
  "dot_product_cube_map_1of3", "dot_product_cube_map_2of3",
  "dot_product_cube_map_3of3",
  "dot_product_reflect_cube_map_eye_from_qs_1of3",
  "dot_product_reflect_cube_map_eye_from_qs_2of3",
  "dot_product_reflect_cube_map_eye_from_qs_3of3",
  "dot_product_reflect_cube_map_const_eye_1of3",
  "dot_product_reflect_cube_map_const_eye_2of3",
  "dot_product_reflect_cube_map_const_eye_3of3",
  "dot_product_cube_map_and_reflect_cube_map_eye_from_qs_1of3",
  "dot_product_cube_map_and_reflect_cube_map_eye_from_qs_2of3",
  "dot_product_cube_map_and_reflect_cube_map_eye_from_qs_3of3",
  "dot_product_cube_map_and_reflect_cube_map_const_eye_1of3",
  "dot_product_cube_map_and_reflect_cube_map_const_eye_2of3",
  "dot_product_cube_map_and_reflect_cube_map_const_eye_3of3", "$accept",
  "WholeEnchilada", "InstListDesc", "MappedVariableDesc", "InstDesc",
  "CondDesc", YY_NULLPTR
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
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304
};
# endif

#define YYPACT_NINF -127

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-127)))

#define YYTABLE_NINF -1

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     -11,    31,    35,    39,    40,    41,    42,    43,    44,    45,
      46,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,   107,
     -11,    36,    99,   100,   101,   102,   103,   104,    37,   105,
     109,   110,   111,   112,   113,   114,    38,    38,    38,    38,
      38,    38,    38,    38,    38,    38,    38,    38,    38,    38,
      38,    38,    38,    38,    38,    38,    38,    38,    38,    38,
    -127,   115,  -127,  -127,  -127,  -127,  -127,  -127,  -127,  -127,
    -127,   116,  -127,   117,   118,   119,   120,   123,   124,  -127,
     126,   127,   128,   129,   130,   131,   132,   133,   134,   135,
     136,   137,   138,   139,   140,   141,   142,   143,   145,   146,
     147,   148,   149,   150,   152,  -127,    37,   121,   155,   159,
     160,  -127,  -127,   158,  -127,  -127,  -127,  -127,  -127,  -127,
    -127,  -127,  -127,  -127,  -127,  -127,  -127,  -127,  -127,   162,
    -127,  -127,  -127,  -127,  -127,   163,  -127,  -127,   156,   157,
     161,   164,   165,   168,   167,   169,    37,   166,   170,   171,
     176,  -127,   178,   179,   172,   173,   174,   175,   177,   180,
     181,    37,   184,   186,   187,   190,   191,   192,   188,   185,
     189,   193,   194,   197,   198,  -127,   195,   196,   199,   200,
    -127,  -127,   201,   202,   203,   204,   205,  -127,   206,  -127,
     207,   208,   212,   213,   211,   214,  -127,  -127
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       2,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       1,     0,     4,     7,     8,     9,    10,    11,    12,    45,
      46,     0,    14,     0,     0,     0,     0,     0,     0,     6,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     3,     0,     0,     0,     0,
       0,    19,    20,     0,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,     0,
      37,    38,    39,    40,    41,     0,    43,    44,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     5,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    13,     0,     0,     0,     0,
      36,    42,     0,     0,     0,     0,     0,    16,     0,    18,
       0,     0,     0,     0,     0,     0,    15,    17
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -127,  -127,  -127,    -1,   182,  -126
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,    39,    40,   101,    41,    91
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
     158,     1,     2,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    42,
     174,    89,    90,    43,    99,   100,    82,    44,    45,    46,
      47,    48,    49,    50,    51,   188,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    83,    84,
      85,    86,    87,    88,    92,    93,    94,    95,    96,    97,
      98,     0,     0,     0,   159,   125,     0,   126,   127,   128,
     129,   130,   131,   132,   133,     0,   134,   135,   136,   137,
     138,   139,   140,   141,   142,   143,   144,   145,   146,   147,
     148,     0,   150,   149,   151,   152,   153,   154,   160,   156,
     155,   157,   161,   162,   163,   164,   165,   166,   167,   175,
       0,     0,   168,   176,   177,   169,   170,   171,   172,   178,
     173,   179,   180,   181,   182,   183,   184,   189,   185,   190,
     191,   186,   187,   192,   193,   194,   196,   195,   202,   203,
     197,     0,   204,   205,   198,   199,   200,   201,   210,   211,
       0,   207,   206,   209,   208,   214,   215,     0,   212,   213,
     216,     0,    81,   217
};

static const yytype_int16 yycheck[] =
{
     126,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,     8,
     166,     4,     5,     8,     6,     7,    10,     8,     8,     8,
       8,     8,     8,     8,     8,   181,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,     8,
       8,     8,     8,     8,     8,     8,     8,     8,     8,     8,
       8,     8,     8,     8,     8,     8,     8,     8,     8,     8,
       8,     8,     8,     8,     8,     8,     8,     0,     9,     9,
       9,     9,     9,     9,     9,     6,     6,     6,     6,     6,
       6,    -1,    -1,    -1,     3,    10,    -1,    11,    11,    11,
      11,    11,     9,     9,     8,    -1,     9,     9,     9,     9,
       9,     9,     9,     9,     9,     9,     9,     9,     9,     9,
       9,    -1,     9,    11,     9,     9,     9,     9,     3,     9,
      11,     9,     3,     3,     6,     3,     3,    11,    11,     3,
      -1,    -1,    11,     3,     3,    11,    11,     9,    11,     3,
      11,     3,     3,    11,    11,    11,    11,     3,    11,     3,
       3,    11,    11,     3,     3,     3,    11,     9,     3,     3,
      11,    -1,     3,     3,    11,    11,     9,     9,     3,     3,
      -1,     9,    11,     9,    11,     3,     3,    -1,    11,    11,
       9,    -1,    40,     9
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    51,
      52,    54,     8,     8,     8,     8,     8,     8,     8,     8,
       8,     8,     8,     8,     8,     8,     8,     8,     8,     8,
       8,     8,     8,     8,     8,     8,     8,     8,     8,     8,
       8,     8,     8,     8,     8,     8,     8,     8,     8,     8,
       0,    54,    10,     9,     9,     9,     9,     9,     9,     4,
       5,    55,     9,     6,     6,     6,     6,     6,     6,     6,
       7,    53,    53,    53,    53,    53,    53,    53,    53,    53,
      53,    53,    53,    53,    53,    53,    53,    53,    53,    53,
      53,    53,    53,    53,    53,    10,    11,    11,    11,    11,
      11,     9,     9,     8,     9,     9,     9,     9,     9,     9,
       9,     9,     9,     9,     9,     9,     9,     9,     9,    11,
       9,     9,     9,     9,     9,    11,     9,     9,    55,     3,
       3,     3,     3,     6,     3,     3,    11,    11,    11,    11,
      11,     9,    11,    11,    55,     3,     3,     3,     3,     3,
       3,    11,    11,    11,    11,    11,    11,    11,    55,     3,
       3,     3,     3,     3,     3,     9,    11,    11,    11,    11,
       9,     9,     3,     3,     3,     3,    11,     9,    11,     9,
       3,     3,    11,    11,     3,     3,     9,     9
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    50,    51,    52,    52,    53,    53,    54,    54,    54,
      54,    54,    54,    54,    54,    54,    54,    54,    54,    54,
      54,    54,    54,    54,    54,    54,    54,    54,    54,    54,
      54,    54,    54,    54,    54,    54,    54,    54,    54,    54,
      54,    54,    54,    54,    54,    55,    55
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     3,     2,     4,     1,     3,     3,     3,
       3,     3,     3,    10,     3,    16,    12,    16,    12,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,    10,     4,     4,     4,
       4,     4,    10,     4,     4,     1,     1
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
#line 74 "ts1.0_grammar.y" /* yacc.c:1646  */
    {
			(yyvsp[0].instList)->Validate();
			(yyvsp[0].instList)->Invoke();
		    delete (yyvsp[0].instList);
		}
#line 1423 "_ts1.0_parser.c" /* yacc.c:1646  */
    break;

  case 3:
#line 82 "ts1.0_grammar.y" /* yacc.c:1646  */
    {
		    *((yyvsp[-2].instList)) += (yyvsp[-1].inst);
			delete (yyvsp[-1].inst);
		    (yyval.instList) = (yyvsp[-2].instList);
		}
#line 1433 "_ts1.0_parser.c" /* yacc.c:1646  */
    break;

  case 4:
#line 88 "ts1.0_grammar.y" /* yacc.c:1646  */
    {
		    InstListPtr instList = new InstList;
		    *instList += (yyvsp[-1].inst);
			delete (yyvsp[-1].inst);
		    (yyval.instList) = instList;
		}
#line 1444 "_ts1.0_parser.c" /* yacc.c:1646  */
    break;

  case 5:
#line 98 "ts1.0_grammar.y" /* yacc.c:1646  */
    {
			(yyval.variable) = new MappedVariable;
			(yyval.variable)->var = (yyvsp[-1].fval);
			(yyval.variable)->expand = true;
		}
#line 1454 "_ts1.0_parser.c" /* yacc.c:1646  */
    break;

  case 6:
#line 104 "ts1.0_grammar.y" /* yacc.c:1646  */
    {
			(yyval.variable) = new MappedVariable;
			(yyval.variable)->var = (yyvsp[0].fval);
			(yyval.variable)->expand = false;
		}
#line 1464 "_ts1.0_parser.c" /* yacc.c:1646  */
    break;

  case 7:
#line 112 "ts1.0_grammar.y" /* yacc.c:1646  */
    {
		    (yyval.inst) = new Inst(TSP_NOP);
		}
#line 1472 "_ts1.0_parser.c" /* yacc.c:1646  */
    break;

  case 8:
#line 116 "ts1.0_grammar.y" /* yacc.c:1646  */
    {
		    (yyval.inst) = new Inst(TSP_TEXTURE_1D);
		}
#line 1480 "_ts1.0_parser.c" /* yacc.c:1646  */
    break;

  case 9:
#line 120 "ts1.0_grammar.y" /* yacc.c:1646  */
    {
		    (yyval.inst) = new Inst(TSP_TEXTURE_2D);
		}
#line 1488 "_ts1.0_parser.c" /* yacc.c:1646  */
    break;

  case 10:
#line 124 "ts1.0_grammar.y" /* yacc.c:1646  */
    {
		    (yyval.inst) = new Inst(TSP_TEXTURE_RECTANGLE);
		}
#line 1496 "_ts1.0_parser.c" /* yacc.c:1646  */
    break;

  case 11:
#line 128 "ts1.0_grammar.y" /* yacc.c:1646  */
    {
		    (yyval.inst) = new Inst(TSP_TEXTURE_3D);
		}
#line 1504 "_ts1.0_parser.c" /* yacc.c:1646  */
    break;

  case 12:
#line 132 "ts1.0_grammar.y" /* yacc.c:1646  */
    {
		    (yyval.inst) = new Inst(TSP_TEXTURE_CUBE_MAP);
		}
#line 1512 "_ts1.0_parser.c" /* yacc.c:1646  */
    break;

  case 13:
#line 136 "ts1.0_grammar.y" /* yacc.c:1646  */
    {
		    (yyval.inst) = new Inst(TSP_CULL_FRAGMENT, (yyvsp[-7].fval), (yyvsp[-5].fval), (yyvsp[-3].fval), (yyvsp[-1].fval));
		}
#line 1520 "_ts1.0_parser.c" /* yacc.c:1646  */
    break;

  case 14:
#line 140 "ts1.0_grammar.y" /* yacc.c:1646  */
    {
		    (yyval.inst) = new Inst(TSP_PASS_THROUGH);
		}
#line 1528 "_ts1.0_parser.c" /* yacc.c:1646  */
    break;

  case 15:
#line 144 "ts1.0_grammar.y" /* yacc.c:1646  */
    {
		    (yyval.inst) = new Inst(TSP_OFFSET_2D_SCALE, (yyvsp[-13].fval), (yyvsp[-11].fval), (yyvsp[-9].fval), (yyvsp[-7].fval), (yyvsp[-5].fval), (yyvsp[-3].fval), (yyvsp[-1].fval));
		}
#line 1536 "_ts1.0_parser.c" /* yacc.c:1646  */
    break;

  case 16:
#line 148 "ts1.0_grammar.y" /* yacc.c:1646  */
    {
		    (yyval.inst) = new Inst(TSP_OFFSET_2D, (yyvsp[-9].fval), (yyvsp[-7].fval), (yyvsp[-5].fval), (yyvsp[-3].fval), (yyvsp[-1].fval));
		}
#line 1544 "_ts1.0_parser.c" /* yacc.c:1646  */
    break;

  case 17:
#line 152 "ts1.0_grammar.y" /* yacc.c:1646  */
    {
		    (yyval.inst) = new Inst(TSP_OFFSET_RECTANGLE_SCALE, (yyvsp[-13].fval), (yyvsp[-11].fval), (yyvsp[-9].fval), (yyvsp[-7].fval), (yyvsp[-5].fval), (yyvsp[-3].fval), (yyvsp[-1].fval));
		}
#line 1552 "_ts1.0_parser.c" /* yacc.c:1646  */
    break;

  case 18:
#line 156 "ts1.0_grammar.y" /* yacc.c:1646  */
    {
		    (yyval.inst) = new Inst(TSP_OFFSET_RECTANGLE, (yyvsp[-9].fval), (yyvsp[-7].fval), (yyvsp[-5].fval), (yyvsp[-3].fval), (yyvsp[-1].fval));
		}
#line 1560 "_ts1.0_parser.c" /* yacc.c:1646  */
    break;

  case 19:
#line 160 "ts1.0_grammar.y" /* yacc.c:1646  */
    {
		    (yyval.inst) = new Inst(TSP_DEPENDENT_AR, (yyvsp[-1].fval));
		}
#line 1568 "_ts1.0_parser.c" /* yacc.c:1646  */
    break;

  case 20:
#line 164 "ts1.0_grammar.y" /* yacc.c:1646  */
    {
		    (yyval.inst) = new Inst(TSP_DEPENDENT_GB, (yyvsp[-1].fval));
		}
#line 1576 "_ts1.0_parser.c" /* yacc.c:1646  */
    break;

  case 21:
#line 168 "ts1.0_grammar.y" /* yacc.c:1646  */
    {
		    (yyval.inst) = new Inst(TSP_DOT_PRODUCT_2D_1_OF_2, (yyvsp[-1].variable));
			delete (yyvsp[-1].variable);
		}
#line 1585 "_ts1.0_parser.c" /* yacc.c:1646  */
    break;

  case 22:
#line 173 "ts1.0_grammar.y" /* yacc.c:1646  */
    {
		    (yyval.inst) = new Inst(TSP_DOT_PRODUCT_2D_2_OF_2, (yyvsp[-1].variable));
			delete (yyvsp[-1].variable);
		}
#line 1594 "_ts1.0_parser.c" /* yacc.c:1646  */
    break;

  case 23:
#line 178 "ts1.0_grammar.y" /* yacc.c:1646  */
    {
		    (yyval.inst) = new Inst(TSP_DOT_PRODUCT_RECTANGLE_1_OF_2, (yyvsp[-1].variable));
			delete (yyvsp[-1].variable);
		}
#line 1603 "_ts1.0_parser.c" /* yacc.c:1646  */
    break;

  case 24:
#line 183 "ts1.0_grammar.y" /* yacc.c:1646  */
    {
		    (yyval.inst) = new Inst(TSP_DOT_PRODUCT_RECTANGLE_2_OF_2, (yyvsp[-1].variable));
			delete (yyvsp[-1].variable);
		}
#line 1612 "_ts1.0_parser.c" /* yacc.c:1646  */
    break;

  case 25:
#line 188 "ts1.0_grammar.y" /* yacc.c:1646  */
    {
		    (yyval.inst) = new Inst(TSP_DOT_PRODUCT_DEPTH_REPLACE_1_OF_2, (yyvsp[-1].variable));
			delete (yyvsp[-1].variable);
		}
#line 1621 "_ts1.0_parser.c" /* yacc.c:1646  */
    break;

  case 26:
#line 193 "ts1.0_grammar.y" /* yacc.c:1646  */
    {
		    (yyval.inst) = new Inst(TSP_DOT_PRODUCT_DEPTH_REPLACE_2_OF_2, (yyvsp[-1].variable));
			delete (yyvsp[-1].variable);
		}
#line 1630 "_ts1.0_parser.c" /* yacc.c:1646  */
    break;

  case 27:
#line 198 "ts1.0_grammar.y" /* yacc.c:1646  */
    {
		    (yyval.inst) = new Inst(TSP_DOT_PRODUCT_3D_1_OF_3, (yyvsp[-1].variable));
			delete (yyvsp[-1].variable);
		}
#line 1639 "_ts1.0_parser.c" /* yacc.c:1646  */
    break;

  case 28:
#line 203 "ts1.0_grammar.y" /* yacc.c:1646  */
    {
		    (yyval.inst) = new Inst(TSP_DOT_PRODUCT_3D_2_OF_3, (yyvsp[-1].variable));
			delete (yyvsp[-1].variable);
		}
#line 1648 "_ts1.0_parser.c" /* yacc.c:1646  */
    break;

  case 29:
#line 208 "ts1.0_grammar.y" /* yacc.c:1646  */
    {
		    (yyval.inst) = new Inst(TSP_DOT_PRODUCT_3D_3_OF_3, (yyvsp[-1].variable));
			delete (yyvsp[-1].variable);
		}
#line 1657 "_ts1.0_parser.c" /* yacc.c:1646  */
    break;

  case 30:
#line 213 "ts1.0_grammar.y" /* yacc.c:1646  */
    {
		    (yyval.inst) = new Inst(TSP_DOT_PRODUCT_CUBE_MAP_1_OF_3, (yyvsp[-1].variable));
			delete (yyvsp[-1].variable);
		}
#line 1666 "_ts1.0_parser.c" /* yacc.c:1646  */
    break;

  case 31:
#line 218 "ts1.0_grammar.y" /* yacc.c:1646  */
    {
		    (yyval.inst) = new Inst(TSP_DOT_PRODUCT_CUBE_MAP_2_OF_3, (yyvsp[-1].variable));
			delete (yyvsp[-1].variable);
		}
#line 1675 "_ts1.0_parser.c" /* yacc.c:1646  */
    break;

  case 32:
#line 223 "ts1.0_grammar.y" /* yacc.c:1646  */
    {
		    (yyval.inst) = new Inst(TSP_DOT_PRODUCT_CUBE_MAP_3_OF_3, (yyvsp[-1].variable));
			delete (yyvsp[-1].variable);
		}
#line 1684 "_ts1.0_parser.c" /* yacc.c:1646  */
    break;

  case 33:
#line 228 "ts1.0_grammar.y" /* yacc.c:1646  */
    {
		    (yyval.inst) = new Inst(TSP_DOT_PRODUCT_REFLECT_CUBE_MAP_EYE_FROM_QS_1_OF_3, (yyvsp[-1].variable));
			delete (yyvsp[-1].variable);
		}
#line 1693 "_ts1.0_parser.c" /* yacc.c:1646  */
    break;

  case 34:
#line 233 "ts1.0_grammar.y" /* yacc.c:1646  */
    {
		    (yyval.inst) = new Inst(TSP_DOT_PRODUCT_REFLECT_CUBE_MAP_EYE_FROM_QS_2_OF_3, (yyvsp[-1].variable));
			delete (yyvsp[-1].variable);
		}
#line 1702 "_ts1.0_parser.c" /* yacc.c:1646  */
    break;

  case 35:
#line 238 "ts1.0_grammar.y" /* yacc.c:1646  */
    {
		    (yyval.inst) = new Inst(TSP_DOT_PRODUCT_REFLECT_CUBE_MAP_EYE_FROM_QS_3_OF_3, (yyvsp[-1].variable));
			delete (yyvsp[-1].variable);
		}
#line 1711 "_ts1.0_parser.c" /* yacc.c:1646  */
    break;

  case 36:
#line 243 "ts1.0_grammar.y" /* yacc.c:1646  */
    {
		    (yyval.inst) = new Inst(TSP_DOT_PRODUCT_REFLECT_CUBE_MAP_CONST_EYE_1_OF_3, (yyvsp[-7].variable), (yyvsp[-5].fval), (yyvsp[-3].fval), (yyvsp[-1].fval));
			delete (yyvsp[-7].variable);
		}
#line 1720 "_ts1.0_parser.c" /* yacc.c:1646  */
    break;

  case 37:
#line 248 "ts1.0_grammar.y" /* yacc.c:1646  */
    {
		    (yyval.inst) = new Inst(TSP_DOT_PRODUCT_REFLECT_CUBE_MAP_CONST_EYE_2_OF_3, (yyvsp[-1].variable));
			delete (yyvsp[-1].variable);
		}
#line 1729 "_ts1.0_parser.c" /* yacc.c:1646  */
    break;

  case 38:
#line 253 "ts1.0_grammar.y" /* yacc.c:1646  */
    {
		    (yyval.inst) = new Inst(TSP_DOT_PRODUCT_REFLECT_CUBE_MAP_CONST_EYE_3_OF_3, (yyvsp[-1].variable));
			delete (yyvsp[-1].variable);
		}
#line 1738 "_ts1.0_parser.c" /* yacc.c:1646  */
    break;

  case 39:
#line 258 "ts1.0_grammar.y" /* yacc.c:1646  */
    {
		    (yyval.inst) = new Inst(TSP_DOT_PRODUCT_CUBE_MAP_AND_REFLECT_CUBE_MAP_EYE_FROM_QS_1_OF_3, (yyvsp[-1].variable));
			delete (yyvsp[-1].variable);
		}
#line 1747 "_ts1.0_parser.c" /* yacc.c:1646  */
    break;

  case 40:
#line 263 "ts1.0_grammar.y" /* yacc.c:1646  */
    {
		    (yyval.inst) = new Inst(TSP_DOT_PRODUCT_CUBE_MAP_AND_REFLECT_CUBE_MAP_EYE_FROM_QS_2_OF_3, (yyvsp[-1].variable));
			delete (yyvsp[-1].variable);
		}
#line 1756 "_ts1.0_parser.c" /* yacc.c:1646  */
    break;

  case 41:
#line 268 "ts1.0_grammar.y" /* yacc.c:1646  */
    {
		    (yyval.inst) = new Inst(TSP_DOT_PRODUCT_CUBE_MAP_AND_REFLECT_CUBE_MAP_EYE_FROM_QS_3_OF_3, (yyvsp[-1].variable));
			delete (yyvsp[-1].variable);
		}
#line 1765 "_ts1.0_parser.c" /* yacc.c:1646  */
    break;

  case 42:
#line 273 "ts1.0_grammar.y" /* yacc.c:1646  */
    {
		    (yyval.inst) = new Inst(TSP_DOT_PRODUCT_CUBE_MAP_AND_REFLECT_CUBE_MAP_CONST_EYE_1_OF_3, (yyvsp[-7].variable), (yyvsp[-5].fval), (yyvsp[-3].fval), (yyvsp[-1].fval));
			delete (yyvsp[-7].variable);
		}
#line 1774 "_ts1.0_parser.c" /* yacc.c:1646  */
    break;

  case 43:
#line 278 "ts1.0_grammar.y" /* yacc.c:1646  */
    {
		    (yyval.inst) = new Inst(TSP_DOT_PRODUCT_CUBE_MAP_AND_REFLECT_CUBE_MAP_CONST_EYE_2_OF_3, (yyvsp[-1].variable));
			delete (yyvsp[-1].variable);
		}
#line 1783 "_ts1.0_parser.c" /* yacc.c:1646  */
    break;

  case 44:
#line 283 "ts1.0_grammar.y" /* yacc.c:1646  */
    {
		    (yyval.inst) = new Inst(TSP_DOT_PRODUCT_CUBE_MAP_AND_REFLECT_CUBE_MAP_CONST_EYE_3_OF_3, (yyvsp[-1].variable));
			delete (yyvsp[-1].variable);
		}
#line 1792 "_ts1.0_parser.c" /* yacc.c:1646  */
    break;

  case 45:
#line 290 "ts1.0_grammar.y" /* yacc.c:1646  */
    {
			(yyval.fval) = (yyvsp[0].fval);
		}
#line 1800 "_ts1.0_parser.c" /* yacc.c:1646  */
    break;

  case 46:
#line 294 "ts1.0_grammar.y" /* yacc.c:1646  */
    {
			(yyval.fval) = (yyvsp[0].fval);
		}
#line 1808 "_ts1.0_parser.c" /* yacc.c:1646  */
    break;


#line 1812 "_ts1.0_parser.c" /* yacc.c:1646  */
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
#line 300 "ts1.0_grammar.y" /* yacc.c:1906  */

void yyerror(const char* s)
{
     errors.set("unrecognized token");
}
