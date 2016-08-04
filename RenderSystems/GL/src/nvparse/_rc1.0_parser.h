/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison interface for Yacc-like parsers in C

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
#line 20 "rc1.0_grammar.y" /* yacc.c:1909  */

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

#line 109 "_rc1.0_parser.h" /* yacc.c:1909  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE rc10_lval;

int rc10_parse (void);

#endif /* !YY_RC10_RC1_0_PARSER_H_INCLUDED  */
