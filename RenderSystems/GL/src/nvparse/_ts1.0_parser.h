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




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 18 "ts1.0_grammar.y"
typedef union YYSTYPE {
  float fval;
  InstPtr inst;
  InstListPtr instList;
  MappedVariablePtr variable;
} YYSTYPE;
/* Line 1240 of yacc.c.  */
#line 141 "_ts1.0_parser.h"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE ts10_lval;



