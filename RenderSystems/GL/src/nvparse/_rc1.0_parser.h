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
/* Line 1240 of yacc.c.  */
#line 125 "_rc1.0_parser.h"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE rc10_lval;



