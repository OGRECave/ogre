/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
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
#define yyparse vs10_parse
#define yylex   vs10_lex
#define yyerror vs10_error
#define yylval  vs10_lval
#define yychar  vs10_char
#define yydebug vs10_debug
#define yynerrs vs10_nerrs


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     VERTEX_SHADER = 258,
     ADD_INSTR = 259,
     DP3_INSTR = 260,
     DP4_INSTR = 261,
     DST_INSTR = 262,
     EXP_INSTR = 263,
     EXPP_INSTR = 264,
     FRC_INSTR = 265,
     LIT_INSTR = 266,
     LOG_INSTR = 267,
     LOGP_INSTR = 268,
     M3X2_INSTR = 269,
     M3X3_INSTR = 270,
     M3X4_INSTR = 271,
     M4X3_INSTR = 272,
     M4X4_INSTR = 273,
     MAD_INSTR = 274,
     MAX_INSTR = 275,
     MIN_INSTR = 276,
     MOV_INSTR = 277,
     MUL_INSTR = 278,
     NOP_INSTR = 279,
     RCP_INSTR = 280,
     RSQ_INSTR = 281,
     SGE_INSTR = 282,
     SLT_INSTR = 283,
     SUB_INSTR = 284,
     ILLEGAL = 285,
     UNKNOWN_STRING = 286,
     INTVAL = 287,
     REGISTER = 288,
     XYZW_MODIFIER = 289,
     COMMENT = 290
   };
#endif
#define VERTEX_SHADER 258
#define ADD_INSTR 259
#define DP3_INSTR 260
#define DP4_INSTR 261
#define DST_INSTR 262
#define EXP_INSTR 263
#define EXPP_INSTR 264
#define FRC_INSTR 265
#define LIT_INSTR 266
#define LOG_INSTR 267
#define LOGP_INSTR 268
#define M3X2_INSTR 269
#define M3X3_INSTR 270
#define M3X4_INSTR 271
#define M4X3_INSTR 272
#define M4X4_INSTR 273
#define MAD_INSTR 274
#define MAX_INSTR 275
#define MIN_INSTR 276
#define MOV_INSTR 277
#define MUL_INSTR 278
#define NOP_INSTR 279
#define RCP_INSTR 280
#define RSQ_INSTR 281
#define SGE_INSTR 282
#define SLT_INSTR 283
#define SUB_INSTR 284
#define ILLEGAL 285
#define UNKNOWN_STRING 286
#define INTVAL 287
#define REGISTER 288
#define XYZW_MODIFIER 289
#define COMMENT 290




/* Copy the first part of user declarations.  */
#line 3 "vs1.0_grammar.y"

void yyerror(const char *s);
int yylex(void);

#include <math.h>
#include <string>

#include <stdlib.h>
#include "vs1.0_inst_list.h"
#include "nvparse_errors.h"
#include "nvparse_externs.h"

//extern bool gbTempInsideMacro;
//extern unsigned int &base_linenumber;
void LexError(const char *format, ...);
extern int line_incr;

#define do_linenum_incr()		{ line_number+=line_incr; line_incr = 0; }
//int get_linenum()			{ return( gbTempInsideMacro ? base_linenumber : line_number ); }
int get_linenum()			{ return( line_number ); }

#define YYDEBUG 1



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
#line 27 "vs1.0_grammar.y"
typedef union YYSTYPE {
  int ival;
  unsigned int lval;
  float fval;
  char mask[4];
  char *comment;
  VS10Reg reg;
  VS10InstPtr inst;
  VS10InstListPtr instList;
} YYSTYPE;
/* Line 191 of yacc.c.  */
#line 190 "_vs1.0_parser.c"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 214 of yacc.c.  */
#line 202 "_vs1.0_parser.c"

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
#define YYFINAL  44
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   88

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  44
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  18
/* YYNRULES -- Number of rules. */
#define YYNRULES  59
/* YYNRULES -- Number of states. */
#define YYNSTATES  103

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   290

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      36,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    43,    37,    38,    39,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    41,     2,    42,     2,     2,     2,     2,     2,    40,
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
static const unsigned char yyprhs[] =
{
       0,     0,     3,     5,     8,    10,    12,    14,    16,    18,
      20,    22,    24,    26,    28,    30,    35,    40,    44,    47,
      49,    51,    53,    58,    65,    74,    79,    86,    92,   100,
     108,   118,   123,   128,   135,   144,   146,   148,   150,   152,
     154,   156,   158,   160,   162,   164,   166,   168,   170,   172,
     174,   176,   178,   180,   182,   184,   186,   188,   190,   192
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      45,     0,    -1,    46,    -1,    46,    47,    -1,    47,    -1,
      48,    -1,    36,    -1,    49,    -1,    53,    -1,    54,    -1,
      55,    -1,    56,    -1,    24,    -1,    35,    -1,     3,    -1,
      57,    50,    37,    50,    -1,    38,    51,    39,    34,    -1,
      51,    39,    34,    -1,    38,    51,    -1,    51,    -1,    33,
      -1,    52,    -1,    40,    41,    32,    42,    -1,    40,    41,
      33,    39,    34,    42,    -1,    40,    41,    33,    39,    34,
      43,    32,    42,    -1,    40,    41,    33,    42,    -1,    40,
      41,    33,    43,    32,    42,    -1,    40,    41,    38,    33,
      42,    -1,    40,    41,    38,    33,    43,    32,    42,    -1,
      40,    41,    38,    33,    39,    34,    42,    -1,    40,    41,
      38,    33,    39,    34,    43,    32,    42,    -1,    58,    50,
      37,    50,    -1,    59,    50,    37,    50,    -1,    60,    50,
      37,    50,    37,    50,    -1,    61,    50,    37,    50,    37,
      50,    37,    50,    -1,    22,    -1,    11,    -1,    25,    -1,
      26,    -1,     8,    -1,     9,    -1,    12,    -1,    13,    -1,
      10,    -1,    23,    -1,     4,    -1,     5,    -1,     6,    -1,
       7,    -1,    21,    -1,    20,    -1,    28,    -1,    27,    -1,
      14,    -1,    15,    -1,    16,    -1,    17,    -1,    18,    -1,
      29,    -1,    19,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,    96,    96,   104,   110,   122,   127,   134,   135,   136,
     137,   138,   139,   143,   147,   153,   159,   169,   179,   189,
     200,   200,   204,   211,   246,   282,   287,   292,   297,   302,
     307,   314,   320,   326,   332,   339,   343,   349,   353,   357,
     361,   365,   369,   375,   381,   385,   389,   393,   397,   401,
     405,   409,   413,   417,   421,   425,   429,   433,   437,   443
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "VERTEX_SHADER", "ADD_INSTR", "DP3_INSTR", 
  "DP4_INSTR", "DST_INSTR", "EXP_INSTR", "EXPP_INSTR", "FRC_INSTR", 
  "LIT_INSTR", "LOG_INSTR", "LOGP_INSTR", "M3X2_INSTR", "M3X3_INSTR", 
  "M3X4_INSTR", "M4X3_INSTR", "M4X4_INSTR", "MAD_INSTR", "MAX_INSTR", 
  "MIN_INSTR", "MOV_INSTR", "MUL_INSTR", "NOP_INSTR", "RCP_INSTR", 
  "RSQ_INSTR", "SGE_INSTR", "SLT_INSTR", "SUB_INSTR", "ILLEGAL", 
  "UNKNOWN_STRING", "INTVAL", "REGISTER", "XYZW_MODIFIER", "COMMENT", 
  "'\\n'", "','", "'-'", "'.'", "'c'", "'['", "']'", "'+'", "$accept", 
  "VS10Program", "InstSequence", "InstLine", "Instruction", 
  "VECTORopinstruction", "genericReg", "genReg", "constantReg", 
  "SCALARopinstruction", "UNARYopinstruction", "BINopinstruction", 
  "TRIopinstruction", "VECTORop", "SCALARop", "UNARYop", "BINop", "TRIop", 0
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
     285,   286,   287,   288,   289,   290,    10,    44,    45,    46,
      99,    91,    93,    43
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    44,    45,    46,    46,    47,    47,    48,    48,    48,
      48,    48,    48,    48,    48,    49,    50,    50,    50,    50,
      51,    51,    52,    52,    52,    52,    52,    52,    52,    52,
      52,    53,    54,    55,    56,    57,    57,    58,    58,    58,
      58,    58,    58,    59,    60,    60,    60,    60,    60,    60,
      60,    60,    60,    60,    60,    60,    60,    60,    60,    61
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     4,     4,     3,     2,     1,
       1,     1,     4,     6,     8,     4,     6,     5,     7,     7,
       9,     4,     4,     6,     8,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       0,    14,    45,    46,    47,    48,    39,    40,    43,    36,
      41,    42,    53,    54,    55,    56,    57,    59,    50,    49,
      35,    44,    12,    37,    38,    52,    51,    58,    13,     6,
       0,     2,     4,     5,     7,     8,     9,    10,    11,     0,
       0,     0,     0,     0,     1,     3,    20,     0,     0,     0,
      19,    21,     0,     0,     0,     0,    18,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    15,    17,
      31,    32,     0,     0,    16,    22,     0,    25,     0,     0,
       0,     0,     0,     0,     0,    27,     0,    33,     0,    23,
       0,    26,     0,     0,     0,     0,    29,     0,    28,    34,
      24,     0,    30
};

/* YYDEFGOTO[NTERM-NUM]. */
static const yysigned_char yydefgoto[] =
{
      -1,    30,    31,    32,    33,    34,    49,    50,    51,    35,
      36,    37,    38,    39,    40,    41,    42,    43
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -41
static const yysigned_char yypact[] =
{
      52,   -41,   -41,   -41,   -41,   -41,   -41,   -41,   -41,   -41,
     -41,   -41,   -41,   -41,   -41,   -41,   -41,   -41,   -41,   -41,
     -41,   -41,   -41,   -41,   -41,   -41,   -41,   -41,   -41,   -41,
       6,    52,   -41,   -41,   -41,   -41,   -41,   -41,   -41,   -29,
     -29,   -29,   -29,   -29,   -41,   -41,   -41,   -28,   -31,   -23,
     -22,   -41,   -18,   -12,    -4,    -3,    -2,   -25,   -29,     1,
     -29,   -29,   -29,   -29,     2,     0,   -15,     5,   -41,   -41,
     -41,   -41,     7,     8,   -41,   -41,     9,   -41,    14,   -13,
     -29,   -29,   -27,    10,    13,   -41,    16,   -41,    12,   -41,
      18,   -41,   -11,    11,   -29,    40,   -41,    19,   -41,   -41,
     -41,    41,   -41
};

/* YYPGOTO[NTERM-NUM].  */
static const yysigned_char yypgoto[] =
{
     -41,   -41,   -41,    53,   -41,   -41,   -40,    -8,   -41,   -41,
     -41,   -41,   -41,   -41,   -41,   -41,   -41,   -41
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const unsigned char yytable[] =
{
      52,    53,    54,    55,    46,    46,    44,    65,    66,    47,
      57,    48,    48,    67,    58,    89,    90,    59,    68,    60,
      70,    71,    72,    73,    76,    61,    84,    77,    78,    85,
      86,    96,    97,    62,    63,    69,    74,    64,    79,    56,
      87,    88,    75,    82,    80,    81,    83,    92,    93,    94,
      95,   101,    91,    98,    99,     1,     2,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,   100,   102,    45,     0,     0,    28,    29
};

static const yysigned_char yycheck[] =
{
      40,    41,    42,    43,    33,    33,     0,    32,    33,    38,
      41,    40,    40,    38,    37,    42,    43,    39,    58,    37,
      60,    61,    62,    63,    39,    37,    39,    42,    43,    42,
      43,    42,    43,    37,    37,    34,    34,    39,    33,    47,
      80,    81,    42,    34,    37,    37,    32,    34,    32,    37,
      32,    32,    42,    42,    94,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    42,    42,    31,    -1,    -1,    35,    36
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    35,    36,
      45,    46,    47,    48,    49,    53,    54,    55,    56,    57,
      58,    59,    60,    61,     0,    47,    33,    38,    40,    50,
      51,    52,    50,    50,    50,    50,    51,    41,    37,    39,
      37,    37,    37,    37,    39,    32,    33,    38,    50,    34,
      50,    50,    50,    50,    34,    42,    39,    42,    43,    33,
      37,    37,    34,    32,    39,    42,    43,    50,    50,    42,
      43,    42,    34,    32,    37,    32,    42,    43,    42,    50,
      42,    32,    42
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
#line 97 "vs1.0_grammar.y"
    {
	yyvsp[0].instList->Validate();
	yyvsp[0].instList->Translate();
	delete yyvsp[0].instList;
	;}
    break;

  case 3:
#line 105 "vs1.0_grammar.y"
    {
		*(yyvsp[-1].instList) += yyvsp[0].inst;
		delete yyvsp[0].inst;
		yyval.instList = yyvsp[-1].instList
	;}
    break;

  case 4:
#line 111 "vs1.0_grammar.y"
    {
 		VS10InstListPtr instList = new VS10InstList;
		if ( yyvsp[0].inst != NULL )
			{
			*instList += yyvsp[0].inst;
			delete yyvsp[0].inst;
			}
		yyval.instList = instList;
	;}
    break;

  case 5:
#line 123 "vs1.0_grammar.y"
    {
		yyval.inst = yyvsp[0].inst;
		do_linenum_incr();
	;}
    break;

  case 6:
#line 128 "vs1.0_grammar.y"
    {
		yyval.inst = new VS10Inst( get_linenum() );
		do_linenum_incr();
	;}
    break;

  case 12:
#line 140 "vs1.0_grammar.y"
    {
		   yyval.inst = new VS10Inst( get_linenum(), VS10_NOP );
		   ;}
    break;

  case 13:
#line 144 "vs1.0_grammar.y"
    {
		   yyval.inst = new VS10Inst( get_linenum(), VS10_COMMENT, yyvsp[0].comment );
		   ;}
    break;

  case 14:
#line 148 "vs1.0_grammar.y"
    {
		   yyval.inst = new VS10Inst( get_linenum(), VS10_HEADER );
		   ;}
    break;

  case 15:
#line 154 "vs1.0_grammar.y"
    {
		yyval.inst = new VS10Inst( get_linenum(), yyvsp[-3].ival, yyvsp[-2].reg, yyvsp[0].reg );
	;}
    break;

  case 16:
#line 160 "vs1.0_grammar.y"
    {
		   VS10Reg reg;
		   reg = yyvsp[-2].reg;
		   reg.sign = -1;
		   reg.type = yyvsp[-2].reg.type;
		   reg.index = yyvsp[-2].reg.index;
		   for ( int i = 0; i < 4; i++ ) reg.mask[i] = yyvsp[0].mask[i];
		   yyval.reg = reg;
		   ;}
    break;

  case 17:
#line 170 "vs1.0_grammar.y"
    {
		   VS10Reg reg;
		   reg = yyvsp[-2].reg;
		   reg.sign = 1;
		   reg.type = yyvsp[-2].reg.type;
		   reg.index = yyvsp[-2].reg.index;
		   for ( int i = 0; i < 4; i++ ) reg.mask[i] = yyvsp[0].mask[i];
		   yyval.reg = reg;
		   ;}
    break;

  case 18:
#line 180 "vs1.0_grammar.y"
    {
		   VS10Reg reg;
		   reg = yyvsp[0].reg;
		   reg.sign = -1;
		   reg.type = yyvsp[0].reg.type;
		   reg.index = yyvsp[0].reg.index;
		   for ( int i = 0; i < 4; i++ ) reg.mask[i] = 0;
		   yyval.reg = reg;
		   ;}
    break;

  case 19:
#line 190 "vs1.0_grammar.y"
    {
		   VS10Reg reg;
		   reg = yyvsp[0].reg;
		   reg.sign = 1;
		   reg.type = yyvsp[0].reg.type;
		   reg.index = yyvsp[0].reg.index;
		   for ( int i = 0; i < 4; i++ ) reg.mask[i] = 0;
		   yyval.reg = reg;
		   ;}
    break;

  case 21:
#line 201 "vs1.0_grammar.y"
    {
	  ;}
    break;

  case 22:
#line 205 "vs1.0_grammar.y"
    {
		   VS10Reg reg;
		   reg.type = TYPE_CONSTANT_MEM_REG;
		   reg.index = yyvsp[-1].ival;
		   yyval.reg = reg;
		   ;}
    break;

  case 23:
#line 212 "vs1.0_grammar.y"
    {
		   // Register is valid only if
		   //	type = TYPE_ADDRESS_REG
		   //	index = 0
		   //	len(mask) = 1
		   //	mask[0] = 'x'
		   VS10Reg reg;
		   yyval.reg.type = TYPE_CONSTANT_A0_REG;
		   if ( yyvsp[-3].reg.type != TYPE_ADDRESS_REG )
		       {
			   LexError( "constant register index must be:\t<int>, a0.x, or a0.x + <int>.\n" );
			   }
		       else if ( yyvsp[-3].reg.index != 0 )
			   {
			   LexError( "constant register index must be:\t<int>, a0.x, or a0.x + <int>.\n" );
			   }
			   else
			   {
			       int len = 0;
				   while ( len < 2 )
				   {
				       if ( yyvsp[-1].mask[len] == 0 )
				           break;
					   len++;
				   }
				   if ( len != 1 || yyvsp[-1].mask[0] != 'x' )
				   {
			       LexError( "constant register index must be:\t<int>, a0.x, or a0.x + <int>.\n" );
				   }

				   reg.type = TYPE_CONSTANT_A0_REG;
				   yyval.reg = reg;
			   }
		   ;}
    break;

  case 24:
#line 247 "vs1.0_grammar.y"
    {
		   // Register is valid only if
		   //	type = TYPE_ADDRESS_REG
		   //	index = 0
		   //	len(mask) = 1
		   //	mask[0] = 'x'
		   VS10Reg reg;
		   yyval.reg.type = TYPE_CONSTANT_A0_OFFSET_REG;
		   if ( yyvsp[-5].reg.type != TYPE_ADDRESS_REG )
		       {
			   LexError( "constant register index must be:\t<int>, a0.x, or a0.x + <int>.\n" );
			   }
		       else if ( yyvsp[-5].reg.index != 0 )
			   {
			   LexError( "constant register index must be:\t<int>, a0.x, or a0.x + <int>.\n" );
			   }
			   else
			   {
			       int len = 0;
				   while ( len < 2 )
				   {
				       if ( yyvsp[-3].mask[len] == 0 )
				           break;
					   len++;
				   }
				   if ( len != 1 || yyvsp[-3].mask[0] != 'x' )
				   {
			       LexError( "constant register index must be:\t<int>, a0.x, or a0.x + <int>.\n" );
				   }

				   reg.type = TYPE_CONSTANT_A0_OFFSET_REG;
				   reg.index = yyvsp[-1].ival;
				   yyval.reg = reg;
			   }
		   ;}
    break;

  case 25:
#line 283 "vs1.0_grammar.y"
    {
		       yyval.reg.type = TYPE_CONSTANT_A0_REG;
			   LexError( "constant register index must be:\t<int>, a0.x, or a0.x + <int>.\n" );
		   ;}
    break;

  case 26:
#line 288 "vs1.0_grammar.y"
    {
		       yyval.reg.type = TYPE_CONSTANT_A0_REG;
			   LexError( "constant register index must be:\t<int>, a0.x, or a0.x + <int>.\n" );
		   ;}
    break;

  case 27:
#line 293 "vs1.0_grammar.y"
    {
		       yyval.reg.type = TYPE_CONSTANT_A0_REG;
			   LexError( "constant register index must be:\t<int>, a0.x, or a0.x + <int>.\n" );
		   ;}
    break;

  case 28:
#line 298 "vs1.0_grammar.y"
    {
		       yyval.reg.type = TYPE_CONSTANT_A0_REG;
			   LexError( "constant register index must be:\t<int>, a0.x, or a0.x + <int>.\n" );
		   ;}
    break;

  case 29:
#line 303 "vs1.0_grammar.y"
    {
		       yyval.reg.type = TYPE_CONSTANT_A0_REG;
			   LexError( "constant register index must be:\t<int>, a0.x, or a0.x + <int>.\n" );
		   ;}
    break;

  case 30:
#line 308 "vs1.0_grammar.y"
    {
		       yyval.reg.type = TYPE_CONSTANT_A0_REG;
			   LexError( "constant register index must be:\t<int>, a0.x, or a0.x + <int>.\n" );
		   ;}
    break;

  case 31:
#line 315 "vs1.0_grammar.y"
    {
		yyval.inst = new VS10Inst( get_linenum(), yyvsp[-3].ival, yyvsp[-2].reg, yyvsp[0].reg );
	;}
    break;

  case 32:
#line 321 "vs1.0_grammar.y"
    {
		yyval.inst = new VS10Inst( get_linenum(), yyvsp[-3].ival, yyvsp[-2].reg, yyvsp[0].reg );
	;}
    break;

  case 33:
#line 327 "vs1.0_grammar.y"
    {
		yyval.inst = new VS10Inst( get_linenum(), yyvsp[-5].ival, yyvsp[-4].reg, yyvsp[-2].reg, yyvsp[0].reg );
	;}
    break;

  case 34:
#line 334 "vs1.0_grammar.y"
    {
		yyval.inst = new VS10Inst( get_linenum(), yyvsp[-7].ival, yyvsp[-6].reg, yyvsp[-4].reg, yyvsp[-2].reg, yyvsp[0].reg );
	;}
    break;

  case 35:
#line 340 "vs1.0_grammar.y"
    {
		yyval.ival = VS10_MOV;
	;}
    break;

  case 36:
#line 344 "vs1.0_grammar.y"
    {
		yyval.ival = VS10_LIT;
	;}
    break;

  case 37:
#line 350 "vs1.0_grammar.y"
    {
	yyval.ival = VS10_RCP;
	;}
    break;

  case 38:
#line 354 "vs1.0_grammar.y"
    {
	yyval.ival = VS10_RSQ;
	;}
    break;

  case 39:
#line 358 "vs1.0_grammar.y"
    {
	yyval.ival = VS10_EXP;
	;}
    break;

  case 40:
#line 362 "vs1.0_grammar.y"
    {
	yyval.ival = VS10_EXPP;
	;}
    break;

  case 41:
#line 366 "vs1.0_grammar.y"
    {
	yyval.ival = VS10_LOG;
	;}
    break;

  case 42:
#line 370 "vs1.0_grammar.y"
    {
	yyval.ival = VS10_LOGP;
	;}
    break;

  case 43:
#line 376 "vs1.0_grammar.y"
    {
	yyval.ival = VS10_FRC;
	;}
    break;

  case 44:
#line 382 "vs1.0_grammar.y"
    {
	yyval.ival = VS10_MUL;
	;}
    break;

  case 45:
#line 386 "vs1.0_grammar.y"
    {
	yyval.ival = VS10_ADD;
	;}
    break;

  case 46:
#line 390 "vs1.0_grammar.y"
    {
	yyval.ival = VS10_DP3;
	;}
    break;

  case 47:
#line 394 "vs1.0_grammar.y"
    {
	yyval.ival = VS10_DP4;
	;}
    break;

  case 48:
#line 398 "vs1.0_grammar.y"
    {
	yyval.ival = VS10_DST;
	;}
    break;

  case 49:
#line 402 "vs1.0_grammar.y"
    {
	yyval.ival = VS10_MIN;
	;}
    break;

  case 50:
#line 406 "vs1.0_grammar.y"
    {
	yyval.ival = VS10_MAX;
	;}
    break;

  case 51:
#line 410 "vs1.0_grammar.y"
    {
	yyval.ival = VS10_SLT;
	;}
    break;

  case 52:
#line 414 "vs1.0_grammar.y"
    {
	yyval.ival = VS10_SGE;
	;}
    break;

  case 53:
#line 418 "vs1.0_grammar.y"
    {
	yyval.ival = VS10_M3X2;
	;}
    break;

  case 54:
#line 422 "vs1.0_grammar.y"
    {
	yyval.ival = VS10_M3X3;
	;}
    break;

  case 55:
#line 426 "vs1.0_grammar.y"
    {
	yyval.ival = VS10_M3X4;
	;}
    break;

  case 56:
#line 430 "vs1.0_grammar.y"
    {
	yyval.ival = VS10_M4X3;
	;}
    break;

  case 57:
#line 434 "vs1.0_grammar.y"
    {
	yyval.ival = VS10_M4X4;
	;}
    break;

  case 58:
#line 438 "vs1.0_grammar.y"
    {
	yyval.ival = VS10_SUB;
	;}
    break;

  case 59:
#line 444 "vs1.0_grammar.y"
    {
	yyval.ival = VS10_MAD;
	;}
    break;


    }

/* Line 999 of yacc.c.  */
#line 1639 "_vs1.0_parser.c"

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


#line 450 "vs1.0_grammar.y"

void yyerror(const char* s)
{
    LexError( "Syntax Error.\n" );
    //errors.set(s);
    //errors.set("unrecognized token");
}

