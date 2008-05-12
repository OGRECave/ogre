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
/* Line 1240 of yacc.c.  */
#line 117 "_vs1.0_parser.h"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE vs10_lval;



