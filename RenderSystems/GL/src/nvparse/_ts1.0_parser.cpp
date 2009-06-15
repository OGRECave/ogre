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
#define yyparse ts10_parse
#define yylex   ts10_lex
#define yyerror ts10_error
#define yylval  ts10_lval
#define yychar  ts10_char
#define yydebug ts10_debug
#define yynerrs ts10_nerrs


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
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
#define floatValue 258
#define gequal 259
#define less 260
#define texVariable 261
#define expandString 262
#define openParen 263
#define closeParen 264
#define semicolon 265
#define comma 266
#define nop 267
#define texture_1d 268
#define texture_2d 269
#define texture_rectangle 270
#define texture_3d 271
#define texture_cube_map 272
#define cull_fragment 273
#define pass_through 274
#define offset_2d_scale 275
#define offset_2d 276
#define offset_rectangle_scale 277
#define offset_rectangle 278
#define dependent_ar 279
#define dependent_gb 280
#define dot_product_2d_1of2 281
#define dot_product_2d_2of2 282
#define dot_product_rectangle_1of2 283
#define dot_product_rectangle_2of2 284
#define dot_product_depth_replace_1of2 285
#define dot_product_depth_replace_2of2 286
#define dot_product_3d_1of3 287
#define dot_product_3d_2of3 288
#define dot_product_3d_3of3 289
#define dot_product_cube_map_1of3 290
#define dot_product_cube_map_2of3 291
#define dot_product_cube_map_3of3 292
#define dot_product_reflect_cube_map_eye_from_qs_1of3 293
#define dot_product_reflect_cube_map_eye_from_qs_2of3 294
#define dot_product_reflect_cube_map_eye_from_qs_3of3 295
#define dot_product_reflect_cube_map_const_eye_1of3 296
#define dot_product_reflect_cube_map_const_eye_2of3 297
#define dot_product_reflect_cube_map_const_eye_3of3 298
#define dot_product_cube_map_and_reflect_cube_map_eye_from_qs_1of3 299
#define dot_product_cube_map_and_reflect_cube_map_eye_from_qs_2of3 300
#define dot_product_cube_map_and_reflect_cube_map_eye_from_qs_3of3 301
#define dot_product_cube_map_and_reflect_cube_map_const_eye_1of3 302
#define dot_product_cube_map_and_reflect_cube_map_const_eye_2of3 303
#define dot_product_cube_map_and_reflect_cube_map_const_eye_3of3 304




/* Copy the first part of user declarations.  */
#line 3 "ts1.0_grammar.y"

void yyerror(const char* s);
int yylex ( void );

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  define NOMINMAX // required to stop windows.h messing up std::min
#  include <windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include "ts1.0_inst.h"
#include "ts1.0_inst_list.h"
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
#line 18 "ts1.0_grammar.y"
typedef union YYSTYPE {
  float fval;
  InstPtr inst;
  InstListPtr instList;
  MappedVariablePtr variable;
} YYSTYPE;
/* Line 191 of yacc.c.  */
#line 205 "_ts1.0_parser.c"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 214 of yacc.c.  */
#line 217 "_ts1.0_parser.c"

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
#define YYFINAL  80
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   223

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  50
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  6
/* YYNRULES -- Number of rules. */
#define YYNRULES  46
/* YYNRULES -- Number of states. */
#define YYNSTATES  218

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   304

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
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     5,     9,    12,    17,    19,    23,    27,
      31,    35,    39,    43,    54,    58,    75,    88,   105,   118,
     123,   128,   133,   138,   143,   148,   153,   158,   163,   168,
     173,   178,   183,   188,   193,   198,   203,   214,   219,   224,
     229,   234,   239,   250,   255,   260,   262
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      51,     0,    -1,    52,    -1,    52,    54,    10,    -1,    54,
      10,    -1,     7,     8,     6,     9,    -1,     6,    -1,    12,
       8,     9,    -1,    13,     8,     9,    -1,    14,     8,     9,
      -1,    15,     8,     9,    -1,    16,     8,     9,    -1,    17,
       8,     9,    -1,    18,     8,    55,    11,    55,    11,    55,
      11,    55,     9,    -1,    19,     8,     9,    -1,    20,     8,
       6,    11,     3,    11,     3,    11,     3,    11,     3,    11,
       3,    11,     3,     9,    -1,    21,     8,     6,    11,     3,
      11,     3,    11,     3,    11,     3,     9,    -1,    22,     8,
       6,    11,     3,    11,     3,    11,     3,    11,     3,    11,
       3,    11,     3,     9,    -1,    23,     8,     6,    11,     3,
      11,     3,    11,     3,    11,     3,     9,    -1,    24,     8,
       6,     9,    -1,    25,     8,     6,     9,    -1,    26,     8,
      53,     9,    -1,    27,     8,    53,     9,    -1,    28,     8,
      53,     9,    -1,    29,     8,    53,     9,    -1,    30,     8,
      53,     9,    -1,    31,     8,    53,     9,    -1,    32,     8,
      53,     9,    -1,    33,     8,    53,     9,    -1,    34,     8,
      53,     9,    -1,    35,     8,    53,     9,    -1,    36,     8,
      53,     9,    -1,    37,     8,    53,     9,    -1,    38,     8,
      53,     9,    -1,    39,     8,    53,     9,    -1,    40,     8,
      53,     9,    -1,    41,     8,    53,    11,     3,    11,     3,
      11,     3,     9,    -1,    42,     8,    53,     9,    -1,    43,
       8,    53,     9,    -1,    44,     8,    53,     9,    -1,    45,
       8,    53,     9,    -1,    46,     8,    53,     9,    -1,    47,
       8,    53,    11,     3,    11,     3,    11,     3,     9,    -1,
      48,     8,    53,     9,    -1,    49,     8,    53,     9,    -1,
       4,    -1,     5,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,    73,    73,    81,    87,    97,   103,   111,   115,   119,
     123,   127,   131,   135,   139,   143,   147,   151,   155,   159,
     163,   167,   172,   177,   182,   187,   192,   197,   202,   207,
     212,   217,   222,   227,   232,   237,   242,   247,   252,   257,
     262,   267,   272,   277,   282,   289,   293
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
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
  "CondDesc", 0
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
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    50,    51,    52,    52,    53,    53,    54,    54,    54,
      54,    54,    54,    54,    54,    54,    54,    54,    54,    54,
      54,    54,    54,    54,    54,    54,    54,    54,    54,    54,
      54,    54,    54,    54,    54,    54,    54,    54,    54,    54,
      54,    54,    54,    54,    54,    55,    55
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     3,     2,     4,     1,     3,     3,     3,
       3,     3,     3,    10,     3,    16,    12,    16,    12,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,    10,     4,     4,     4,
       4,     4,    10,     4,     4,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
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

/* YYDEFGOTO[NTERM-NUM]. */
static const yysigned_char yydefgoto[] =
{
      -1,    39,    40,   101,    41,    91
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -127
static const short yypact[] =
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

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -127,  -127,  -127,    -1,   182,  -126
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const unsigned char yytable[] =
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

static const short yycheck[] =
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
static const unsigned char yystos[] =
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
#line 74 "ts1.0_grammar.y"
    {
			yyvsp[0].instList->Validate();
			yyvsp[0].instList->Invoke();
		    delete yyvsp[0].instList;
		;}
    break;

  case 3:
#line 82 "ts1.0_grammar.y"
    {
		    *(yyvsp[-2].instList) += yyvsp[-1].inst;
			delete yyvsp[-1].inst;
		    yyval.instList = yyvsp[-2].instList;
		;}
    break;

  case 4:
#line 88 "ts1.0_grammar.y"
    {
		    InstListPtr instList = new InstList;
		    *instList += yyvsp[-1].inst;
			delete yyvsp[-1].inst;
		    yyval.instList = instList;
		;}
    break;

  case 5:
#line 98 "ts1.0_grammar.y"
    {
			yyval.variable = new MappedVariable;
			yyval.variable->var = yyvsp[-1].fval;
			yyval.variable->expand = true;
		;}
    break;

  case 6:
#line 104 "ts1.0_grammar.y"
    {
			yyval.variable = new MappedVariable;
			yyval.variable->var = yyvsp[0].fval;
			yyval.variable->expand = false;
		;}
    break;

  case 7:
#line 112 "ts1.0_grammar.y"
    {
		    yyval.inst = new Inst(TSP_NOP);
		;}
    break;

  case 8:
#line 116 "ts1.0_grammar.y"
    {
		    yyval.inst = new Inst(TSP_TEXTURE_1D);
		;}
    break;

  case 9:
#line 120 "ts1.0_grammar.y"
    {
		    yyval.inst = new Inst(TSP_TEXTURE_2D);
		;}
    break;

  case 10:
#line 124 "ts1.0_grammar.y"
    {
		    yyval.inst = new Inst(TSP_TEXTURE_RECTANGLE);
		;}
    break;

  case 11:
#line 128 "ts1.0_grammar.y"
    {
		    yyval.inst = new Inst(TSP_TEXTURE_3D);
		;}
    break;

  case 12:
#line 132 "ts1.0_grammar.y"
    {
		    yyval.inst = new Inst(TSP_TEXTURE_CUBE_MAP);
		;}
    break;

  case 13:
#line 136 "ts1.0_grammar.y"
    {
		    yyval.inst = new Inst(TSP_CULL_FRAGMENT, yyvsp[-7].fval, yyvsp[-5].fval, yyvsp[-3].fval, yyvsp[-1].fval);
		;}
    break;

  case 14:
#line 140 "ts1.0_grammar.y"
    {
		    yyval.inst = new Inst(TSP_PASS_THROUGH);
		;}
    break;

  case 15:
#line 144 "ts1.0_grammar.y"
    {
		    yyval.inst = new Inst(TSP_OFFSET_2D_SCALE, yyvsp[-13].fval, yyvsp[-11].fval, yyvsp[-9].fval, yyvsp[-7].fval, yyvsp[-5].fval, yyvsp[-3].fval, yyvsp[-1].fval);
		;}
    break;

  case 16:
#line 148 "ts1.0_grammar.y"
    {
		    yyval.inst = new Inst(TSP_OFFSET_2D, yyvsp[-9].fval, yyvsp[-7].fval, yyvsp[-5].fval, yyvsp[-3].fval, yyvsp[-1].fval);
		;}
    break;

  case 17:
#line 152 "ts1.0_grammar.y"
    {
		    yyval.inst = new Inst(TSP_OFFSET_RECTANGLE_SCALE, yyvsp[-13].fval, yyvsp[-11].fval, yyvsp[-9].fval, yyvsp[-7].fval, yyvsp[-5].fval, yyvsp[-3].fval, yyvsp[-1].fval);
		;}
    break;

  case 18:
#line 156 "ts1.0_grammar.y"
    {
		    yyval.inst = new Inst(TSP_OFFSET_RECTANGLE, yyvsp[-9].fval, yyvsp[-7].fval, yyvsp[-5].fval, yyvsp[-3].fval, yyvsp[-1].fval);
		;}
    break;

  case 19:
#line 160 "ts1.0_grammar.y"
    {
		    yyval.inst = new Inst(TSP_DEPENDENT_AR, yyvsp[-1].fval);
		;}
    break;

  case 20:
#line 164 "ts1.0_grammar.y"
    {
		    yyval.inst = new Inst(TSP_DEPENDENT_GB, yyvsp[-1].fval);
		;}
    break;

  case 21:
#line 168 "ts1.0_grammar.y"
    {
		    yyval.inst = new Inst(TSP_DOT_PRODUCT_2D_1_OF_2, yyvsp[-1].variable);
			delete yyvsp[-1].variable;
		;}
    break;

  case 22:
#line 173 "ts1.0_grammar.y"
    {
		    yyval.inst = new Inst(TSP_DOT_PRODUCT_2D_2_OF_2, yyvsp[-1].variable);
			delete yyvsp[-1].variable;
		;}
    break;

  case 23:
#line 178 "ts1.0_grammar.y"
    {
		    yyval.inst = new Inst(TSP_DOT_PRODUCT_RECTANGLE_1_OF_2, yyvsp[-1].variable);
			delete yyvsp[-1].variable;
		;}
    break;

  case 24:
#line 183 "ts1.0_grammar.y"
    {
		    yyval.inst = new Inst(TSP_DOT_PRODUCT_RECTANGLE_2_OF_2, yyvsp[-1].variable);
			delete yyvsp[-1].variable;
		;}
    break;

  case 25:
#line 188 "ts1.0_grammar.y"
    {
		    yyval.inst = new Inst(TSP_DOT_PRODUCT_DEPTH_REPLACE_1_OF_2, yyvsp[-1].variable);
			delete yyvsp[-1].variable;
		;}
    break;

  case 26:
#line 193 "ts1.0_grammar.y"
    {
		    yyval.inst = new Inst(TSP_DOT_PRODUCT_DEPTH_REPLACE_2_OF_2, yyvsp[-1].variable);
			delete yyvsp[-1].variable;
		;}
    break;

  case 27:
#line 198 "ts1.0_grammar.y"
    {
		    yyval.inst = new Inst(TSP_DOT_PRODUCT_3D_1_OF_3, yyvsp[-1].variable);
			delete yyvsp[-1].variable;
		;}
    break;

  case 28:
#line 203 "ts1.0_grammar.y"
    {
		    yyval.inst = new Inst(TSP_DOT_PRODUCT_3D_2_OF_3, yyvsp[-1].variable);
			delete yyvsp[-1].variable;
		;}
    break;

  case 29:
#line 208 "ts1.0_grammar.y"
    {
		    yyval.inst = new Inst(TSP_DOT_PRODUCT_3D_3_OF_3, yyvsp[-1].variable);
			delete yyvsp[-1].variable;
		;}
    break;

  case 30:
#line 213 "ts1.0_grammar.y"
    {
		    yyval.inst = new Inst(TSP_DOT_PRODUCT_CUBE_MAP_1_OF_3, yyvsp[-1].variable);
			delete yyvsp[-1].variable;
		;}
    break;

  case 31:
#line 218 "ts1.0_grammar.y"
    {
		    yyval.inst = new Inst(TSP_DOT_PRODUCT_CUBE_MAP_2_OF_3, yyvsp[-1].variable);
			delete yyvsp[-1].variable;
		;}
    break;

  case 32:
#line 223 "ts1.0_grammar.y"
    {
		    yyval.inst = new Inst(TSP_DOT_PRODUCT_CUBE_MAP_3_OF_3, yyvsp[-1].variable);
			delete yyvsp[-1].variable;
		;}
    break;

  case 33:
#line 228 "ts1.0_grammar.y"
    {
		    yyval.inst = new Inst(TSP_DOT_PRODUCT_REFLECT_CUBE_MAP_EYE_FROM_QS_1_OF_3, yyvsp[-1].variable);
			delete yyvsp[-1].variable;
		;}
    break;

  case 34:
#line 233 "ts1.0_grammar.y"
    {
		    yyval.inst = new Inst(TSP_DOT_PRODUCT_REFLECT_CUBE_MAP_EYE_FROM_QS_2_OF_3, yyvsp[-1].variable);
			delete yyvsp[-1].variable;
		;}
    break;

  case 35:
#line 238 "ts1.0_grammar.y"
    {
		    yyval.inst = new Inst(TSP_DOT_PRODUCT_REFLECT_CUBE_MAP_EYE_FROM_QS_3_OF_3, yyvsp[-1].variable);
			delete yyvsp[-1].variable;
		;}
    break;

  case 36:
#line 243 "ts1.0_grammar.y"
    {
		    yyval.inst = new Inst(TSP_DOT_PRODUCT_REFLECT_CUBE_MAP_CONST_EYE_1_OF_3, yyvsp[-7].variable, yyvsp[-5].fval, yyvsp[-3].fval, yyvsp[-1].fval);
			delete yyvsp[-7].variable;
		;}
    break;

  case 37:
#line 248 "ts1.0_grammar.y"
    {
		    yyval.inst = new Inst(TSP_DOT_PRODUCT_REFLECT_CUBE_MAP_CONST_EYE_2_OF_3, yyvsp[-1].variable);
			delete yyvsp[-1].variable;
		;}
    break;

  case 38:
#line 253 "ts1.0_grammar.y"
    {
		    yyval.inst = new Inst(TSP_DOT_PRODUCT_REFLECT_CUBE_MAP_CONST_EYE_3_OF_3, yyvsp[-1].variable);
			delete yyvsp[-1].variable;
		;}
    break;

  case 39:
#line 258 "ts1.0_grammar.y"
    {
		    yyval.inst = new Inst(TSP_DOT_PRODUCT_CUBE_MAP_AND_REFLECT_CUBE_MAP_EYE_FROM_QS_1_OF_3, yyvsp[-1].variable);
			delete yyvsp[-1].variable;
		;}
    break;

  case 40:
#line 263 "ts1.0_grammar.y"
    {
		    yyval.inst = new Inst(TSP_DOT_PRODUCT_CUBE_MAP_AND_REFLECT_CUBE_MAP_EYE_FROM_QS_2_OF_3, yyvsp[-1].variable);
			delete yyvsp[-1].variable;
		;}
    break;

  case 41:
#line 268 "ts1.0_grammar.y"
    {
		    yyval.inst = new Inst(TSP_DOT_PRODUCT_CUBE_MAP_AND_REFLECT_CUBE_MAP_EYE_FROM_QS_3_OF_3, yyvsp[-1].variable);
			delete yyvsp[-1].variable;
		;}
    break;

  case 42:
#line 273 "ts1.0_grammar.y"
    {
		    yyval.inst = new Inst(TSP_DOT_PRODUCT_CUBE_MAP_AND_REFLECT_CUBE_MAP_CONST_EYE_1_OF_3, yyvsp[-7].variable, yyvsp[-5].fval, yyvsp[-3].fval, yyvsp[-1].fval);
			delete yyvsp[-7].variable;
		;}
    break;

  case 43:
#line 278 "ts1.0_grammar.y"
    {
		    yyval.inst = new Inst(TSP_DOT_PRODUCT_CUBE_MAP_AND_REFLECT_CUBE_MAP_CONST_EYE_2_OF_3, yyvsp[-1].variable);
			delete yyvsp[-1].variable;
		;}
    break;

  case 44:
#line 283 "ts1.0_grammar.y"
    {
		    yyval.inst = new Inst(TSP_DOT_PRODUCT_CUBE_MAP_AND_REFLECT_CUBE_MAP_CONST_EYE_3_OF_3, yyvsp[-1].variable);
			delete yyvsp[-1].variable;
		;}
    break;

  case 45:
#line 290 "ts1.0_grammar.y"
    {
			yyval.fval = yyvsp[0].fval;
		;}
    break;

  case 46:
#line 294 "ts1.0_grammar.y"
    {
			yyval.fval = yyvsp[0].fval;
		;}
    break;


    }

/* Line 999 of yacc.c.  */
#line 1610 "_ts1.0_parser.c"

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


#line 300 "ts1.0_grammar.y"

void yyerror(const char* s)
{
     errors.set("unrecognized token");
}

