/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
/* A Bison parser, made by GNU Bison 1.875.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Written by Richard Stallman by simplifying the original so called
   ``semantic'' parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */
#include <OgreGLPrerequisites.h>
/* Identify Bison output.  */
#define YYBISON 1

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0

/* If NAME_PREFIX is specified substitute the variables and functions
   names.  */
#define yyparse rc10_parse
#define yylex   rc10_lex
#define yyerror rc10_error
#define yylval  rc10_lval
#define yychar  rc10_char
#define yydebug rc10_debug
#define yynerrs rc10_nerrs


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
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
#define regVariable 258
#define constVariable 259
#define color_sum 260
#define final_product 261
#define expandString 262
#define halfBiasString 263
#define unsignedString 264
#define unsignedInvertString 265
#define muxString 266
#define sumString 267
#define rgb_portion 268
#define alpha_portion 269
#define openParen 270
#define closeParen 271
#define openBracket 272
#define closeBracket 273
#define semicolon 274
#define comma 275
#define dot 276
#define times 277
#define minus 278
#define equals 279
#define plus 280
#define bias_by_negative_one_half_scale_by_two 281
#define bias_by_negative_one_half 282
#define scale_by_one_half 283
#define scale_by_two 284
#define scale_by_four 285
#define clamp_color_sum 286
#define lerp 287
#define fragment_rgb 288
#define fragment_alpha 289
#define floatValue 290




/* Copy the first part of user declarations.  */
#line 3 "rc1.0_grammar.y"

void yyerror(char* s);
int yylex ( void );

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  define NOMINMAX // required to stop windows.h messing up std::min
#  include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#include "rc1.0_combiners.h"
#include "nvparse_errors.h"
#include "nvparse_externs.h"




/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 20 "rc1.0_grammar.y"
typedef union YYSTYPE {
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
} YYSTYPE;
/* Line 191 of yacc.c.  */
#line 191 "_rc1.0_parser.c"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 214 of yacc.c.  */
#line 203 "_rc1.0_parser.c"

#if ! defined (yyoverflow) || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# if YYSTACK_USE_ALLOCA
#  define YYSTACK_ALLOC alloca
# else
#  ifndef YYSTACK_USE_ALLOCA
#   if defined (alloca) || defined (_ALLOCA_H)
#    define YYSTACK_ALLOC alloca
#   else
#    ifdef __GNUC__
#     define YYSTACK_ALLOC __builtin_alloca
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC malloc
#  define YYSTACK_FREE free
# endif
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE))				\
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  27
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   258

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  36
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  22
/* YYNRULES -- Number of rules. */
#define YYNRULES  88
/* YYNRULES -- Number of states. */
#define YYNSTATES  220

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   290

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
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
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     5,     9,    14,    17,    20,    24,    26,
      39,    42,    44,    49,    55,    62,    66,    71,    77,    83,
      88,    90,    92,    94,    97,   102,   108,   113,   119,   124,
     129,   132,   135,   138,   140,   143,   147,   151,   153,   160,
     167,   172,   179,   186,   191,   196,   201,   206,   211,   213,
     218,   223,   225,   230,   235,   237,   242,   247,   250,   254,
     258,   263,   268,   271,   275,   279,   284,   289,   291,   294,
     297,   301,   305,   307,   310,   313,   317,   321,   326,   333,
     347,   361,   373,   380,   389,   394,   401,   406,   408
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      37,     0,    -1,    38,    -1,    39,    40,    52,    -1,    39,
      39,    40,    52,    -1,    40,    52,    -1,    39,    52,    -1,
      39,    39,    52,    -1,    52,    -1,     4,    24,    15,    35,
      20,    35,    20,    35,    20,    35,    16,    19,    -1,    40,
      41,    -1,    41,    -1,    17,    42,    42,    18,    -1,    17,
      39,    42,    42,    18,    -1,    17,    39,    39,    42,    42,
      18,    -1,    17,    42,    18,    -1,    17,    39,    42,    18,
      -1,    17,    39,    39,    42,    18,    -1,    43,    17,    45,
      50,    18,    -1,    43,    17,    45,    18,    -1,    13,    -1,
      14,    -1,    57,    -1,    23,    57,    -1,     7,    15,    57,
      16,    -1,    23,     7,    15,    57,    16,    -1,     8,    15,
      57,    16,    -1,    23,     8,    15,    57,    16,    -1,     9,
      15,    57,    16,    -1,    10,    15,    57,    16,    -1,    46,
      46,    -1,    46,    47,    -1,    47,    46,    -1,    46,    -1,
      47,    47,    -1,    47,    47,    48,    -1,    47,    47,    49,
      -1,    47,    -1,    57,    24,    44,    21,    44,    19,    -1,
      57,    24,    44,    22,    44,    19,    -1,    57,    24,    44,
      19,    -1,    57,    24,    11,    15,    16,    19,    -1,    57,
      24,    12,    15,    16,    19,    -1,    26,    15,    16,    19,
      -1,    27,    15,    16,    19,    -1,    28,    15,    16,    19,
      -1,    29,    15,    16,    19,    -1,    30,    15,    16,    19,
      -1,    57,    -1,     9,    15,    57,    16,    -1,    10,    15,
      57,    16,    -1,     5,    -1,     9,    15,     5,    16,    -1,
      10,    15,     5,    16,    -1,     6,    -1,     9,    15,     6,
      16,    -1,    10,    15,     6,    16,    -1,    56,    55,    -1,
      53,    56,    55,    -1,    54,    56,    55,    -1,    53,    54,
      56,    55,    -1,    54,    53,    56,    55,    -1,    55,    56,
      -1,    53,    55,    56,    -1,    54,    55,    56,    -1,    53,
      54,    55,    56,    -1,    54,    53,    55,    56,    -1,    55,
      -1,    53,    55,    -1,    54,    55,    -1,    53,    54,    55,
      -1,    54,    53,    55,    -1,    56,    -1,    53,    56,    -1,
      54,    56,    -1,    53,    54,    56,    -1,    54,    53,    56,
      -1,    31,    15,    16,    19,    -1,     6,    24,    51,    22,
      51,    19,    -1,    33,    24,    32,    15,    51,    20,    51,
      20,    51,    16,    25,    51,    19,    -1,    33,    24,    51,
      25,    32,    15,    51,    20,    51,    20,    51,    16,    19,
      -1,    33,    24,    32,    15,    51,    20,    51,    20,    51,
      16,    19,    -1,    33,    24,    51,    22,    51,    19,    -1,
      33,    24,    51,    22,    51,    25,    51,    19,    -1,    33,
      24,    51,    19,    -1,    33,    24,    51,    25,    51,    19,
      -1,    34,    24,    51,    19,    -1,     4,    -1,     3,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
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

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "regVariable", "constVariable", 
  "color_sum", "final_product", "expandString", "halfBiasString", 
  "unsignedString", "unsignedInvertString", "muxString", "sumString", 
  "rgb_portion", "alpha_portion", "openParen", "closeParen", 
  "openBracket", "closeBracket", "semicolon", "comma", "dot", "times", 
  "minus", "equals", "plus", "bias_by_negative_one_half_scale_by_two", 
  "bias_by_negative_one_half", "scale_by_one_half", "scale_by_two", 
  "scale_by_four", "clamp_color_sum", "lerp", "fragment_rgb", 
  "fragment_alpha", "floatValue", "$accept", "WholeEnchilada", 
  "Combiners", "ConstColor", "GeneralCombiners", "GeneralCombiner", 
  "GeneralPortion", "PortionDesignator", "GeneralMappedRegister", 
  "GeneralFunction", "Dot", "Mul", "Mux", "Sum", "BiasScale", 
  "FinalMappedRegister", "FinalCombiner", "ClampColorSum", "FinalProduct", 
  "FinalRgbFunction", "FinalAlphaFunction", "Register", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
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

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
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

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
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

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,     7,     8,     9,    10,    11,    22,    23,   143,    78,
      79,    80,   135,   136,   110,    48,    12,    13,    14,    15,
      16,    49
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -63
static const short yypact[] =
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

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
     -63,   -63,   -63,     9,    -2,     0,   -18,   -63,   -62,   -63,
      39,    61,   -63,   -63,   -63,   -25,    -4,   190,   208,     1,
       8,   -52
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const unsigned char yytable[] =
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

static const short yycheck[] =
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
static const unsigned char yystos[] =
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

#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrlab1


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");\
      YYERROR;							\
    }								\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)         \
  Current.first_line   = Rhs[1].first_line;      \
  Current.first_column = Rhs[1].first_column;    \
  Current.last_line    = Rhs[N].last_line;       \
  Current.last_column  = Rhs[N].last_column;
#endif

/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (0)

# define YYDSYMPRINT(Args)			\
do {						\
  if (yydebug)					\
    yysymprint Args;				\
} while (0)

# define YYDSYMPRINTF(Title, Token, Value, Location)		\
do {								\
  if (yydebug)							\
    {								\
      YYFPRINTF (stderr, "%s ", Title);				\
      yysymprint (stderr, 					\
                  Token, Value);	\
      YYFPRINTF (stderr, "\n");					\
    }								\
} while (0)

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (cinluded).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (short *bottom, short *top)
#else
static void
yy_stack_print (bottom, top)
    short *bottom;
    short *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (/* Nothing. */; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_reduce_print (int yyrule)
#else
static void
yy_reduce_print (yyrule)
    int yyrule;
#endif
{
  int yyi;
  unsigned int yylineno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %u), ",
             yyrule - 1, yylineno);
  /* Print the symbols being reduced, and their result.  */
  for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
    YYFPRINTF (stderr, "%s ", yytname [yyrhs[yyi]]);
  YYFPRINTF (stderr, "-> %s\n", yytname [yyr1[yyrule]]);
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (Rule);		\
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YYDSYMPRINT(Args)
# define YYDSYMPRINTF(Title, Token, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#if YYMAXDEPTH == 0
# undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

#endif /* !YYERROR_VERBOSE */



#if YYDEBUG
/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yysymprint (FILE *yyoutput, int yytype, YYSTYPE *yyvaluep)
#else
static void
yysymprint (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  if (yytype < YYNTOKENS)
    {
      YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
# ifdef YYPRINT
      YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
    }
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  switch (yytype)
    {
      default:
        break;
    }
  YYFPRINTF (yyoutput, ")");
}

#endif /* ! YYDEBUG */
/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yydestruct (int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yytype, yyvaluep)
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  switch (yytype)
    {

      default:
        break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM);
# else
int yyparse ();
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM)
# else
int yyparse (YYPARSE_PARAM)
  void *YYPARSE_PARAM;
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  short	yyssa[YYINITDEPTH];
  short *yyss = yyssa;
  register short *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;



#define YYPOPSTACK   (yyvsp--, yyssp--)

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* When reducing, the number of symbols on the RHS of the reduced
     rule.  */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	short *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

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

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YYDSYMPRINTF ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */
  YYDPRINTF ((stderr, "Shifting token %s, ", yytname[yytoken]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;


  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
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
     `$$ = $1'.

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
#line 74 "rc1.0_grammar.y"
    {
			yyvsp[0].combinersStruct.Validate();
			yyvsp[0].combinersStruct.Invoke();
		;}
    break;

  case 3:
#line 81 "rc1.0_grammar.y"
    {
			CombinersStruct combinersStruct;
			combinersStruct.Init(yyvsp[-1].generalCombinersStruct, yyvsp[0].finalCombinerStruct, yyvsp[-2].constColorStruct);
			yyval.combinersStruct = combinersStruct;
		;}
    break;

  case 4:
#line 87 "rc1.0_grammar.y"
    {
			CombinersStruct combinersStruct;
			combinersStruct.Init(yyvsp[-1].generalCombinersStruct, yyvsp[0].finalCombinerStruct, yyvsp[-3].constColorStruct, yyvsp[-2].constColorStruct);
			yyval.combinersStruct = combinersStruct;
		;}
    break;

  case 5:
#line 93 "rc1.0_grammar.y"
    {
			CombinersStruct combinersStruct;
			combinersStruct.Init(yyvsp[-1].generalCombinersStruct, yyvsp[0].finalCombinerStruct);
			yyval.combinersStruct = combinersStruct;
		;}
    break;

  case 6:
#line 99 "rc1.0_grammar.y"
    {
			GeneralCombinersStruct generalCombinersStruct;
			generalCombinersStruct.Init();
			CombinersStruct combinersStruct;
			combinersStruct.Init(generalCombinersStruct, yyvsp[0].finalCombinerStruct, yyvsp[-1].constColorStruct);
			yyval.combinersStruct = combinersStruct;
		;}
    break;

  case 7:
#line 107 "rc1.0_grammar.y"
    {
			GeneralCombinersStruct generalCombinersStruct;
			generalCombinersStruct.Init();
			CombinersStruct combinersStruct;
			combinersStruct.Init(generalCombinersStruct, yyvsp[0].finalCombinerStruct, yyvsp[-2].constColorStruct, yyvsp[-1].constColorStruct);
			yyval.combinersStruct = combinersStruct;
		;}
    break;

  case 8:
#line 115 "rc1.0_grammar.y"
    {
			GeneralCombinersStruct generalCombinersStruct;
			generalCombinersStruct.Init();
			CombinersStruct combinersStruct;
			combinersStruct.Init(generalCombinersStruct, yyvsp[0].finalCombinerStruct);
			yyval.combinersStruct = combinersStruct;
		;}
    break;

  case 9:
#line 125 "rc1.0_grammar.y"
    {
			ConstColorStruct constColorStruct;
			constColorStruct.Init(yyvsp[-11].registerEnum, yyvsp[-8].fval, yyvsp[-6].fval, yyvsp[-4].fval, yyvsp[-2].fval);
			yyval.constColorStruct = constColorStruct;
		;}
    break;

  case 10:
#line 134 "rc1.0_grammar.y"
    {
			yyvsp[-1].generalCombinersStruct += yyvsp[0].generalCombinerStruct;
			yyval.generalCombinersStruct = yyvsp[-1].generalCombinersStruct;
		;}
    break;

  case 11:
#line 139 "rc1.0_grammar.y"
    {
			GeneralCombinersStruct generalCombinersStruct;
			generalCombinersStruct.Init(yyvsp[0].generalCombinerStruct);
			yyval.generalCombinersStruct = generalCombinersStruct;
		;}
    break;

  case 12:
#line 147 "rc1.0_grammar.y"
    {
			GeneralCombinerStruct generalCombinerStruct;
			generalCombinerStruct.Init(yyvsp[-2].generalPortionStruct, yyvsp[-1].generalPortionStruct);
			yyval.generalCombinerStruct = generalCombinerStruct;
		;}
    break;

  case 13:
#line 153 "rc1.0_grammar.y"
    {
			GeneralCombinerStruct generalCombinerStruct;
			generalCombinerStruct.Init(yyvsp[-2].generalPortionStruct, yyvsp[-1].generalPortionStruct, yyvsp[-3].constColorStruct);
			yyval.generalCombinerStruct = generalCombinerStruct;
		;}
    break;

  case 14:
#line 159 "rc1.0_grammar.y"
    {
			GeneralCombinerStruct generalCombinerStruct;
			generalCombinerStruct.Init(yyvsp[-2].generalPortionStruct, yyvsp[-1].generalPortionStruct, yyvsp[-4].constColorStruct, yyvsp[-3].constColorStruct);
			yyval.generalCombinerStruct = generalCombinerStruct;
		;}
    break;

  case 15:
#line 165 "rc1.0_grammar.y"
    {
			GeneralCombinerStruct generalCombinerStruct;
			generalCombinerStruct.Init(yyvsp[-1].generalPortionStruct);
			yyval.generalCombinerStruct = generalCombinerStruct;
		;}
    break;

  case 16:
#line 171 "rc1.0_grammar.y"
    {
			GeneralCombinerStruct generalCombinerStruct;
			generalCombinerStruct.Init(yyvsp[-1].generalPortionStruct, yyvsp[-2].constColorStruct);
			yyval.generalCombinerStruct = generalCombinerStruct;
		;}
    break;

  case 17:
#line 177 "rc1.0_grammar.y"
    {
			GeneralCombinerStruct generalCombinerStruct;
			generalCombinerStruct.Init(yyvsp[-1].generalPortionStruct, yyvsp[-3].constColorStruct, yyvsp[-2].constColorStruct);
			yyval.generalCombinerStruct = generalCombinerStruct;
		;}
    break;

  case 18:
#line 185 "rc1.0_grammar.y"
    {
			GeneralPortionStruct generalPortionStruct;
			generalPortionStruct.Init(yyvsp[-4].ival, yyvsp[-2].generalFunctionStruct, yyvsp[-1].biasScaleEnum);
			yyval.generalPortionStruct = generalPortionStruct;
		;}
    break;

  case 19:
#line 191 "rc1.0_grammar.y"
    {
			BiasScaleEnum noScale;
			noScale.word = RCP_SCALE_BY_ONE;
			GeneralPortionStruct generalPortionStruct;
			generalPortionStruct.Init(yyvsp[-3].ival, yyvsp[-1].generalFunctionStruct, noScale);
			yyval.generalPortionStruct = generalPortionStruct;
		;}
    break;

  case 20:
#line 201 "rc1.0_grammar.y"
    {
			yyval.ival = yyvsp[0].ival;
		;}
    break;

  case 21:
#line 205 "rc1.0_grammar.y"
    {
			yyval.ival = yyvsp[0].ival;
		;}
    break;

  case 22:
#line 211 "rc1.0_grammar.y"
    {
			MappedRegisterStruct reg;
			reg.Init(yyvsp[0].registerEnum, GL_SIGNED_IDENTITY_NV);
			yyval.mappedRegisterStruct = reg;
		;}
    break;

  case 23:
#line 217 "rc1.0_grammar.y"
    {
			MappedRegisterStruct reg;
			reg.Init(yyvsp[0].registerEnum, GL_SIGNED_NEGATE_NV);
			yyval.mappedRegisterStruct = reg;
		;}
    break;

  case 24:
#line 223 "rc1.0_grammar.y"
    {
			MappedRegisterStruct reg;
			reg.Init(yyvsp[-1].registerEnum, GL_EXPAND_NORMAL_NV);
			yyval.mappedRegisterStruct = reg;
		;}
    break;

  case 25:
#line 229 "rc1.0_grammar.y"
    {
			MappedRegisterStruct reg;
			reg.Init(yyvsp[-1].registerEnum, GL_EXPAND_NEGATE_NV);
			yyval.mappedRegisterStruct = reg;
		;}
    break;

  case 26:
#line 235 "rc1.0_grammar.y"
    {
			MappedRegisterStruct reg;
			reg.Init(yyvsp[-1].registerEnum, GL_HALF_BIAS_NORMAL_NV);
			yyval.mappedRegisterStruct = reg;
		;}
    break;

  case 27:
#line 241 "rc1.0_grammar.y"
    {
			MappedRegisterStruct reg;
			reg.Init(yyvsp[-1].registerEnum, GL_HALF_BIAS_NEGATE_NV);
			yyval.mappedRegisterStruct = reg;
		;}
    break;

  case 28:
#line 247 "rc1.0_grammar.y"
    {
			MappedRegisterStruct reg;
			reg.Init(yyvsp[-1].registerEnum, GL_UNSIGNED_IDENTITY_NV);
			yyval.mappedRegisterStruct = reg;
		;}
    break;

  case 29:
#line 253 "rc1.0_grammar.y"
    {
			MappedRegisterStruct reg;
			reg.Init(yyvsp[-1].registerEnum, GL_UNSIGNED_INVERT_NV);
			yyval.mappedRegisterStruct = reg;
		;}
    break;

  case 30:
#line 261 "rc1.0_grammar.y"
    {
			GeneralFunctionStruct generalFunction;
			generalFunction.Init(yyvsp[-1].opStruct, yyvsp[0].opStruct);
			yyval.generalFunctionStruct = generalFunction;
		;}
    break;

  case 31:
#line 267 "rc1.0_grammar.y"
    {
			GeneralFunctionStruct generalFunction;
			generalFunction.Init(yyvsp[-1].opStruct, yyvsp[0].opStruct);
			yyval.generalFunctionStruct = generalFunction;
		;}
    break;

  case 32:
#line 273 "rc1.0_grammar.y"
    {
			GeneralFunctionStruct generalFunction;
			generalFunction.Init(yyvsp[-1].opStruct, yyvsp[0].opStruct);
			yyval.generalFunctionStruct = generalFunction;
		;}
    break;

  case 33:
#line 279 "rc1.0_grammar.y"
    {
			GeneralFunctionStruct generalFunction;
			generalFunction.Init(yyvsp[0].opStruct);
			yyval.generalFunctionStruct = generalFunction;
		;}
    break;

  case 34:
#line 285 "rc1.0_grammar.y"
    {
			GeneralFunctionStruct generalFunction;
			generalFunction.Init(yyvsp[-1].opStruct, yyvsp[0].opStruct);
			yyval.generalFunctionStruct = generalFunction;
		;}
    break;

  case 35:
#line 291 "rc1.0_grammar.y"
    {
			GeneralFunctionStruct generalFunction;
			generalFunction.Init(yyvsp[-2].opStruct, yyvsp[-1].opStruct, yyvsp[0].opStruct);
			yyval.generalFunctionStruct = generalFunction;
		;}
    break;

  case 36:
#line 297 "rc1.0_grammar.y"
    {
			GeneralFunctionStruct generalFunction;
			generalFunction.Init(yyvsp[-2].opStruct, yyvsp[-1].opStruct, yyvsp[0].opStruct);
			yyval.generalFunctionStruct = generalFunction;
		;}
    break;

  case 37:
#line 303 "rc1.0_grammar.y"
    {
			GeneralFunctionStruct generalFunction;
			generalFunction.Init(yyvsp[0].opStruct);
			yyval.generalFunctionStruct = generalFunction;
		;}
    break;

  case 38:
#line 311 "rc1.0_grammar.y"
    {
			OpStruct dotFunction;
			dotFunction.Init(RCP_DOT, yyvsp[-5].registerEnum, yyvsp[-3].mappedRegisterStruct, yyvsp[-1].mappedRegisterStruct);
			yyval.opStruct = dotFunction;
		;}
    break;

  case 39:
#line 319 "rc1.0_grammar.y"
    {
			OpStruct mulFunction;
			mulFunction.Init(RCP_MUL, yyvsp[-5].registerEnum, yyvsp[-3].mappedRegisterStruct, yyvsp[-1].mappedRegisterStruct);
			yyval.opStruct = mulFunction;
		;}
    break;

  case 40:
#line 325 "rc1.0_grammar.y"
    {
			RegisterEnum zero;
			zero.word = RCP_ZERO;
			MappedRegisterStruct one;
			one.Init(zero, GL_UNSIGNED_INVERT_NV);
			OpStruct mulFunction;
			mulFunction.Init(RCP_MUL, yyvsp[-3].registerEnum, yyvsp[-1].mappedRegisterStruct, one);
			yyval.opStruct = mulFunction;
		;}
    break;

  case 41:
#line 337 "rc1.0_grammar.y"
    {
			OpStruct muxFunction;
			muxFunction.Init(RCP_MUX, yyvsp[-5].registerEnum);
			yyval.opStruct = muxFunction;
		;}
    break;

  case 42:
#line 345 "rc1.0_grammar.y"
    {
			OpStruct sumFunction;
			sumFunction.Init(RCP_SUM, yyvsp[-5].registerEnum);
			yyval.opStruct = sumFunction;
		;}
    break;

  case 43:
#line 353 "rc1.0_grammar.y"
    {
			yyval.biasScaleEnum = yyvsp[-3].biasScaleEnum;
		;}
    break;

  case 44:
#line 357 "rc1.0_grammar.y"
    {
			yyval.biasScaleEnum = yyvsp[-3].biasScaleEnum;
		;}
    break;

  case 45:
#line 361 "rc1.0_grammar.y"
    {
			yyval.biasScaleEnum = yyvsp[-3].biasScaleEnum;
		;}
    break;

  case 46:
#line 365 "rc1.0_grammar.y"
    {
			yyval.biasScaleEnum = yyvsp[-3].biasScaleEnum;
		;}
    break;

  case 47:
#line 369 "rc1.0_grammar.y"
    {
			yyval.biasScaleEnum = yyvsp[-3].biasScaleEnum;
		;}
    break;

  case 48:
#line 375 "rc1.0_grammar.y"
    {
			MappedRegisterStruct reg;
			reg.Init(yyvsp[0].registerEnum, GL_UNSIGNED_IDENTITY_NV);
			yyval.mappedRegisterStruct = reg;
		;}
    break;

  case 49:
#line 381 "rc1.0_grammar.y"
    {
			MappedRegisterStruct reg;
			reg.Init(yyvsp[-1].registerEnum, GL_UNSIGNED_IDENTITY_NV);
			yyval.mappedRegisterStruct = reg;
		;}
    break;

  case 50:
#line 387 "rc1.0_grammar.y"
    {
			MappedRegisterStruct reg;
			reg.Init(yyvsp[-1].registerEnum, GL_UNSIGNED_INVERT_NV);
			yyval.mappedRegisterStruct = reg;
		;}
    break;

  case 51:
#line 393 "rc1.0_grammar.y"
    {
			MappedRegisterStruct reg;
			reg.Init(yyvsp[0].registerEnum);
			yyval.mappedRegisterStruct = reg;
		;}
    break;

  case 52:
#line 399 "rc1.0_grammar.y"
    {
			MappedRegisterStruct reg;
			reg.Init(yyvsp[-1].registerEnum, GL_UNSIGNED_IDENTITY_NV);
			yyval.mappedRegisterStruct = reg;
		;}
    break;

  case 53:
#line 405 "rc1.0_grammar.y"
    {
			MappedRegisterStruct reg;
			reg.Init(yyvsp[-1].registerEnum, GL_UNSIGNED_INVERT_NV);
			yyval.mappedRegisterStruct = reg;
		;}
    break;

  case 54:
#line 411 "rc1.0_grammar.y"
    {
			MappedRegisterStruct reg;
			reg.Init(yyvsp[0].registerEnum);
			yyval.mappedRegisterStruct = reg;
		;}
    break;

  case 55:
#line 417 "rc1.0_grammar.y"
    {
			MappedRegisterStruct reg;
			reg.Init(yyvsp[-1].registerEnum, GL_UNSIGNED_IDENTITY_NV);
			yyval.mappedRegisterStruct = reg;
		;}
    break;

  case 56:
#line 423 "rc1.0_grammar.y"
    {
			MappedRegisterStruct reg;
			reg.Init(yyvsp[-1].registerEnum, GL_UNSIGNED_INVERT_NV);
			yyval.mappedRegisterStruct = reg;
		;}
    break;

  case 57:
#line 431 "rc1.0_grammar.y"
    {
			FinalCombinerStruct finalCombinerStruct;
			finalCombinerStruct.Init(yyvsp[0].finalRgbFunctionStruct, yyvsp[-1].finalAlphaFunctionStruct, false);
			yyval.finalCombinerStruct = finalCombinerStruct;
		;}
    break;

  case 58:
#line 437 "rc1.0_grammar.y"
    {
			FinalCombinerStruct finalCombinerStruct;
			finalCombinerStruct.Init(yyvsp[0].finalRgbFunctionStruct, yyvsp[-1].finalAlphaFunctionStruct, true);
			yyval.finalCombinerStruct = finalCombinerStruct;
		;}
    break;

  case 59:
#line 443 "rc1.0_grammar.y"
    {
			FinalCombinerStruct finalCombinerStruct;
			finalCombinerStruct.Init(yyvsp[0].finalRgbFunctionStruct, yyvsp[-1].finalAlphaFunctionStruct, false, yyvsp[-2].finalProductStruct);
			yyval.finalCombinerStruct = finalCombinerStruct;
		;}
    break;

  case 60:
#line 449 "rc1.0_grammar.y"
    {
			FinalCombinerStruct finalCombinerStruct;
			finalCombinerStruct.Init(yyvsp[0].finalRgbFunctionStruct, yyvsp[-1].finalAlphaFunctionStruct, true, yyvsp[-2].finalProductStruct);
			yyval.finalCombinerStruct = finalCombinerStruct;
		;}
    break;

  case 61:
#line 455 "rc1.0_grammar.y"
    {
			FinalCombinerStruct finalCombinerStruct;
			finalCombinerStruct.Init(yyvsp[0].finalRgbFunctionStruct, yyvsp[-1].finalAlphaFunctionStruct, true, yyvsp[-3].finalProductStruct);
			yyval.finalCombinerStruct = finalCombinerStruct;
		;}
    break;

  case 62:
#line 462 "rc1.0_grammar.y"
    {
			FinalCombinerStruct finalCombinerStruct;
			finalCombinerStruct.Init(yyvsp[-1].finalRgbFunctionStruct, yyvsp[0].finalAlphaFunctionStruct, false);
			yyval.finalCombinerStruct = finalCombinerStruct;
		;}
    break;

  case 63:
#line 468 "rc1.0_grammar.y"
    {
			FinalCombinerStruct finalCombinerStruct;
			finalCombinerStruct.Init(yyvsp[-1].finalRgbFunctionStruct, yyvsp[0].finalAlphaFunctionStruct, true);
			yyval.finalCombinerStruct = finalCombinerStruct;
		;}
    break;

  case 64:
#line 474 "rc1.0_grammar.y"
    {
			FinalCombinerStruct finalCombinerStruct;
			finalCombinerStruct.Init(yyvsp[-1].finalRgbFunctionStruct, yyvsp[0].finalAlphaFunctionStruct, false, yyvsp[-2].finalProductStruct);
			yyval.finalCombinerStruct = finalCombinerStruct;
		;}
    break;

  case 65:
#line 480 "rc1.0_grammar.y"
    {
			FinalCombinerStruct finalCombinerStruct;
			finalCombinerStruct.Init(yyvsp[-1].finalRgbFunctionStruct, yyvsp[0].finalAlphaFunctionStruct, true, yyvsp[-2].finalProductStruct);
			yyval.finalCombinerStruct = finalCombinerStruct;
		;}
    break;

  case 66:
#line 486 "rc1.0_grammar.y"
    {
			FinalCombinerStruct finalCombinerStruct;
			finalCombinerStruct.Init(yyvsp[-1].finalRgbFunctionStruct, yyvsp[0].finalAlphaFunctionStruct, true, yyvsp[-3].finalProductStruct);
			yyval.finalCombinerStruct = finalCombinerStruct;
		;}
    break;

  case 67:
#line 493 "rc1.0_grammar.y"
    {
			FinalCombinerStruct finalCombinerStruct;
			FinalAlphaFunctionStruct finalAlphaFunctionStruct;
			finalAlphaFunctionStruct.ZeroOut();
			finalCombinerStruct.Init(yyvsp[0].finalRgbFunctionStruct, finalAlphaFunctionStruct, false);
			yyval.finalCombinerStruct = finalCombinerStruct;
		;}
    break;

  case 68:
#line 501 "rc1.0_grammar.y"
    {
			FinalCombinerStruct finalCombinerStruct;
			FinalAlphaFunctionStruct finalAlphaFunctionStruct;
			finalAlphaFunctionStruct.ZeroOut();
			finalCombinerStruct.Init(yyvsp[0].finalRgbFunctionStruct, finalAlphaFunctionStruct, true);
			yyval.finalCombinerStruct = finalCombinerStruct;
		;}
    break;

  case 69:
#line 509 "rc1.0_grammar.y"
    {
			FinalCombinerStruct finalCombinerStruct;
			FinalAlphaFunctionStruct finalAlphaFunctionStruct;
			finalAlphaFunctionStruct.ZeroOut();
			finalCombinerStruct.Init(yyvsp[0].finalRgbFunctionStruct, finalAlphaFunctionStruct, false, yyvsp[-1].finalProductStruct);
			yyval.finalCombinerStruct = finalCombinerStruct;
		;}
    break;

  case 70:
#line 517 "rc1.0_grammar.y"
    {
			FinalCombinerStruct finalCombinerStruct;
			FinalAlphaFunctionStruct finalAlphaFunctionStruct;
			finalAlphaFunctionStruct.ZeroOut();
			finalCombinerStruct.Init(yyvsp[0].finalRgbFunctionStruct, finalAlphaFunctionStruct, true, yyvsp[-1].finalProductStruct);
			yyval.finalCombinerStruct = finalCombinerStruct;
		;}
    break;

  case 71:
#line 525 "rc1.0_grammar.y"
    {
			FinalCombinerStruct finalCombinerStruct;
			FinalAlphaFunctionStruct finalAlphaFunctionStruct;
			finalAlphaFunctionStruct.ZeroOut();
			finalCombinerStruct.Init(yyvsp[0].finalRgbFunctionStruct, finalAlphaFunctionStruct, true, yyvsp[-2].finalProductStruct);
			yyval.finalCombinerStruct = finalCombinerStruct;
		;}
    break;

  case 72:
#line 534 "rc1.0_grammar.y"
    {
			FinalCombinerStruct finalCombinerStruct;
			FinalRgbFunctionStruct finalRgbFunctionStruct;
			finalRgbFunctionStruct.ZeroOut();
			finalCombinerStruct.Init(finalRgbFunctionStruct, yyvsp[0].finalAlphaFunctionStruct, false);
			yyval.finalCombinerStruct = finalCombinerStruct;
		;}
    break;

  case 73:
#line 542 "rc1.0_grammar.y"
    {
			FinalCombinerStruct finalCombinerStruct;
			FinalRgbFunctionStruct finalRgbFunctionStruct;
			finalRgbFunctionStruct.ZeroOut();
			finalCombinerStruct.Init(finalRgbFunctionStruct, yyvsp[0].finalAlphaFunctionStruct, true);
			yyval.finalCombinerStruct = finalCombinerStruct;
		;}
    break;

  case 74:
#line 550 "rc1.0_grammar.y"
    {
			FinalCombinerStruct finalCombinerStruct;
			FinalRgbFunctionStruct finalRgbFunctionStruct;
			finalRgbFunctionStruct.ZeroOut();
			finalCombinerStruct.Init(finalRgbFunctionStruct, yyvsp[0].finalAlphaFunctionStruct, false, yyvsp[-1].finalProductStruct);
			yyval.finalCombinerStruct = finalCombinerStruct;
		;}
    break;

  case 75:
#line 558 "rc1.0_grammar.y"
    {
			FinalCombinerStruct finalCombinerStruct;
			FinalRgbFunctionStruct finalRgbFunctionStruct;
			finalRgbFunctionStruct.ZeroOut();
			finalCombinerStruct.Init(finalRgbFunctionStruct, yyvsp[0].finalAlphaFunctionStruct, true, yyvsp[-1].finalProductStruct);
			yyval.finalCombinerStruct = finalCombinerStruct;
		;}
    break;

  case 76:
#line 566 "rc1.0_grammar.y"
    {
			FinalCombinerStruct finalCombinerStruct;
			FinalRgbFunctionStruct finalRgbFunctionStruct;
			finalRgbFunctionStruct.ZeroOut();
			finalCombinerStruct.Init(finalRgbFunctionStruct, yyvsp[0].finalAlphaFunctionStruct, true, yyvsp[-2].finalProductStruct);
			yyval.finalCombinerStruct = finalCombinerStruct;
		;}
    break;

  case 77:
#line 576 "rc1.0_grammar.y"
    {
			yyval.ival = yyvsp[-3].ival;
		;}
    break;

  case 78:
#line 582 "rc1.0_grammar.y"
    {
			FinalProductStruct finalProductStruct;
			finalProductStruct.Init(yyvsp[-3].mappedRegisterStruct, yyvsp[-1].mappedRegisterStruct);
			yyval.finalProductStruct = finalProductStruct;
		;}
    break;

  case 79:
#line 590 "rc1.0_grammar.y"
    {
			FinalRgbFunctionStruct finalRgbFunctionStruct;
			finalRgbFunctionStruct.Init(yyvsp[-8].mappedRegisterStruct, yyvsp[-6].mappedRegisterStruct, yyvsp[-4].mappedRegisterStruct, yyvsp[-1].mappedRegisterStruct);
			yyval.finalRgbFunctionStruct = finalRgbFunctionStruct;
		;}
    break;

  case 80:
#line 596 "rc1.0_grammar.y"
    {
			FinalRgbFunctionStruct finalRgbFunctionStruct;
			finalRgbFunctionStruct.Init(yyvsp[-6].mappedRegisterStruct, yyvsp[-4].mappedRegisterStruct, yyvsp[-2].mappedRegisterStruct, yyvsp[-10].mappedRegisterStruct);
			yyval.finalRgbFunctionStruct = finalRgbFunctionStruct;
		;}
    break;

  case 81:
#line 602 "rc1.0_grammar.y"
    {
			RegisterEnum zero;
			zero.word = RCP_ZERO;
			MappedRegisterStruct reg;
			reg.Init(zero, GL_UNSIGNED_IDENTITY_NV);
			FinalRgbFunctionStruct finalRgbFunctionStruct;
			finalRgbFunctionStruct.Init(yyvsp[-6].mappedRegisterStruct, yyvsp[-4].mappedRegisterStruct, yyvsp[-2].mappedRegisterStruct, reg);
			yyval.finalRgbFunctionStruct = finalRgbFunctionStruct;
		;}
    break;

  case 82:
#line 612 "rc1.0_grammar.y"
    {
			RegisterEnum zero;
			zero.word = RCP_ZERO;
			MappedRegisterStruct reg1;
			reg1.Init(zero, GL_UNSIGNED_IDENTITY_NV);
			MappedRegisterStruct reg2;
			reg2.Init(zero, GL_UNSIGNED_IDENTITY_NV);
			FinalRgbFunctionStruct finalRgbFunctionStruct;
			finalRgbFunctionStruct.Init(yyvsp[-3].mappedRegisterStruct, yyvsp[-1].mappedRegisterStruct, reg1, reg2);
			yyval.finalRgbFunctionStruct = finalRgbFunctionStruct;
		;}
    break;

  case 83:
#line 624 "rc1.0_grammar.y"
    {
			RegisterEnum zero;
			zero.word = RCP_ZERO;
			MappedRegisterStruct reg1;
			reg1.Init(zero, GL_UNSIGNED_IDENTITY_NV);
			FinalRgbFunctionStruct finalRgbFunctionStruct;
			finalRgbFunctionStruct.Init(yyvsp[-5].mappedRegisterStruct, yyvsp[-3].mappedRegisterStruct, reg1, yyvsp[-1].mappedRegisterStruct);
			yyval.finalRgbFunctionStruct = finalRgbFunctionStruct;
		;}
    break;

  case 84:
#line 634 "rc1.0_grammar.y"
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
			finalRgbFunctionStruct.Init(reg1, reg2, reg3, yyvsp[-1].mappedRegisterStruct);
			yyval.finalRgbFunctionStruct = finalRgbFunctionStruct;
		;}
    break;

  case 85:
#line 648 "rc1.0_grammar.y"
    {
			RegisterEnum zero;
			zero.word = RCP_ZERO;
			MappedRegisterStruct reg2;
			reg2.Init(zero, GL_UNSIGNED_INVERT_NV);
			MappedRegisterStruct reg3;
			reg3.Init(zero, GL_UNSIGNED_IDENTITY_NV);
			FinalRgbFunctionStruct finalRgbFunctionStruct;
			finalRgbFunctionStruct.Init(yyvsp[-3].mappedRegisterStruct, reg2, reg3, yyvsp[-1].mappedRegisterStruct);
			yyval.finalRgbFunctionStruct = finalRgbFunctionStruct;
		;}
    break;

  case 86:
#line 662 "rc1.0_grammar.y"
    {
			FinalAlphaFunctionStruct finalAlphaFunctionStruct;
			finalAlphaFunctionStruct.Init(yyvsp[-1].mappedRegisterStruct);
			yyval.finalAlphaFunctionStruct = finalAlphaFunctionStruct;
		;}
    break;

  case 87:
#line 670 "rc1.0_grammar.y"
    {
			yyval.registerEnum = yyvsp[0].registerEnum;
		;}
    break;

  case 88:
#line 674 "rc1.0_grammar.y"
    {
			yyval.registerEnum = yyvsp[0].registerEnum;
		;}
    break;


    }

/* Line 999 of yacc.c.  */
#line 2093 "_rc1.0_parser.c"

  yyvsp -= yylen;
  yyssp -= yylen;


  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (YYPACT_NINF < yyn && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  int yytype = YYTRANSLATE (yychar);
	  char *yymsg;
	  int yyx, yycount;

	  yycount = 0;
	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  for (yyx = yyn < 0 ? -yyn : 0;
	       yyx < (int) (sizeof (yytname) / sizeof (char *)); yyx++)
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	      yysize += yystrlen (yytname[yyx]) + 15, yycount++;
	  yysize += yystrlen ("syntax error, unexpected ") + 1;
	  yysize += yystrlen (yytname[yytype]);
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "syntax error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[yytype]);

	      if (yycount < 5)
		{
		  yycount = 0;
		  for (yyx = yyn < 0 ? -yyn : 0;
		       yyx < (int) (sizeof (yytname) / sizeof (char *));
		       yyx++)
		    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
		      {
			const char *yyq = ! yycount ? ", expecting " : " or ";
			yyp = yystpcpy (yyp, yyq);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yycount++;
		      }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("syntax error; also virtual memory exhausted");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror ("syntax error");
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      /* Return failure if at end of input.  */
      if (yychar == YYEOF)
        {
	  /* Pop the error token.  */
          YYPOPSTACK;
	  /* Pop the rest of the stack.  */
	  while (yyss < yyssp)
	    {
	      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
	      yydestruct (yystos[*yyssp], yyvsp);
	      YYPOPSTACK;
	    }
	  YYABORT;
        }

      YYDSYMPRINTF ("Error: discarding", yytoken, &yylval, &yylloc);
      yydestruct (yytoken, &yylval);
      yychar = YYEMPTY;

    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*----------------------------------------------------.
| yyerrlab1 -- error raised explicitly by an action.  |
`----------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
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

      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
      yydestruct (yystos[yystate], yyvsp);
      yyvsp--;
      yystate = *--yyssp;

      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;


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

#ifndef yyoverflow
/*----------------------------------------------.
| yyoverflowlab -- parser overflow comes here.  |
`----------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}


#line 679 "rc1.0_grammar.y"

void yyerror(char* s)
{
     errors.set("unrecognized token");
}

